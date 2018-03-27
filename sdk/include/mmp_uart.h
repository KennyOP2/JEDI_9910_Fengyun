#ifndef MMP_UART_H 
#define MMP_UART_H 

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
	#if defined(UART_EXPORTS)
		#define UART_API __declspec(dllexport)
	#else
		#define UART_API __declspec(dllimport)
	#endif
#else
	#define UART_API extern
#endif

#include "mmp_types.h"

//
#define UART_PARITY_NONE		0
#define UART_PARITY_ODD		    1
#define UART_PARITY_EVEN		2
#define UART_PARITY_MARK		3
#define UART_PARITY_SPACE	    4

//error code
#define UART_CREATE_EVENT_FAIL  (-2)
#define UART_ALLOCATE_MEM_FAIL  (-3)

typedef enum 
{
    UART1,
    UART2
}URAT_PORT;

/**
 * Initialize UART module.
 *
 * @param port UART1 or UART2.
 * @param baudrate the baud rate.
 * @param parity the parity .
 * @param numofStop the number of stop.
 * @param len the char len.
 
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
UART_API MMP_RESULT mmpUartInitialize(URAT_PORT port, MMP_UINT32 baudrate, MMP_UINT32 parity,MMP_UINT32 numofStop,MMP_UINT32 len);

/**
 * Terminate UART module.
 *
 * @param port UART1 or UART2.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
UART_API MMP_RESULT mmpUartTerminate(URAT_PORT port);

/**
 * Get Character by UART module.
 *
 * @param port UART1 or UART2.
 * @param pout buffer to get the Character.
 
 * @return true is successful.
 */
UART_API MMP_BOOL mmpUartGetChar(URAT_PORT port, char *pout);

/**
 * Put Character by UART module.
 *
 * @param port UART1 or UART2.
 * @param ch the Character.
 *
 * @return.
 */
UART_API void mmpUartPutChar(URAT_PORT port, char ch);

UART_API void mmpUartSetLoopback(URAT_PORT port, MMP_UINT32 onoff);


UART_API void mmpUartPutChars(URAT_PORT port, char *chs, MMP_UINT size);

UART_API MMP_BOOL mmpUartGetChars(URAT_PORT port, char *pout, MMP_UINT size);

UART_API MMP_INT mmpUartInterruptGetChars(URAT_PORT port, char *pout, MMP_UINT32 MaxSize);

#endif
