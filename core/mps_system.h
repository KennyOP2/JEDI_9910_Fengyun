#ifndef MPS_SYSTEM_H
#define MPS_SYSTEM_H

#include "mmp_types.h"
#include "mps_cmdq.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

typedef enum MPS_ERROR_CODE_TAG
{
    MPS_ERROR_CODE_INVALID_INPUT,
    MPS_ERROR_CODE_NO_MATCH_ELEMENT,
    MPS_ERROR_CODE_NO_MATCH_CONNECTOR,
    MPS_ERROR_CODE_CONNECTOR_IN_USE,
    MPS_ERROR_CODE_NO_ERROR = 0
} MPS_ERROR_CODE;

typedef enum MPS_ELEMENT_ID_TAG
{
    MPS_VIDEO_CAPTURE = 0,
    MPS_AUDIO_IN,
    MPS_VIDEO_ENCODER,
    MPS_STREAM_MUX,
#ifdef ENABLE_MENCODER
    MPS_AVI_MUX,
#endif
} MPS_ELEMENT_ID;

typedef enum MPS_ELEMENT_ROLE_TAG
{
    MPS_SRC_ELEMENT = 0,
    MPS_DEST_ELEMENT
} MPS_ELEMENT_ROLE;

typedef enum MPS_MPS_SYSTEM_TYPE_TAG
{
        MPS_SYSTEM_TYPE_NULL_STREAM = 0xFF,
//#ifndef ENABLE_MENCODER
    MPS_SYSTEM_TYPE_STREAM_MUX,
//#else
    MPS_SYSTEM_TYPE_AVI_MUX,
//#endif
    MPS_SYSTEM_TYPE_UNKNOWN
} MPS_SYSTEM_TYPE;

typedef enum MPS_PROPERITY_ID_TAG
{
    MPS_PROPERITY_OPERATION = 1,    // replace later
    MPS_PROPERITY_SET_ENCODE_PARAMETER,
    MPS_PROPERITY_SET_MUXER_PARAMETER,
    MPS_PROPERITY_SET_AUDIO_ENCODE_PARAMETER,
    MPS_PROPERITY_GET_AUDIO_ENCODE_PARAMETER,
    MPS_PROPERITY_SET_CAPTURE_DEVICE,
    MPS_PROPERITY_SET_ISP_MODE,
    MPS_PROPERITY_SET_ENABLE_AV_ENGINE,
    MPS_PROPERITY_SET_RECORD_MODE,
} MPS_PROPERITY_ID;

typedef enum MPS_NOTIFY_REASON_TAG
{
    MPS_NOTIFY_REASON_OPEN_FILE_SUCCESS = 1,    // replace later
    MPS_NOTIFY_REASON_OPEN_FILE_FAIL,
    MPS_NOTIFY_REASON_END_OF_FILE,
    MPS_NOTIFY_REASON_FILE_WRITE_FAIL,
    MPS_NOTIFY_REASON_NO_MORE_STORAGE,
    MPS_NOTIFY_REASON_FILE_CLOSE_FAIL,
    MPS_NOTIFY_REASON_FILE_CLOSE_SUCCESS,
    MPS_NOTIFY_REASON_WRITE_SPEED_TOO_SLOW,
} MPS_NOTIFY_REASON;

typedef enum MPS_CMD_QUEUE_ID_TAG
{
    VIDEO_CAPTURE_CMD_ID        = CMD_QUEUE1_ID,
    AUDIO_IN_CMD_ID             = CMD_QUEUE2_ID,
    VIDEO_ENCODER_CMD_ID        = CMD_QUEUE3_ID,
    STREAM_MUX_CMD_ID           = CMD_QUEUE4_ID,
    AVI_MUX_CMD_ID              = CMD_QUEUE4_ID,

    MPS_CTRL_CMD_ID             = CMD_QUEUE5_ID,
    MPS_CTRL_EVENT_ID           = EVENT_QUEUE_ID,
    MPS_CTRL_SPECIAL_EVENT_ID   = SPECIAL_EVENT_QUEUE_ID,
    MPS_TOTAL_CMD_QUEUE
} MPS_CMD_QUEUE_ID;

typedef enum MPS_STATE_TAG
{
    MPS_STATE_ZERO = 0,
    MPS_STATE_STOP,
    MPS_STATE_PAUSE,
    MPS_STATE_RUN,
    MPS_STATE_TOTAL
} MPS_STATE;

//=============================================================================
//                              Macro Definition
//=============================================================================

typedef void (*OPERATION_API)(MMP_UINT32 argc);
typedef struct MPS_CONNECTOR_LIST_OBJ_TAG MPS_CONNECTOR_LIST_OBJ;
typedef struct MPS_ELEMENT_LIST_OBJ_TAG MPS_ELEMENT_LIST_OBJ;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct MPS_ELEMENT_TAG
{
    MPS_ELEMENT_ID              id;

    OPERATION_API               pfInit;
    OPERATION_API               pfTerminate;

    MPS_CONNECTOR_LIST_OBJ*     ptSrcList;
    MPS_CONNECTOR_LIST_OBJ*     ptDestList;
    MPS_CMD_QUEUE_ID            cmdQueueId;     // Always acts as receiver
    MPS_CMD_QUEUE_ID            eventId;        // Notify event to controller
    MPS_CMD_QUEUE_ID            specialEventId; // For some special event notification.
} MPS_ELEMENT;

struct MPS_ELEMENT_LIST_OBJ_TAG
{
    MPS_ELEMENT*          ptElement;
    MPS_ELEMENT_LIST_OBJ* ptNext;
    MPS_ELEMENT_LIST_OBJ* ptPrevious;
};

typedef struct MPS_CONNECTOR_TAG
{
    QUEUE_ID            queueId;
    MMP_UINT32          refCount;
    MPS_ELEMENT*        ptSrcElement;
    MPS_ELEMENT*        ptDestElement;
} MPS_CONNECTOR;

struct MPS_CONNECTOR_LIST_OBJ_TAG
{
    MPS_CONNECTOR*          ptConnector;
    MPS_CONNECTOR_LIST_OBJ* ptNext;
};

typedef struct MPS_SYSTEM_HANDLE_TAG
{
    MPS_ELEMENT_LIST_OBJ*   ptFirstElementObj;
    MPS_ELEMENT_LIST_OBJ*   ptLastElementObj;
    MPS_CONNECTOR_LIST_OBJ* ptFirstConnectorObj;
} MPS_SYSTEM_HANDLE;

typedef struct STREAM_HANDLE_TAG
{
    QUEUE_ID           queueId;
    QUEUE_CTRL_HANDLE* ptQueueHandle;
} STREAM_HANDLE;

typedef struct MPS_POPERITY_DATA_TAG
{
    MPS_PROPERITY_ID        properityId;
    MMP_UINT32              data;
} MPS_PROPERITY_DATA;

typedef struct MPS_NOTIFY_REASON_DATA_TAG
{
    MPS_NOTIFY_REASON       notifyReason;
    MMP_BOOL                bDatatNeedFree;
    MMP_UINT32              data;
} MPS_NOTIFY_REASON_DATA;

//=============================================================================
//                              Function  Definition
//=============================================================================

//=============================================================================
/**
 * Get the primary handle of whole player system.
 *
 * @return  MPS_SYSTEM_HANDLE* which represent the primary
 *          handle to control the player system.
 */
//=============================================================================
MPS_SYSTEM_HANDLE*
mpsSys_GetHandle(
    void);

//=============================================================================
/**
 * Release all allocated resource and then destroy the whole system
 *
 * @return  none
 */
//=============================================================================
void
mpsSys_DestroySystem(
    void);

//=============================================================================
/**
 * Create a new element to act as one of the following characters of player -
 * source reader, parser, decoder.
 *
 * @param id        a identifier of the element.
 * @param cmdId     a command queue id to handle the command transmission between
 *                  MPS Ctrl task and the task of the element.
 * @return          MPS_ELEMENT* to represents one of the role of
 *                  the player system.
 */
//=============================================================================
MPS_ELEMENT*
mpsSys_CreateNewElement(
    MPS_ELEMENT_ID   id,
    MPS_CMD_QUEUE_ID cmdId);

//=============================================================================
/**
 * Get a created element from the player system
 *
 * @param id        a identifier of the element.
 * @return          MPS_ELEMENT* if the pointer is equal to MMP_NULL, it means
 *                  no hit of the search.
 */
//=============================================================================
MPS_ELEMENT*
mpsSys_GetElement(
    MPS_ELEMENT_ID   id);

//=============================================================================
/**
 * Remove a element object from the player system
 *
 * @param pptElement   The pointer to a specific element object pointer.
 * @return             MPS_ERROR_CODE to notify if the operation is success
 *                     or failed.
 */
//=============================================================================
MPS_ERROR_CODE
mpsSys_DeleteElement(
    MPS_ELEMENT** pptElement);

//=============================================================================
/**
 * Destroy the whole elment list from the player system
 *
 * @return  MPS_ERROR_CODE to notify if the operation is success or failed.
 */
//=============================================================================
MPS_ERROR_CODE
mpsSys_DestoryElementList(
    void);

//=============================================================================
/**
 * Create a new connector to be a unidirectional bus between two elements.
 *
 * @return          MPS_CONNECTOR* to play as the unidirectional bus between
 *                  the source and destination elements.
 */
//=============================================================================
MPS_CONNECTOR*
mpsSys_CreateNewConnector(
    void);

//=============================================================================
/**
 * Remove a connector object from the player system
 *
 * @param pptConnector The pointer to a specific connector object pointer.
 * @return             MPS_ERROR_CODE to notify if the operation is success
 *                     or failed.
 */
//=============================================================================
MPS_ERROR_CODE
mpsSys_DeleteConnector(
    MPS_CONNECTOR** pptConnector);

//=============================================================================
/**
 * Destroy the whole connector list from the player system
 *
 * @return  MPS_ERROR_CODE to notify if the operation is success or failed.
 */
//=============================================================================
MPS_ERROR_CODE
mpsSys_DestoryConnectorList(
    void);

//=============================================================================
/**
 * Hook a created connector on a specific element.
 *
 * @param ptElement     The pointer to a specific element.
 * @param role          The role, source or destination, of the element.
 * @param ptConnector   The pointer to a specific connector object pointer.
 * @return              MPS_ERROR_CODE to notify if the operation is success
 *                      or failed.
 */
//=============================================================================
MPS_ERROR_CODE
mpsSys_HookConnector(
    MPS_ELEMENT*        ptElement,
    MPS_ELEMENT_ROLE    role,
    MPS_CONNECTOR*      ptConnector);

//=============================================================================
/**
 * Un-Hook a connector from a specific element.
 *
 * @param ptElement     The pointer to a specific element.
 * @param ptConnector   The pointer to a specific connector object pointer.
 * @return              MPS_ERROR_CODE to notify if the operation is success
 *                      or failed.
 */
//=============================================================================
MPS_ERROR_CODE
mpsSys_UnHookConnector(
    MPS_ELEMENT*        ptElement,
    MPS_CONNECTOR*      ptConnector);

//=============================================================================
/**
 * Un-hook all connectors of the element.
 *
 * @param ptElement The pointer to a specific element.
 * @return          MPS_ERROR_CODE to notify if the operation is success or
 *                  failed.
 */
//=============================================================================
MPS_ERROR_CODE
mpsSys_UnHookConnectorList(
    MPS_ELEMENT*     ptElement);

//=============================================================================
/**
 * Check if the state transaction is valid or not.
 *
 * @param previousState    The Current state of the state machine.
 * @param newState         The New state of the state machine.
 * @return                 MMP_TRUE if the transaction is valid, invalid
 *                         otherwise.
 */
//=============================================================================
static MMP_INLINE MMP_BOOL mpsSys_CheckStateChange(
    MPS_STATE currentState,
    MPS_STATE newState)
{
//   [Previous Statem]  [New State]
// {
//   {[MPS_STATE_ZERO]  [MPS_STATE_ZERO],  [MPS_STATE_ZERO]  [MPS_STATE_STOP],
//    [MPS_STATE_ZERO]  [MPS_STATE_PAUSE], [MPS_STATE_ZERO]  [MPS_STATE_RUN], [MPS_STATE_ZERO] },

//   {[MPS_STATE_STOP]  [MPS_STATE_ZERO],  [MPS_STATE_STOP]  [MPS_STATE_STOP],
//    [MPS_STATE_STOP]  [MPS_STATE_PAUSE], [MPS_STATE_STOP]  [MPS_STATE_RUN], [MPS_STATE_STOP] },

//   {[MPS_STATE_PAUSE] [MPS_STATE_ZERO],  [MPS_STATE_PAUSE] [MPS_STATE_STOP],
//    [MPS_STATE_PAUSE] [MPS_STATE_PAUSE], [MPS_STATE_PAUSE] [MPS_STATE_RUN], [MPS_STATE_PAUSE]},

//   {[MPS_STATE_RUN]   [MPS_STATE_ZERO],  [MPS_STATE_RUN]   [MPS_STATE_STOP],
//    [MPS_STATE_RUN]   [MPS_STATE_PAUSE], [MPS_STATE_RUN]   [MPS_STATE_RUN], [MPS_STATE_RUN]  },
// }
    MMP_BOOL transactionTable[MPS_STATE_TOTAL][MPS_STATE_TOTAL] =
        {{MMP_TRUE,  MMP_TRUE,
          MMP_FALSE, MMP_FALSE},

         {MMP_TRUE,  MMP_TRUE,
          MMP_TRUE,  MMP_TRUE},

         {MMP_TRUE,  MMP_TRUE,
          MMP_TRUE,  MMP_TRUE},

         {MMP_FALSE, MMP_TRUE,
          MMP_TRUE,  MMP_TRUE}
        };

    return transactionTable[currentState][newState];
}

#ifdef __cplusplus
}
#endif

#endif
