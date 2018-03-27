/*
 * Copyright (c) 2009 ITE Technology Corp. All Rights Reserved.
 */
/** @file msg_route.h
 *
 * @author I-Chun Lai
 */

#ifndef MSG_ROUTE_H
#define MSG_ROUTE_H

#include "msg_core.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef void
    (*PROCESS_MSG)(
        MSG_OBJECT* ptMsg);

//=============================================================================
//                              Function  Definition
//=============================================================================

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
extern void
SendMsg(
    MSG_NAME name,
    MMP_UINT32 extraData,
    MMP_UINT32 sender,
    MMP_UINT32 receiver);

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
extern void
SendAsyncMsg(
    MSG_NAME name,
    MMP_UINT32 extraData,
    MMP_UINT32 sender,
    MMP_UINT32 receiver);

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
extern void
SendDelayedMsg(
    MSG_NAME name,
    PAL_CLOCK_T delay,
    MMP_UINT32 extraData,
    MMP_UINT32 sender,
    MMP_UINT32 receiver);

//=============================================================================
/**
 * Check if there are delayed messages needed to be sent, and send them if
 * necessary.
 *
 * @return none
 */
//=============================================================================
extern void
SendDelayedMessages(
    void);

//=============================================================================
/**
 * Initialize the delayed message queue
 *
 * @return none
 */
//=============================================================================
extern void
InitDelayedMessages(
    void);

//=============================================================================
/**
 * Terminate the delayed message queue
 *
 * @return none
 */
//=============================================================================
extern void
TerminateDelayedMessages(
    void);

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
extern void
CancelDelayedMessages(
    MSG_NAME name,
    MMP_UINT32 receiver);

extern void
RegisterProcessMsgRoutine(
    PROCESS_MSG pfRoutine);

extern void 
SetDelayedMessagesProcessRoutine( 
   PROCESS_MSG pfRoutine);

extern PROCESS_MSG 
GetDelayedMessagesProcessRoutine( 
 void);

#ifdef __cplusplus
}
#endif
#endif
