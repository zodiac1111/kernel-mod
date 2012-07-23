
#ifndef __KGEN_THRD_H__
#define __KGEN_THRD_H__

#include <linux/err.h>
#include <linux/sched.h>
#include <linux/kthread.h>

typedef enum
{
    KGen_THREAD_PRI_IDLE,
    KGen_THREAD_PRI_NORMAL,
    KGen_THREAD_PRI_CRITICAL,
    KGen_THREAD_PRI_END
}KGenThreadPriority;

typedef struct task_struct stdKGenTaskId;

stdKGenTaskId *kgen_thread_new(char *pcThreadName,
			       KGenThreadPriority ePriority, 
                        unsigned int iStackSize,
			int (* fnThreadEntry)(void *data), 
			       void *pArg);
int kgen_thread_free(stdKGenTaskId *kgen_task);

#endif



