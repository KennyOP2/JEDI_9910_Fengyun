#ifndef CRC32_H
#define CRC32_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Function Declaration
//=============================================================================
unsigned int
cksum(
    unsigned char const *buf,
    unsigned long length);

#ifdef __cplusplus
}
#endif

#endif // end of CRC32_H