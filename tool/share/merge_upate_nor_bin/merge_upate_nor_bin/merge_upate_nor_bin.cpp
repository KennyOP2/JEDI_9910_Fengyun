// merge_upate_nor_bin.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "merge_upate_nor_bin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define APPEND_OFFSET (2 * 1024 * 1024)
#define APPEND_FILE_SIZE (4)
// The one and only application object

CWinApp theApp;

using namespace std;

int main(int argc, char* argv[])
{
	int nRetCode = 0;

   	FILE* pReadFile1 = 0;
    int fileOffset = 0;
   	FILE* pReadFile2 = 0;
	FILE* pOutputFile = 0;
    unsigned char* pReadBuffer1 = 0;
    unsigned char* pReadBuffer2 = 0;
    unsigned char* pWriteBuffer = 0;
    int readFile1Size = 0;
    int readFile2Size = 0;
    int writeFileSize = 0;
    int i = 0;
    pReadFile1 = fopen((char*) argv[1], "rb");
    
    if (pReadFile1 == 0)
    {
        printf("open input file 1 is failed: %s\n", argv[1]);
        return 0;
    }
    else
    {
        fseek(pReadFile1, 0, SEEK_END);
        readFile1Size = ftell(pReadFile1);
        fseek(pReadFile1, 0, SEEK_SET);
        pReadBuffer1 = (unsigned char*) malloc(readFile1Size);
        if (pReadBuffer1)
        {
            printf("Size of file 1 is %u bytes\n", readFile1Size);
            fread(pReadBuffer1, 1, readFile1Size, pReadFile1);
            fclose(pReadFile1);
        }
        else
        {
            printf("alloc buffer for read file 1 is failed\n");
            return 0;
        }
    }

    pReadFile2 = fopen((char*)argv[2], "rb");
    if (pReadFile2 == 0)
    {
        printf("open input file 2 is failed: %s\n", argv[2]);
        return 0;
    }
    else
    {
        fseek(pReadFile2, 0, SEEK_END);
        readFile2Size = ftell(pReadFile2);
        fseek(pReadFile2, 0, SEEK_SET);
        pReadBuffer2 = (unsigned char*) malloc(readFile2Size);
        if (pReadBuffer2)
        {
            printf("Size of file 2 is %u bytes\n", readFile2Size);
            fread(pReadBuffer2, 1, readFile2Size, pReadFile2);
            fclose(pReadFile2);
        }
        else
        {
            printf("alloc buffer for read file 2 is failed\n");
            return 0;
        }
    }
    pOutputFile = fopen((char*)argv[3], "wb");
    if (pOutputFile == 0)
    {
        printf("open output file 2 is failed: %s\n", argv[3]);
        return 0;
    }
    else
    {
        pWriteBuffer = (unsigned char*) malloc(APPEND_OFFSET + APPEND_FILE_SIZE + readFile2Size);
        if (pWriteBuffer)
        {
            memset(pWriteBuffer, 0x0, APPEND_OFFSET + APPEND_FILE_SIZE + readFile2Size);
            memcpy(pWriteBuffer, pReadBuffer1, readFile1Size);
            memcpy(&pWriteBuffer[APPEND_OFFSET], (unsigned char*) &readFile2Size, 4);
            memcpy(&pWriteBuffer[APPEND_OFFSET + 4], pReadBuffer2, readFile2Size);
            fwrite(pWriteBuffer, 1, APPEND_OFFSET + APPEND_FILE_SIZE + readFile2Size, pOutputFile);
            fclose(pOutputFile);
            free(pWriteBuffer);
            free(pReadBuffer2);
            free(pReadBuffer1);
        }
        else
        {
            printf("alloc buffer for output file is failed\n");
            return 0;
        }
    }

	return nRetCode;
}
