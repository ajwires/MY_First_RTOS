#include <stdint.h>
#include "rtos.h"
#include "stm32f4xx_hal_conf.h"
#include "qassert.h"

Q_DEFINE_THIS_FILE

//* before volatile makes the POINTER volatile
OSThread * volatile OS_curr;	//pointer to the current thread. vol because called in interrupt. 
OSThread * volatile OS_next;	//pointer to next thread. vol because called in interrupt

OSThread *OS_thread[32 + 1]; /* array of threads started so far */
uint8_t OS_threadNum; /* number of threads started so far */
uint8_t OS_currIdx; /* current thread index for round robin scheduling */

void OS_init(void)
{
	//Set PendSV interrupt priority to lowest
	*(uint32_t volatile *)0xE000ED20 |= (0xFFU << 16);
}

/*	OS run
 *	
 *	Starts after threads are created and OS_init()
 *
 */
void OS_run(void) {
	/* callback to configure and start interrupts */
	OS_onStartup();

	__disable_irq();
	OS_sched();	//start first thread
	__enable_irq();

	// should never execute 
	Q_ERROR();
}

/*	schedular
 *
 *	Round robin
 *	
 */
void OS_sched(void)
{
	/* OS_next = ... */
	++OS_currIdx;
	if (OS_currIdx == OS_threadNum) {
		OS_currIdx = 0U;
	}
	OS_next = OS_thread[OS_currIdx];
    
	/* trigger PendSV, if needed */
	if (OS_next != OS_curr) {
		*(uint32_t volatile *)0xE000ED04 = (1U << 28);
	}

}

void OSThread_start(OSThread *me, OSThreadHandler threadHandler, void *stkSto, uint32_t stkSize)
{
	/* round down the stack top to the 8-byte boundary
	* NOTE: ARM Cortex-M stack grows down from hi -> low memory
	*/
	uint32_t *sp = (uint32_t *)((((uint32_t)stkSto + stkSize) / 8) * 8);
	uint32_t *stk_limit;
    
	*(--sp) = (1U << 24); /* xPSR */
	*(--sp) = (uint32_t)threadHandler; /* PC */
	*(--sp) = 0x0000000EU; /* LR  */
	*(--sp) = 0x0000000CU; /* R12 */
	*(--sp) = 0x00000003U; /* R3  */
	*(--sp) = 0x00000002U; /* R2  */
	*(--sp) = 0x00000001U; /* R1  */
	*(--sp) = 0x00000000U; /* R0  */
	/* additionally, fake registers R4-R11 */
	*(--sp) = 0x0000000BU; /* R11 */
	*(--sp) = 0x0000000AU; /* R10 */
	*(--sp) = 0x00000009U; /* R9 */
	*(--sp) = 0x00000008U; /* R8 */
	*(--sp) = 0x00000007U; /* R7 */
	*(--sp) = 0x00000006U; /* R6 */
	*(--sp) = 0x00000005U; /* R5 */
	*(--sp) = 0x00000004U; /* R4 */

	/* save the top of the stack in the thread's attibute */
	me->sp = sp;
    
	/* round up the bottom of the stack to the 8-byte boundary */
	stk_limit = (uint32_t *)(((((uint32_t)stkSto - 1U) / 8) + 1U) * 8);

	/* pre-fill the unused part of the stack with 0xDEADBEEF */
	for (sp = sp - 1U; sp >= stk_limit; --sp) {
		*sp = 0xDEADBEEFU;
	}

	Q_ASSERT(OS_threadNum < Q_DIM(OS_thread));

	/* register the thread with the OS */
	OS_thread[OS_threadNum] = me;
	++OS_threadNum;
}


void PendSV_Handler(void) {
	//"IMPORT  OS_curr  /* extern variable */"
	//"IMPORT  OS_next  /* extern variable */"
   
	// __disable_irq(); 
	__asm("CPSID         I");

	// if (OS_curr != (OSThread *)0) { 
	__asm("LDR           r1,=OS_curr");
	__asm("LDR           r1, [r1,  # 0x00]");
	//__asm("CBZ           r1, PendSV_restore");
	//     push registers r4-r11 on the stack 
	__asm("PUSH          { r4 - r11 } ");

	//     OS_curr->sp = sp; 
	__asm("LDR           r1,  = OS_curr");
	__asm("LDR           r1, [r1,  # 0x00]");
	__asm("STR           sp, [r1,  # 0x00]");


	__asm("PendSV_restore:   ");
	// sp = OS_next->sp; 
	__asm("		LDR           r1,  = OS_next");
	__asm("		LDR           r1, [r1,  # 0x00]");
	__asm("		LDR           sp, [r1,  # 0x00]");
	// OS_curr = OS_next; 
	__asm("    LDR           r1,  = OS_next");
	__asm("    LDR           r1, [r1,  # 0x00]");
	__asm("    LDR           r2,  = OS_curr");
	__asm("    STR           r1, [r2,  # 0x00]");
	// pop registers r4-r11  
	__asm("    POP           { r4 - r11 }    ");
	// __enable_irq(); 
	__asm("    CPSIE         I");
	// return to the next thread 
	__asm("    BX            lr");
}