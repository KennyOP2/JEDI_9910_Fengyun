#ifndef	USBEX_URBBUF_H
#define	USBEX_URBBUF_H


typedef enum {
    URB_BUF_FREE = 0,
    URB_BUF_USED,
    URB_BUF_ISO
} URB_BUF_T;


#define URBLIST_HEAD(entryType)                \
struct {                                      \
        struct entryType *listHead,*listTail; \
        MMP_UINT32 count;                     \
        MMP_MUTEX lock;                       \
} 

#define URBLIST_ENTRY(entryType)               \
        struct entryType *next;

/** This mutex will never be destroyed */
#define URBLIST_INIT(head)                  do{   \
        head.listHead = head.listTail = MMP_NULL; \
        head.count = 0;                       \
        head.lock = SYS_CreateSemaphore(1, MMP_NULL);   \
}while(0) 

#define URBLIST_CNT(head)                    (head.count)
                
#define URBLIST_INSERT(head,entry,field)    do{\
    SYS_WaitSemaphore(head.lock); \
    entry->field = MMP_NULL;                 \
    if(head.listHead == MMP_NULL)            \
        head.listHead = entry;               \
    else                                     \
        head.listTail->field = entry;        \
    head.listTail = entry;                   \
    head.count++;                            \
    if(entry->type != URB_BUF_FREE)          \
        entry->type = URB_BUF_FREE;          \
    SYS_ReleaseSemaphore(head.lock);               \
} while(0) 

#define URBLIST_REMOVE(head,entry,field)     do{\
    SYS_WaitSemaphore(head.lock); \
    if(head.count) {                           \
        entry = head.listHead;                 \
        if(entry) {                            \
           head.listHead = entry->field;       \
           entry->field = MMP_NULL;            \
           head.count--;                       \
           memset((void*)((MMP_UINT32)entry+12), 0x0, (sizeof(*entry)-12)); \
           entry->cookies = URB_BUF_COOKIE; \
           entry->type = URB_BUF_USED;         \
        }                                      \
    } else {entry = MMP_NULL;}                 \
    SYS_ReleaseSemaphore(head.lock);                 \
} while(0) 


MMP_INT urbBufInitialize(void);
MMP_INT urbBufGet(struct urb** urb);
MMP_INT urbBufFree(struct urb* urb);
MMP_UINT32 urbBufCount(void);


#endif


