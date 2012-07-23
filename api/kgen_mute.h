

#ifndef __KGEN_MUTE_H__
#define __KGEN_MUTE_H__



typedef struct 
{
    struct semaphore    MutexId;
}stdKGenMutexId;

typedef struct 
{
    spinlock_t lock;
}stdKGenSpinId;

int kgen_mutex_lock(stdKGenMutexId *pstKGenMutexId);
int kgen_mutex_trylock(stdKGenMutexId *pstKGenMutexId);
int kgen_mutex_unlock(stdKGenMutexId *pstKGenMutexId);
stdKGenMutexId *kgen_mutex_new(void);
int kgen_mutex_free(stdKGenMutexId *pstKGenMutexId);
int kgen_spin_lock(stdKGenSpinId *pstKGenSpinId, unsigned long *pulFlags);
int kgen_spin_unlock(stdKGenSpinId *pstKGenSpinId, unsigned long ulFlags);
stdKGenSpinId *kgen_spin_new(void);
int kgen_spin_free(stdKGenSpinId *pstKGenSpinId);

#endif

