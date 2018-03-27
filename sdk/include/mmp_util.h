/*
 * Copyright (c) 2004 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * SMedia Driver utilities library.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef SIS_UTIL_H
#define SIS_UTIL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MMP_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MMP_MIN(a, b) (((a) < (b)) ? (a) : (b))

#define MMP_COUNTOF(array) \
    (sizeof (array) / sizeof (array[0]))

typedef struct MMP_NODE_TAG
{
    struct MMP_NODE_TAG* next;
    struct MMP_NODE_TAG* prev;
} MMP_NODE;

typedef struct MMP_LIST_TAG
{
    MMP_NODE* head;
} MMP_LIST;

static __inline void MMP_LIST_Init(MMP_LIST* list)
{
    list->head = NULL;
}

static __inline void MMP_LIST_PushFront(MMP_LIST* list, MMP_NODE* node)
{
    node->next = list->head;
	if (node->next != NULL)		node->next->prev = node;
    list->head = node;
    node->prev = NULL;
}

static __inline void MMP_LIST_Remove(MMP_LIST* list, MMP_NODE* node)
{
    if (node->prev == NULL) {
        list->head = node->next;
    } else {
        node->prev->next = node->next;
    }
    if (node->next != NULL)
        node->next->prev = node->prev;
}

static __inline MMP_NODE* MMP_LIST_First(MMP_LIST* list)
{
    return list->head;
}

static __inline MMP_NODE* MMP_LIST_Next(MMP_NODE* node)
{
    return node->next;
}

/**
 * Converts 16-bit value to another endian integer.
 *
 * @param value The value.
 * @return The converted value.
 */
#define MMP_SWAP_ENDIAN16(value) \
    ((((value) >> 8) & 0x00FF) | \
     (((value) << 8) & 0xFF00))

/**
 * Converts 32-bit value to another endian integer.
 *
 * @param value The value.
 * @return The converted value.
 */
#define MMP_SWAP_ENDIAN32(value) \
    ((((value) >> 24) & 0x000000FF) | \
     (((value) >>  8) & 0x0000FF00) | \
     (((value) <<  8) & 0x00FF0000) | \
     (((value) << 24) & 0xFF000000))

/* Endian macro definitions */
#if defined(__FREERTOS__)
    #define MMP_DATA16(value) \
        ((((value) >> 8) & 0x00FF) | \
         (((value) << 8) & 0xFF00))

    #define MMP_DATA32(value) \
        ((((value) >> 24) & 0x000000FF) | \
         (((value) >>  8) & 0x0000FF00) | \
         (((value) <<  8) & 0x00FF0000) | \
         (((value) << 24) & 0xFF000000))
#else
    #define MMP_DATA16(value) ((MMP_UINT16) (value))
    #define MMP_DATA32(value) ((MMP_UINT32) (value))
#endif // #if defined(__FREERTOS__)


#if defined(__FREERTOS__) && defined(__OR32__)
    #define cpu_to_le32(x) MMP_SWAP_ENDIAN32((MMP_UINT32)(x))
    #define le32_to_cpu(x) MMP_SWAP_ENDIAN32((MMP_UINT32)(x))
    #define cpu_to_le16(x) MMP_SWAP_ENDIAN16((MMP_UINT16)(x))
    #define le16_to_cpu(x) MMP_SWAP_ENDIAN16((MMP_UINT16)(x))
#else
    #define cpu_to_le32(x) ((MMP_UINT32)(x))
    #define le32_to_cpu(x) ((MMP_UINT32)(x))
    #define cpu_to_le16(x) ((MMP_UINT16)(x))
    #define le16_to_cpu(x) ((MMP_UINT16)(x))
#endif // #if defined(__FREERTOS__)



#ifdef __cplusplus
}
#endif

#endif /* SIS_UTIL_H */
