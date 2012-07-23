
#ifndef __KGEN_MSGQ_H__
#define __KGEN_MSGQ_H__

#define	MSG_STATE_IDLE 0      /* not included  msg */
#define	MSG_STATE_ACTIVE 1    /* included msg */

typedef struct 
{
    int           state;  /*  for list status */
    struct list_head        free;  /* free list that not in recv queue */
    struct list_head        stream; /*  list */
	wait_queue_head_t       done;  /* wait queue */
    char                    private[0]; /* char buf[0] means the pointer to the follow buf */
}stdKGenMsgqMem;

typedef struct 
{
    struct list_head        msgWaitList;
    struct list_head        msgFreeList;
    struct mutex            lock;
	spinlock_t              irqlock;
    void                   *priv_data;  /* private data pointer */
    int                     priv_size;  /* private data size */
}stdKGenMsgqId;

int kgen_msgq_timed_recv(stdKGenMsgqId *q, char * pchMsg,int iMsgByte,int iWaitFlag);
int kgen_msgq_timed_send(stdKGenMsgqId *q, char * pchMsg,int iMsgByte,int iNonBlocking);
int kgen_msgq_msg_nums(stdKGenMsgqId *q);
stdKGenMsgqId *kgen_msgq_new(int iMsgNum, int iMsgByte);
int kgen_msgq_free(stdKGenMsgqId *pstKGenMsgqId);



#endif


