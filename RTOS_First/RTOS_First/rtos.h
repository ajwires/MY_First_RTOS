/*************************************************
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 **************************************************/


#ifndef RTOS_H
#define RTOS_H






//Thread control block (TCB)
typedef struct
{
	void *sp;	//stack pointer
	
} OSThread;

typedef void (*OSThreadHandler)();

void OS_init(void);

void OS_onStartup(void);

void OS_run(void);

void OS_sched(void);

void OSThread_start(OSThread *me, OSThreadHandler threadHandler, void *stkSto, uint32_t stkSize);

void PendSV_Handler(void);




#endif //RTOS_H