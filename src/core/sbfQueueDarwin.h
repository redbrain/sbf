#ifndef _SBF_QUEUE_PRIVATE_H_
#error "must be included from sbfQueuePrivate.h"
#endif

#ifndef SBF_QUEUE_FUNCTIONS

#define SBF_QUEUE_DECL                          \
    sbfMutex                          mMutex;   \
    sbfCondVar                        mCondVar; \
    SIMPLEQ_HEAD (, sbfQueueItemImpl) mItems;

#define SBF_QUEUE_ITEM_DECL                     \
    SIMPLEQ_ENTRY (sbfQueueItemImpl) mEntry;

#else /* SBF_QUEUE_FUNCTIONS */

static SBF_INLINE void
sbfQueueCreate (sbfQueue queue)
{
    sbfMutex_init (&queue->mMutex, 0);
    sbfCondVar_init (&queue->mCondVar);
    SIMPLEQ_INIT (&queue->mItems);
}

static SBF_INLINE void
sbfQueueDestroy (sbfQueue queue)
{
    sbfQueueItem item;

    while ((item = SIMPLEQ_FIRST (&queue->mItems)) != NULL)
    {
        SIMPLEQ_REMOVE_HEAD (&queue->mItems, mEntry);
        sbfPool_put (item);
    }

    sbfCondVar_destroy (&queue->mCondVar);
    sbfMutex_destroy (&queue->mMutex);
}

static SBF_INLINE void
sbfQueueEnqueue (sbfQueue queue, sbfQueueItem item)
{
    sbfMutex_lock (&queue->mMutex);
    SIMPLEQ_INSERT_TAIL (&queue->mItems, item, mEntry);
    sbfCondVar_signal (&queue->mCondVar);
    sbfMutex_unlock (&queue->mMutex);
}

static SBF_INLINE sbfQueueItem
sbfQueueDequeue (sbfQueue queue)
{
    sbfQueueItem item = NULL;

    sbfMutex_lock (&queue->mMutex);
    while ((item = SIMPLEQ_FIRST (&queue->mItems)) == NULL)
        sbfCondVar_wait (&queue->mCondVar, &queue->mMutex);
    if (item != NULL)
        SIMPLEQ_REMOVE_HEAD (&queue->mItems, mEntry);
    sbfMutex_unlock (&queue->mMutex);

    return item;
}

#endif /* SBF_QUEUE_FUNCTIONS */
