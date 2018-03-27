/*
 * Copyright (c) 2009 ITE Technology Corp. All Rights Reserved.
 */
/** @file msg_route.h
 *
 * @author I-Chun Lai
 */

#include "pal/pal.h"
#include "msg_route.h"
#include "msg_core.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

#define DELAYED_MESSAGE_COUNT   (32)

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct DELAYED_MESSAGE_TAG
{
    MSG_OBJECT  tMsg;
} DELAYED_MESSAGE;

typedef struct
{
    DELAYED_MESSAGE tDelayedMessage[DELAYED_MESSAGE_COUNT];

    /**
     * Sets of unsent delayed messages.
     *
     * delayedMessages: 1 << delayedMessage 0 ... 31
     */
    MMP_UINT32      delayedMessages;

    MMP_MUTEX       tMutex;
} DELAYED_MESSAGE_QUEUE;

typedef MMP_BOOL
    (*COMPARE_MSG)(
        MSG_OBJECT* ptMsg,
        MSG_NAME name,
        MMP_UINT32 receiver_id);

//=============================================================================
//                              Global Data Definition
//=============================================================================

static DELAYED_MESSAGE_QUEUE gtDelayedMessageQueue;
static PROCESS_MSG gtpfProcessMsg;

//=============================================================================
//                              Private Function Declaration
//=============================================================================

void
_RouteMessage(
    MSG_OBJECT* ptMsg,
    MMP_BOOL    forceStore);

static MMP_BOOL
_RouteMessageHelper(
    MSG_OBJECT* ptMsg);

static void
_StoreDelayedMessage(
    MSG_OBJECT* ptMsg);

static MMP_INLINE void
_DeleteDelayedMessage(
    DELAYED_MESSAGE_QUEUE*  ptMsgQ,
    MMP_UINT32              msgIndex)
{
    ptMsgQ->tDelayedMessage[msgIndex].tMsg.name = MSG_NULL;
    ptMsgQ->delayedMessages &= ~(1 << msgIndex);
}

static MMP_INLINE void
_DeleteAllDelayedMessage(
    DELAYED_MESSAGE_QUEUE*  ptMsgQ)
{
    PalMemset(ptMsgQ->tDelayedMessage, 0, sizeof(ptMsgQ->tDelayedMessage));
    ptMsgQ->delayedMessages = 0;
}

static MMP_INLINE MMP_BOOL
_CompareMsg_Name(
    MSG_OBJECT* ptMsg,
    MSG_NAME name,
    MMP_UINT32 receiver)
{
    return (ptMsg->name == name) ? MMP_TRUE : MMP_FALSE;
}

static MMP_INLINE MMP_BOOL
_CompareMsg_Receiver(
    MSG_OBJECT* ptMsg,
    MSG_NAME name,
    MMP_UINT32 receiver)
{
    return (ptMsg->receiver_id == receiver) ? MMP_TRUE : MMP_FALSE;
}

static MMP_INLINE MMP_BOOL
_CompareMsg_NameAndReceiver(
    MSG_OBJECT* ptMsg,
    MSG_NAME name,
    MMP_UINT32 receiver)
{
    return ((ptMsg->name == name) && (ptMsg->receiver_id == receiver))
        ? MMP_TRUE : MMP_FALSE;
}

static MMP_INLINE MMP_BOOL
_CompareMsg_None(
    MSG_OBJECT* ptMsg,
    MSG_NAME name,
    MMP_UINT32 receiver)
{
    return MMP_TRUE;
}

//=============================================================================
//                              Public Function Definition
//=============================================================================

void
RegisterProcessMsgRoutine(
    PROCESS_MSG pfRoutine)
{
    SetDelayedMessagesProcessRoutine(pfRoutine); 
}

void 
SetDelayedMessagesProcessRoutine( 
   PROCESS_MSG pfRoutine) 
{ 
    gtpfProcessMsg = pfRoutine;
}

PROCESS_MSG
GetDelayedMessagesProcessRoutine(
    void)
{
    return gtpfProcessMsg;
}

//=============================================================================
/**
 * Initialize the delayed message queue
 *
 * @return none
 */
//=============================================================================
void
InitDelayedMessages(
    void )
{
    if (MMP_NULL == gtDelayedMessageQueue.tMutex)
        gtDelayedMessageQueue.tMutex = PalCreateMutex(MMP_NULL);

    if (gtDelayedMessageQueue.tMutex)
    {
        PalWaitMutex(gtDelayedMessageQueue.tMutex, PAL_MUTEX_INFINITE);
        _DeleteAllDelayedMessage(&gtDelayedMessageQueue);
        PalReleaseMutex(gtDelayedMessageQueue.tMutex);
    }
}

//=============================================================================
/**
 * Terminate the delayed message queue
 *
 * @return none
 */
//=============================================================================
void
TerminateDelayedMessages(
    void)
{
    if (gtDelayedMessageQueue.tMutex)
    {
        PalWaitMutex(gtDelayedMessageQueue.tMutex, PAL_MUTEX_INFINITE);
        _DeleteAllDelayedMessage(&gtDelayedMessageQueue);
        PalReleaseMutex(gtDelayedMessageQueue.tMutex);

        PalDestroyMutex(gtDelayedMessageQueue.tMutex);
        gtDelayedMessageQueue.tMutex = MMP_NULL;
    }
}

//=============================================================================
/**
 * Check if there are delayed messages needed to be sent, and send them if
 * necessary.
 *
 * @return none
 */
//=============================================================================
void SendDelayedMessages(void)
{   // This function is called every tick
    if (gtDelayedMessageQueue.tMutex)
    {
        DELAYED_MESSAGE* ptCurMsg   = MMP_NULL;
        DELAYED_MESSAGE  tSendMsg   = {0};
        MMP_UINT32 delayMsgIndex = 0;
        MMP_UINT32 smallestTimeStamp = 0;
        MMP_UINT i = 0;

        PalWaitMutex(gtDelayedMessageQueue.tMutex, PAL_MUTEX_INFINITE);
        if (gtDelayedMessageQueue.delayedMessages)
        {
            // find the smallest delivery timestamp of the delay message queue.
            for (i = 0; i < DELAYED_MESSAGE_COUNT; ++i)
            {
                ptCurMsg = &gtDelayedMessageQueue.tDelayedMessage[i];

                if (MSG_NULL != ptCurMsg->tMsg.name
                 && PalGetDuration(ptCurMsg->tMsg.delivery_time) >= ptCurMsg->tMsg.delay)
                {
                    if (smallestTimeStamp)
                    {
                        if ((ptCurMsg->tMsg.delivery_time + ptCurMsg->tMsg.delay) < smallestTimeStamp)
                        {
                            smallestTimeStamp = ptCurMsg->tMsg.delivery_time + ptCurMsg->tMsg.delay;
                            delayMsgIndex = i;
                        }
                    }
                    else
                    {
                        smallestTimeStamp = ptCurMsg->tMsg.delivery_time + ptCurMsg->tMsg.delay;
                        delayMsgIndex = i;
                    }
                }
            }

            if (smallestTimeStamp)
            {
                PalMemcpy(&tSendMsg, &gtDelayedMessageQueue.tDelayedMessage[delayMsgIndex], sizeof(DELAYED_MESSAGE));
                _DeleteDelayedMessage(&gtDelayedMessageQueue, delayMsgIndex);
            }
        }
        PalReleaseMutex(gtDelayedMessageQueue.tMutex);

        if (MSG_NULL != tSendMsg.tMsg.name)
        {
            _RouteMessage(&tSendMsg.tMsg, MMP_FALSE);
        }
    }
}

//=============================================================================
/**
 * Cancel the request of sending certain delayed message queue
 *
 * @param name      Message name.
 * @param receiver  Receiver module id
 * @return none
 * @remark  1. If neither of message name and receiver module id is 0, only the
 *             delayed messages with certain name and receiver are canceled.
 *          2. If message name is 0 but receiver module id is not 0, the delayed
 *             messages with certain receiver are canceled (whatever the message
 *             name is).
 *          3. If message name is 0 but receiver module id is not 0, the delayed
 *             messages with certain name are canceled (whatever the receiver
 *             is).
 *          4. If both message name and receiver module id are 0, all the
 *             delayed messages are canceled.
 */
//=============================================================================
void
CancelDelayedMessages(
    MSG_NAME name,
    MMP_UINT32 receiver)
{
    COMPARE_MSG pfCompare = MMP_NULL;

    if (name)
    {
        if (receiver)
            pfCompare = _CompareMsg_NameAndReceiver;
        else
            pfCompare = _CompareMsg_Name;
    }
    else
    {
        if (receiver)
            pfCompare = _CompareMsg_Receiver;
        else
            pfCompare = _CompareMsg_None;
    }

    if (gtDelayedMessageQueue.tMutex)
    {
        DELAYED_MESSAGE* ptCurMsg = MMP_NULL;
        MMP_UINT i = 0;

        PalWaitMutex(gtDelayedMessageQueue.tMutex, PAL_MUTEX_INFINITE);

        for (i = 0; i < DELAYED_MESSAGE_COUNT; ++i)
        {
            ptCurMsg = &gtDelayedMessageQueue.tDelayedMessage[i];
            if (MSG_NULL != ptCurMsg->tMsg.name
             && pfCompare(&ptCurMsg->tMsg, name, receiver))
            {
                _DeleteDelayedMessage(&gtDelayedMessageQueue, i);
            }
        }

        PalReleaseMutex(gtDelayedMessageQueue.tMutex);
    }
}

//=============================================================================
/**
 * Send a delayed message from sender module to receiver module
 *
 * @param name              The name of message
 * @param delay             The delay time (in milliseconds)
 * @param extraData         The extra data to be sent with the message
 * @param sender            The sender module id
 * @param receiver          The receiver module id
 * @return                  none
 */
//=============================================================================
void
SendDelayedMsg(
    MSG_NAME name,
    PAL_CLOCK_T delay,
    MMP_UINT32 extraData,
    MMP_UINT32 sender,
    MMP_UINT32 receiver)
{
    MSG_OBJECT msg;
    msg.name = name;                    //The name of the message
    msg.sender_id = sender;             //The sender
    msg.receiver_id = receiver;         //The receiver
    msg.delivery_time = PalGetClock();  //Send the message at a future time
    msg.delay = delay;
    msg.extraData = extraData;

    _RouteMessage(&msg, delay > 0 ? MMP_TRUE : MMP_FALSE);
}

//=============================================================================
/**
 * Send an asynchronous message from sender module to receiver module
 *
 * @param name              The name of message
 * @param extraData         The extra data to be sent with the message
 * @param sender            The sender module id
 * @param receiver          The receiver module id
 * @return                  none
 */
//=============================================================================
void
SendAsyncMsg(
    MSG_NAME name,
    MMP_UINT32 extraData,
    MMP_UINT32 sender,
    MMP_UINT32 receiver)
{
    SendDelayedMsg(name, 1, extraData, sender, receiver);
}

//=============================================================================
/**
 * Send a message from sender module to receiver module
 *
 * @param name              The name of message
 * @param extraData         The extra data to be sent with the message
 * @param sender            The sender module id
 * @param receiver          The receiver module id
 * @return                  none
 */
//=============================================================================
void
SendMsg(
    MSG_NAME name,
    MMP_UINT32 extraData,
    MMP_UINT32 sender,
    MMP_UINT32 receiver)
{
    MSG_OBJECT msg;
    msg.name = name;                    //The name of the message
    msg.sender_id = sender;             //The sender
    msg.receiver_id = receiver;         //The receiver
    msg.delivery_time = PalGetClock();  //Send the message NOW
    msg.delay = 0;
    msg.extraData = extraData;

    _RouteMessage(&msg, MMP_FALSE);
}

//=============================================================================
//                              Private Function Definition
//=============================================================================

//=============================================================================
/**
 * Routing a message
 *
 * @param msg           The message to be routed
 * @param forceStore    MMP_TRUE means force to store the message into the
 *                      delayed message queue.
 * @return none
 */
//=============================================================================
void
_RouteMessage(
    MSG_OBJECT* ptMsg,
    MMP_BOOL    forceStore)
{
    if (forceStore
     || (0 < ptMsg->delay
         && PalGetDuration(ptMsg->delivery_time) < ptMsg->delay))
    {   // This message needs to be stored until its time to send it
        _StoreDelayedMessage(ptMsg);
        return;
    }

    _RouteMessageHelper(ptMsg);
}

MMP_BOOL
_RouteMessageHelper(
    MSG_OBJECT* ptMsg)
{
    if (gtpfProcessMsg)
        gtpfProcessMsg(ptMsg);
    return MMP_TRUE;
}

//=============================================================================
/**
 * Store this message (in some data structure) for later routing
 * Note: Call SendDelayedMessages() every tick to check if its time
 *       to send the stored messages
 *
 * @param msg   The message to be stored
 * @return none
 */
//=============================================================================
void
_StoreDelayedMessage(
    MSG_OBJECT* ptMsg)
{
    if (gtDelayedMessageQueue.tMutex)
    {
        DELAYED_MESSAGE* ptCurMsg   = MMP_NULL;
        MMP_BOOL hasSameMsg         = MMP_FALSE;
        MMP_UINT emptyMsgIndex      = DELAYED_MESSAGE_COUNT;
        MMP_UINT i = 0;

        PalWaitMutex(gtDelayedMessageQueue.tMutex, PAL_MUTEX_INFINITE);
        if (gtDelayedMessageQueue.delayedMessages)
        {
            for (i = 0; i < DELAYED_MESSAGE_COUNT; ++i)
            {
                ptCurMsg = &gtDelayedMessageQueue.tDelayedMessage[i];
                if (MSG_NULL != ptCurMsg->tMsg.name)
                {
                    if (ptCurMsg->tMsg.name        == ptMsg->name
                     && ptCurMsg->tMsg.receiver_id == ptMsg->receiver_id
                     && ptCurMsg->tMsg.sender_id   == ptMsg->sender_id )
                    {
                        hasSameMsg = MMP_TRUE;
                        break;
                    }
                }
                else
                {
                    // If we find an empty delayed message slot.
                    if (DELAYED_MESSAGE_COUNT <= emptyMsgIndex)
                    {
                        emptyMsgIndex = i;
                    }
                }
            }
        }
        else
        {
            emptyMsgIndex = 0;
        }

        if ((!hasSameMsg) && (emptyMsgIndex < DELAYED_MESSAGE_COUNT))
        {
            PalMemcpy(
                &gtDelayedMessageQueue.tDelayedMessage[emptyMsgIndex].tMsg,
                ptMsg,
                sizeof(*ptMsg));
            gtDelayedMessageQueue.delayedMessages |= (1 << emptyMsgIndex);
        }
        PalReleaseMutex(gtDelayedMessageQueue.tMutex);
    }
}
