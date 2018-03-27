#include "pal/pal.h"
#include "mps_system.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
static MPS_SYSTEM_HANDLE gtMpsSystem = { 0 };

//=============================================================================
//                              Private Function Declaration
//=============================================================================

static void
_MPS_SystemAddElement(
    MPS_ELEMENT* ptElement);

static void
_MPS_SystemAddConnector(
    MPS_CONNECTOR* ptConnector);

static void
_MPS_ElementAddConnector(
    MPS_ELEMENT*            ptElement,
    MPS_ELEMENT_ROLE        role,
    MPS_CONNECTOR_LIST_OBJ* ptListObj);

//=============================================================================
//                              Public Function Definition
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
mpsSys_GetHandle(void)
{
    return &gtMpsSystem;
}

//=============================================================================
/**
 * Release all allocated resource and then destroy the whole system
 *
 * @return  none
 */
//=============================================================================
void
mpsSys_DestroySystem(void)
{
    mpsSys_DestoryElementList();
    mpsSys_DestoryConnectorList();
    PalMemset(&gtMpsSystem, 0x0, sizeof(gtMpsSystem));
    dbg_msg(DBG_MSG_TYPE_ERROR, "The system is terminated!!\n");
}

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
    MPS_CMD_QUEUE_ID cmdId)
{
    MPS_ELEMENT* ptNewElement = MMP_NULL;
    ptNewElement = (MPS_ELEMENT*)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                              sizeof(MPS_ELEMENT));

    PalMemset(ptNewElement, 0x0, sizeof(MPS_ELEMENT));
    ptNewElement->id = id;
    ptNewElement->cmdQueueId = cmdId;
    ptNewElement->eventId = MPS_CTRL_EVENT_ID;
    ptNewElement->specialEventId = MPS_CTRL_SPECIAL_EVENT_ID;
    _MPS_SystemAddElement(ptNewElement);

    return ptNewElement;
}

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
    MPS_ELEMENT** pptElement)
{
    MPS_ELEMENT_LIST_OBJ* ptPreviousObj = MMP_NULL;
    MPS_ELEMENT_LIST_OBJ* ptCurrentObj = gtMpsSystem.ptFirstElementObj;

    if (MMP_NULL == ptCurrentObj)
        return MPS_ERROR_CODE_INVALID_INPUT;
    else
    {
        do
        {
            if (ptCurrentObj->ptElement == *pptElement)
                break;

            ptPreviousObj = ptCurrentObj;
            ptCurrentObj = ptCurrentObj->ptNext;
        } while (ptCurrentObj);

        // No matched element of the element list.
        if (MMP_NULL == ptCurrentObj)
            return MPS_ERROR_CODE_NO_MATCH_ELEMENT;

        // Remove the element from the element list
        if (ptPreviousObj)
        {
            ptPreviousObj->ptNext = ptCurrentObj->ptNext;
            if (ptCurrentObj->ptNext)
                ptCurrentObj->ptNext->ptPrevious = ptPreviousObj;
            else
                gtMpsSystem.ptLastElementObj = ptPreviousObj;
        }
        else // No any previous element obj.
        {
            if (ptCurrentObj->ptNext)
            {
                ptCurrentObj->ptNext->ptPrevious = MMP_NULL;
                gtMpsSystem.ptFirstElementObj = ptCurrentObj->ptNext;
            }
            else // All elements are deleted.
            {
                gtMpsSystem.ptFirstElementObj = MMP_NULL;
                gtMpsSystem.ptLastElementObj = MMP_NULL;
            }
        }

        // Release the allocated resource of the element, and list obj.
        mpsSys_UnHookConnectorList(*pptElement);
        PalHeapFree(PAL_HEAP_DEFAULT, *pptElement);
        *pptElement = MMP_NULL;
        PalHeapFree(PAL_HEAP_DEFAULT, ptCurrentObj);

        return MPS_ERROR_CODE_NO_ERROR;
    }
}

//=============================================================================
/**
 * Destroy the whole elment list from the player system
 *
 * @return  MPS_ERROR_CODE to notify if the operation is success or failed.
 */
//=============================================================================
MPS_ERROR_CODE
mpsSys_DestoryElementList(
    void)
{
    MPS_ERROR_CODE result = MPS_ERROR_CODE_NO_ERROR;
    while (gtMpsSystem.ptFirstElementObj)
    {
        if ((result = mpsSys_DeleteElement(&(gtMpsSystem.ptFirstElementObj->ptElement)))
         != MPS_ERROR_CODE_NO_ERROR)
            break;
    }
    if (MPS_ERROR_CODE_NO_ERROR == result)
    {
        gtMpsSystem.ptFirstElementObj = gtMpsSystem.ptLastElementObj
                                      = MMP_NULL;
    }
    return result;
}

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
    void)
{
    MPS_CONNECTOR* ptNewConnector = MMP_NULL;
    ptNewConnector = (MPS_CONNECTOR*)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                  sizeof(MPS_CONNECTOR));

    PalMemset(ptNewConnector, 0x0, sizeof(MPS_CONNECTOR));
    _MPS_SystemAddConnector(ptNewConnector);

    return ptNewConnector;
}

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
    MPS_ELEMENT_ID   id)
{
    MPS_ELEMENT_LIST_OBJ* ptCurrentObj = gtMpsSystem.ptFirstElementObj;

    while (ptCurrentObj)
    {
        if (id == ptCurrentObj->ptElement->id)
            return ptCurrentObj->ptElement;

        ptCurrentObj = ptCurrentObj->ptNext;
    }

    // No hit of the element list or the element list is empty.
    return MMP_NULL;
}

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
    MPS_CONNECTOR** pptConnector)
{
    MPS_CONNECTOR_LIST_OBJ* ptPreviousObj = MMP_NULL;
    MPS_CONNECTOR_LIST_OBJ* ptCurrentObj = gtMpsSystem.ptFirstConnectorObj;

    if (MMP_NULL == ptCurrentObj)
        return MPS_ERROR_CODE_INVALID_INPUT;
    else
    {
        do
        {
            if (ptCurrentObj->ptConnector == *pptConnector)
                break;

            ptPreviousObj = ptCurrentObj;
            ptCurrentObj = ptCurrentObj->ptNext;
        } while (ptCurrentObj);

        // No matched element of the element list.
        if (MMP_NULL == ptCurrentObj)
        {
            return MPS_ERROR_CODE_NO_MATCH_CONNECTOR;
        }
        else if (ptCurrentObj->ptConnector->refCount > 0)
        {
            return MPS_ERROR_CODE_CONNECTOR_IN_USE;
        }

        // The matched element is the first element of the list.
        if (MMP_NULL == ptPreviousObj)
            gtMpsSystem.ptFirstConnectorObj = ptCurrentObj->ptNext;
        else
            ptPreviousObj->ptNext = ptCurrentObj->ptNext;

        // Release the allocated resource of the element, and list obj.
        PalHeapFree(PAL_HEAP_DEFAULT, *pptConnector);
        *pptConnector = MMP_NULL;
        PalHeapFree(PAL_HEAP_DEFAULT, ptCurrentObj);

        return MPS_ERROR_CODE_NO_ERROR;
    }
}

//=============================================================================
/**
 * Destroy the whole connector list from the player system
 *
 * @return  MPS_ERROR_CODE to notify if the operation is success or failed.
 */
//=============================================================================
MPS_ERROR_CODE
mpsSys_DestoryConnectorList(void)
{
    MPS_ERROR_CODE result = MPS_ERROR_CODE_NO_ERROR;
    while (gtMpsSystem.ptFirstConnectorObj)
    {
        if ((result = mpsSys_DeleteConnector(&(gtMpsSystem.ptFirstConnectorObj->ptConnector)))
         != MPS_ERROR_CODE_NO_ERROR)
            break;
    }

    if (MPS_ERROR_CODE_NO_ERROR == result)
        gtMpsSystem.ptFirstConnectorObj = MMP_NULL;

    return result;
}

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
    MPS_CONNECTOR*      ptConnector)
{
    MPS_CONNECTOR_LIST_OBJ* ptObj = MMP_NULL;

    // Invalid Input.
    if (MMP_NULL == ptElement || MMP_NULL == ptConnector)
        return MPS_ERROR_CODE_INVALID_INPUT;

    if (MPS_SRC_ELEMENT == role)
        ptConnector->ptSrcElement = ptElement;
    else if (MPS_DEST_ELEMENT == role)
        ptConnector->ptDestElement = ptElement;
    else // Invalid Input.
        return MPS_ERROR_CODE_INVALID_INPUT;

    ptConnector->refCount++;
    ptObj = (MPS_CONNECTOR_LIST_OBJ*)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                  sizeof(MPS_CONNECTOR_LIST_OBJ));
    ptObj->ptConnector = ptConnector;
    _MPS_ElementAddConnector(ptElement, role, ptObj);
    return MPS_ERROR_CODE_NO_ERROR;
}

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
    MPS_CONNECTOR*      ptConnector)
{
    MPS_CONNECTOR_LIST_OBJ* ptCurrentObj = ptElement->ptSrcList;
    MPS_CONNECTOR_LIST_OBJ* ptPreviousObj = MMP_NULL;

    // Invalid Input.
    if (MMP_NULL == ptElement || MMP_NULL == ptConnector)
    {
        return MPS_ERROR_CODE_INVALID_INPUT;
    }

    if (ptConnector->ptSrcElement == ptElement)
        ptConnector->ptSrcElement = MMP_NULL;
    else if (ptConnector->ptDestElement == ptElement)
        ptConnector->ptDestElement = MMP_NULL;
    else // Invalid Input.
        return MPS_ERROR_CODE_INVALID_INPUT;

    ptConnector->refCount--;

    // Check whether the connector is in the src connector list.
    while (ptCurrentObj)
    {
        if (ptCurrentObj->ptConnector == ptConnector)
            break;
        ptPreviousObj = ptCurrentObj;
        ptCurrentObj = ptCurrentObj->ptNext;
    }

    // First connector object of the list
    if (MMP_NULL == ptPreviousObj && ptCurrentObj != MMP_NULL)
    {
        ptElement->ptSrcList = ptCurrentObj->ptNext;
        goto end;
    }
    else if (ptCurrentObj != MMP_NULL)
    {
        ptPreviousObj->ptNext = ptCurrentObj->ptNext;
        goto end;
    }

    ptCurrentObj = ptElement->ptDestList;
    ptPreviousObj = MMP_NULL;

    // Check whether the connector is in the dest connector list.
    while (ptCurrentObj)
    {
        if (ptCurrentObj->ptConnector == ptConnector)
            break;
        ptPreviousObj = ptCurrentObj;
        ptCurrentObj = ptCurrentObj->ptNext;
    }

    // First connector object of the list
    if (MMP_NULL == ptPreviousObj && ptCurrentObj != MMP_NULL)
    {
        ptElement->ptDestList = ptCurrentObj->ptNext;
        goto end;
    }
    else if (ptCurrentObj != MMP_NULL)
    {
        ptPreviousObj->ptNext = ptCurrentObj->ptNext;
        goto end;
    }
    else // No hit.
        return MPS_ERROR_CODE_NO_MATCH_CONNECTOR;

end:
    PalHeapFree(PAL_HEAP_DEFAULT, ptCurrentObj);
    return MPS_ERROR_CODE_NO_ERROR;
}

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
    MPS_ELEMENT*     ptElement)
{
    MPS_ERROR_CODE result = MPS_ERROR_CODE_NO_ERROR;
    while (ptElement->ptSrcList)
    {
        if ((result = mpsSys_UnHookConnector(ptElement, ptElement->ptSrcList->ptConnector))
         != MPS_ERROR_CODE_NO_ERROR)
            break;
    }
    while (ptElement->ptDestList)
    {
        if ((result = mpsSys_UnHookConnector(ptElement, ptElement->ptDestList->ptConnector))
         != MPS_ERROR_CODE_NO_ERROR)
            break;
    }
    return result;
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
//=============================================================================
/**
 * Add a element including sorting into the player system.
 *
 * @param ptElement    The pointer to a element object.
 * @return             none.
 */
//=============================================================================
static void
_MPS_SystemAddElement(
    MPS_ELEMENT* ptElement)
{
    MPS_ELEMENT_LIST_OBJ* ptPreviousObj = MMP_NULL;
    MPS_ELEMENT_LIST_OBJ* ptNewObj = MMP_NULL;
    MPS_ELEMENT_LIST_OBJ* ptCurrentObj = gtMpsSystem.ptFirstElementObj;

    ptNewObj = (MPS_ELEMENT_LIST_OBJ*)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                   sizeof(MPS_ELEMENT_LIST_OBJ));
    ptNewObj->ptElement = ptElement;

    if (MMP_NULL == ptCurrentObj)
    {
        gtMpsSystem.ptFirstElementObj = ptNewObj;
        gtMpsSystem.ptLastElementObj = ptNewObj;
        gtMpsSystem.ptFirstElementObj->ptNext = MMP_NULL;
        gtMpsSystem.ptFirstElementObj->ptPrevious = MMP_NULL;
    }
    else // find the insertion position.
    {
        do
        {
            if (ptCurrentObj->ptElement->id < ptNewObj->ptElement->id)
            {
                ptPreviousObj = ptCurrentObj;
                ptCurrentObj = ptCurrentObj->ptNext;
            }
            else if (ptCurrentObj->ptElement->id > ptNewObj->ptElement->id)
            {
                if (ptPreviousObj)
                {
                    ptNewObj->ptPrevious = ptPreviousObj;
                    ptPreviousObj->ptNext = ptNewObj;
                }
                else
                {
                    ptNewObj->ptPrevious = MMP_NULL;
                    gtMpsSystem.ptFirstElementObj = ptNewObj;
                }

                if (ptCurrentObj)
                {
                    ptNewObj->ptNext = ptCurrentObj;
                    ptCurrentObj->ptPrevious = ptNewObj;
                }
                else
                {
                    ptNewObj->ptNext = MMP_NULL;
                    gtMpsSystem.ptLastElementObj = ptNewObj;
                }
                return;
            }
            else // ptCurrentObj->ptElement->id == ptNewObj->ptElement->id
            {
                // Invalid case with two element own same id
                dbg_msg(DBG_MSG_TYPE_ERROR, "mps_system.c(%d) The element Id shouldn't be duplicated!!\n",
                          __LINE__);
                PalAssert(0);
            }
        } while (ptCurrentObj);

        ptPreviousObj->ptNext = ptNewObj;
        ptNewObj->ptNext = MMP_NULL;
        ptNewObj->ptPrevious = ptPreviousObj;
        gtMpsSystem.ptLastElementObj = ptNewObj;
    }
}

//=============================================================================
/**
 * Add a connector into the player system.
 *
 * @param ptConnector  The pointer to a connector object.
 * @return             none.
 */
//=============================================================================
static void
_MPS_SystemAddConnector(
    MPS_CONNECTOR* ptConnector)
{
    MPS_CONNECTOR_LIST_OBJ* ptLastObj = MMP_NULL;
    MPS_CONNECTOR_LIST_OBJ* ptNewObj = MMP_NULL;
    MPS_CONNECTOR_LIST_OBJ* ptCurrentObj = gtMpsSystem.ptFirstConnectorObj;

    ptNewObj = (MPS_CONNECTOR_LIST_OBJ*)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                     sizeof(MPS_CONNECTOR_LIST_OBJ));
    ptNewObj->ptConnector = ptConnector;
    ptNewObj->ptNext = MMP_NULL;

    if (MMP_NULL == ptCurrentObj)
        gtMpsSystem.ptFirstConnectorObj = ptNewObj;
    else // find the last obj position.
    {
        do
        {
            ptLastObj = ptCurrentObj;
            ptCurrentObj = ptCurrentObj->ptNext;
        } while (ptCurrentObj);

        ptLastObj->ptNext = ptNewObj;
    }
}

//=============================================================================
/**
 * Add a connector into the specific element.
 *
 * @param ptElement The pointer to a element object.
 * @param role      The role is used to identify the connector object belongs
 *                  to which connector list.
 * @param ptListObj The pointer to a connector list object of the element.
 * @return          none.
 */
//=============================================================================
static void
_MPS_ElementAddConnector(
    MPS_ELEMENT*            ptElement,
    MPS_ELEMENT_ROLE        role,
    MPS_CONNECTOR_LIST_OBJ* ptListObj)
{
    MPS_CONNECTOR_LIST_OBJ* ptLastObj = MMP_NULL;
    MPS_CONNECTOR_LIST_OBJ* ptCurrentObj = MMP_NULL;

    if (MPS_SRC_ELEMENT == role)
        ptCurrentObj = ptElement->ptDestList;
    else
        ptCurrentObj = ptElement->ptSrcList;

    if (MMP_NULL == ptCurrentObj)
    {
        if (MPS_SRC_ELEMENT == role)
            ptElement->ptDestList = ptListObj;
        else
            ptElement->ptSrcList = ptListObj;

        ptListObj->ptNext = MMP_NULL;
    }
    else
    {
        do
        {
            ptLastObj = ptCurrentObj;
            ptCurrentObj = ptCurrentObj->ptNext;
        } while (ptCurrentObj);

        ptLastObj->ptNext = ptListObj;
        ptListObj->ptNext = MMP_NULL;
    }
}
