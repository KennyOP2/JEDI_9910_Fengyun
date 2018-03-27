#ifndef EHCI_VMEM_H
#define EHCI_VMEM_H


#if defined(__OPENRTOS__)
#include "ite/ith.h"
#endif
#if defined(__OR32__)
#include "or32.h"
#define ithInvalidateDCacheRange    or32_invalidate_cache
#endif

#if defined(__FREERTOS__) || defined(__OPENRTOS__)

#define VMEM_STRUCT_W(entryType, ptr, member, value)  \
    (ptr)->member = (value);

#if defined(__FREERTOS__) || defined(__OPENRTOS__)
#define VMEM_STRUCT_R(entryType, ptr, member, value)   do{\
    ithInvalidateDCacheRange((void*)(&((ptr)->member)), sizeof(MMP_UINT32)); \
    (*value) = ((ptr)->member); \
} while(0)
#else
#define VMEM_STRUCT_R(entryType, ptr, member, value)   \
    (*value) = (ptr)->member; 
#endif

#define LIST_ADD_TAIL_VMEM_SV           list_add_tail
#define LIST_DEL_VMEM_SV(entry, head)   list_del(entry)
#define LIST_DEL_VMEM_VV(entry)         list_del(entry)
#define LIST_EMPTY_VMEM                 list_empty
#define LIST_SPLICE_VMEM                list_splice


#else // WIN32

/** NOTE: All must be 4 bytes alignment!! */

#define VMEM_STRUCT_W(entryType, ptr, member, value) do{ \
    MMP_UINT32 member_addr = (MMP_UINT32)((MMP_UINT32)(ptr)+(MMP_UINT32)(&((struct entryType*)0)->member)); \
    HOST_WriteBlockMemory(member_addr, (MMP_UINT32)&(value), sizeof(MMP_UINT32)); \
} while(0)

#define VMEM_STRUCT_R(entryType, ptr, member, value)   do{\
    MMP_UINT32 member_addr = (MMP_UINT32)((MMP_UINT32)(ptr)+(MMP_UINT32)(&((struct entryType*)0)->member)); \
    HOST_ReadBlockMemory((MMP_UINT32)(value), member_addr, sizeof(MMP_UINT32)); \
} while(0)

/*------------------------------------------------*/
/** prev_s <-> new_v <-> next_s */
static MMP_INLINE void __LIST_ADD_VMEM_SVS(struct list_head* new_v, struct list_head* prev_s, struct list_head* next_s)
{
    (next_s)->prev = new_v;                       
    VMEM_STRUCT_W(list_head, new_v, next, next_s);      
    VMEM_STRUCT_W(list_head, new_v, prev, prev_s);      
    (prev_s)->next = new_v;                       
}

/** prev_v <-> new_v <-> next_s */
static MMP_INLINE void __LIST_ADD_VMEM_VVS(struct list_head* new_v, struct list_head* prev_v, struct list_head* next_s)
{
    (next_s)->prev = new_v;                       
    VMEM_STRUCT_W(list_head, new_v, next, next_s);      
    VMEM_STRUCT_W(list_head, new_v, prev, prev_v);      
    VMEM_STRUCT_W(list_head, prev_v, next, new_v);      
}

/** head is in system memory, others are in video memory */
static MMP_INLINE void LIST_ADD_TAIL_VMEM_SV(struct list_head* new_v, struct list_head* head_s) 
{
    if((head_s)->prev == head_s)
        __LIST_ADD_VMEM_SVS(new_v, head_s->prev, head_s);
    else 
        __LIST_ADD_VMEM_VVS(new_v, head_s->prev, head_s);
}

/*------------------------------------------------*/
#define __LIST_DEL_VMEM_SS      __list_del

static MMP_INLINE void __LIST_DEL_VMEM_SV(struct list_head* prev_s, struct list_head* next_v)
{
    VMEM_STRUCT_W(list_head, next_v, prev, prev_s);
    prev_s->next = next_v;
}

static MMP_INLINE void __LIST_DEL_VMEM_VV(struct list_head* prev_v, struct list_head* next_v)
{
    VMEM_STRUCT_W(list_head, next_v, prev, prev_v);
    VMEM_STRUCT_W(list_head, prev_v, next, next_v);
}

/** entry->prev is head (in system memory), entry->next may in video or system memory */
static MMP_INLINE void LIST_DEL_VMEM_SV(struct list_head* head_s, struct list_head* entry_v)
{
    void* value = 0;
    struct list_head* next_tmp = 0;
    struct list_head* prev_tmp = 0;

    VMEM_STRUCT_R(list_head, entry_v, next, &next_tmp);
    VMEM_STRUCT_R(list_head, entry_v, prev, &prev_tmp);
    if(prev_tmp != head_s)
    {
        LOG_ERROR " LIST_DEL_VMEM_SV() prev != haed_s \n" LOG_END
    }
    if(next_tmp == head_s)
        __LIST_DEL_VMEM_SS(prev_tmp, next_tmp); /* prev_v -> this_v -> next_s(head) */
    else
        __LIST_DEL_VMEM_SV(prev_tmp, next_tmp); /* prev_v -> this_v -> next_v */
    VMEM_STRUCT_W(list_head, entry_v, next, value);
    VMEM_STRUCT_W(list_head, entry_v, prev, value);
}

/** all in video memory */
static MMP_INLINE void LIST_DEL_VMEM_VV(struct list_head* entry_v)
{
    void* value = 0;
    struct list_head* next_tmp = 0;
    struct list_head* prev_tmp = 0;

    VMEM_STRUCT_R(list_head, entry_v, next, &next_tmp);
    VMEM_STRUCT_R(list_head, entry_v, prev, &prev_tmp);
    __LIST_DEL_VMEM_VV(prev_tmp, next_tmp); /* prev_v -> this_v -> next_v */
    VMEM_STRUCT_W(list_head, entry_v, next, value);
    VMEM_STRUCT_W(list_head, entry_v, prev, value);
}

/*------------------------------------------------*/
static MMP_INLINE MMP_BOOL IsVram(MMP_UINT8* addr)
{
#if defined(ALPHA_SDK)
    return false;
#else
    if(addr >= HOST_GetVramBaseAddress())
        return MMP_TRUE;
    else
        return MMP_FALSE;
#endif
}

static MMP_INLINE MMP_INT LIST_EMPTY_VMEM(struct list_head *head)
{
    if(IsVram((MMP_UINT8*)head))
    {
        struct list_head* tmp_list_head;
        VMEM_STRUCT_R(list_head, head, next, &tmp_list_head);
        return (tmp_list_head == head);
    }
    else
    	return head->next == head;
}

/** list's head is in system memory, others in vram */
static MMP_INLINE void __LIST_SPLICE_VMEM(struct list_head* list, struct list_head* head)
{
	struct list_head* first_v = list->next;
	struct list_head* last_v = list->prev;
	struct list_head* at_v = MMP_NULL;

    VMEM_STRUCT_R(list_head, head, next, &at_v);

    VMEM_STRUCT_W(list_head, first_v, prev, head);
    VMEM_STRUCT_W(list_head, head, next, first_v);

    VMEM_STRUCT_W(list_head, last_v, next, at_v);
    VMEM_STRUCT_W(list_head, at_v, prev, last_v);
}

/**
 * list_splice - join two lists
 * @list: the new list to add. @@ list's head is in system memory, others in vram
 * @head: the place to add it in the first list. @@ head is in vram
 */
static MMP_INLINE void LIST_SPLICE_VMEM(struct list_head *list, struct list_head *head)
{
    if(IsVram((MMP_UINT8*)list))
    {
        LOG_ERROR " list's head must be in system memory! \n" LOG_END
        while(1);
    }
	if(!list_empty(list))
		__LIST_SPLICE_VMEM(list, head);
}



#endif // // #if defined(__FREERTOS__) || defined(__OPENRTOS__)





#define VMEM_STRUCT_R_ADDR(entryType, ptr, member, value)   do{\
    (*value) = (MMP_UINT8*)((MMP_UINT32)(ptr)+(MMP_UINT32)(&((struct entryType*)0)->member)); \
} while(0)

#define INIT_LIST_HEAD_VMEM(ptr) do{\
    VMEM_STRUCT_W(list_head, (ptr), next, (ptr)); \
    VMEM_STRUCT_W(list_head, (ptr), prev, (ptr)); \
} while(0)



#endif

