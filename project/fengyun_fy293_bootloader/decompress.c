#include "global.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* magic file header for compressed files */
static const MMP_UINT8 magic[4] = { 'S', 'M', 'A', 'Z' };

#ifndef __FREERTOS__
#define DATA_SWAP32(n) \
    ( (((n) >> 24) & 0x000000FF) | \
      (((n) >>  8) & 0x0000FF00) | \
      (((n) <<  8) & 0x00FF0000) | \
      (((n) << 24) & 0xFF000000) )
#else
#define DATA_SWAP32(n) (n)
#endif

#define SAFE
#ifdef SAFE
#   ifdef __DEBUG__
#       define fail(x,r)    if (x) { printf("%s #%d\n", __FILE__, __LINE__); *dst_len = olen; return r; }
#   else
#       define fail(x,r)    if (x) { *dst_len = olen; return r; }
#   endif   // __DEBUG__
#else
#   define fail(x,r)
#endif  // SAFE

/* Thinned out version of the UCL 2e decompression sourcecode
 * Original (C) Markus F.X.J Oberhumer under GNU GPL license */

#define GETBYTE(src)  (src[ilen++])

#define GETBIT(bb, src) \
    (((bb = ((bb & 0x7f) ? (bb*2) : ((unsigned)GETBYTE(src)*2+1))) >> 8) & 1)

enum status {
    UCL_E_OK                =  0,
    UCL_E_INPUT_OVERRUN     = -0x1,
    UCL_E_OUTPUT_OVERRUN    = -0x2,
    UCL_E_LOOKBEHIND_OVERRUN= -0x3,
    UCL_E_OVERLAP_OVERRUN   = -0x4,
    UCL_E_INPUT_NOT_CONSUMED= -0x5
};

MMP_INT32 decompress(
    const MMP_UINT8* src, MMP_UINT32  src_len,
          MMP_UINT8* dst, MMP_UINT32* dst_len)
{
    MMP_UINT32 bb = 0;
    MMP_UINT32 ilen = 0, olen = 0, last_m_off = 1;

#if defined(SAFE)
    const MMP_UINT32 oend = *dst_len;
#endif

    for (;;) {
        MMP_UINT32 m_off, m_len;

        while (GETBIT(bb,src)) {
            fail(ilen >= src_len, UCL_E_INPUT_OVERRUN);
            fail(olen >= oend, UCL_E_OUTPUT_OVERRUN);
            dst[olen++] = GETBYTE(src);
        }

        m_off = 1;

        for (;;) {
            m_off = m_off*2 + GETBIT(bb,src);
            fail(ilen >= src_len, UCL_E_INPUT_OVERRUN);
            fail(m_off > 0xfffffful + 3, UCL_E_LOOKBEHIND_OVERRUN);
            if (GETBIT(bb,src)) {
                break;
            }
            m_off = (m_off-1)*2 + GETBIT(bb,src);
        }

        if (m_off == 2) {
            m_off = last_m_off;
            m_len = GETBIT(bb,src);
        } else {
            fail(ilen >= src_len, UCL_E_INPUT_OVERRUN);
            m_off = (m_off-3)*256 + GETBYTE(src);
            if (m_off == 0xfffffffful) {
                break;
            }
            m_len = (m_off ^ 0xfffffffful) & 1;
            m_off >>= 1;
            last_m_off = ++m_off;
        }

        if (m_len) {
            m_len = 1 + GETBIT(bb,src);
        } else if (GETBIT(bb,src)) {
            m_len = 3 + GETBIT(bb,src);
        } else {
            m_len++;
            do {
                m_len = m_len*2 + GETBIT(bb,src);
                fail(ilen >= src_len, UCL_E_INPUT_OVERRUN);
                fail(m_len >= oend, UCL_E_OUTPUT_OVERRUN);
            } while (!GETBIT(bb,src));
            m_len += 3;
        }

        m_len += (m_off > 0x500);
        fail(olen + m_len > oend, UCL_E_OUTPUT_OVERRUN);
        fail(m_off > olen, UCL_E_LOOKBEHIND_OVERRUN);

        {
            const MMP_UINT8* m_pos;
            m_pos = dst + olen - m_off;
            dst[olen++] = *m_pos++;
            do {
                dst[olen++] = *m_pos++;
            } while (--m_len > 0);
        }
    }

    *dst_len = olen;

    return (ilen == src_len) ? UCL_E_OK
                             : (ilen < src_len ? UCL_E_INPUT_NOT_CONSUMED
                                               : UCL_E_INPUT_OVERRUN);
}

MMP_INT32 do_decompress(
    MMP_UINT8 *InBuf,
    MMP_UINT8 *outBuf)
{
    MMP_INT32   r = 0;
    MMP_UINT8   *buf = NULL;
    MMP_UINT8   *pOutBuf;
    MMP_UINT32  buf_len;
    MMP_UINT32  block_size;
    MMP_UINT32  overhead = 0;
    MMP_UINT32  icon_header;
    //MMP_UINT32   OutSize;

    /*
     * Step 1: check magic header, read flags & block size, init checksum
     */
    //PalMemcpy(&icon_header, InBuf, sizeof(MMP_UINT8) * sizeof(MMP_INT32));
    //InBuf += sizeof(MMP_INT32);

    //icon_header = DATA_SWAP32(icon_header);

    //OutSize = icon_header;

    //pOutBuf = OutBuf = (MMP_UINT8 *) calloc(OutSize, sizeof(MMP_UINT8));
    pOutBuf = outBuf;

    PalMemcpy(&block_size, InBuf, sizeof(MMP_INT32) * 1);
    InBuf += sizeof(MMP_INT32);

    block_size = DATA_SWAP32(block_size);

    overhead   = block_size / 8 + 256;

    if (overhead == 0)
    {
        printf("header error - invalid header\n");
        r = 2;
        goto err;
    }

    if (block_size < 1024 || block_size > 8*1024*1024L)
    {
        printf("header error - invalid block size %ld\n",
            (long) block_size);
        r = 3;
        goto err;
    }

    printf("block-size is %ld bytes\n", (long)block_size);
    mmpWatchDogRefreshTimer();

    /*
     * Step 2: allocate buffer for in-place decompression
     */
    buf_len = (block_size + overhead + 3) & (~3l);
    buf = (MMP_UINT8*) MALLOC(buf_len);
    if (buf == NULL)
    {
        printf("out of memory\n");
        r = 5;
        goto err;
    }
    mmpWatchDogRefreshTimer();

    /*
     * Step 3: process blocks
     */
    for (;;)
    {
        MMP_UINT8* in;
        MMP_UINT8* out;
        MMP_UINT32 in_len;
        MMP_UINT32 out_len;

        mmpWatchDogRefreshTimer();
        /* read uncompressed size */
        PalMemcpy(&out_len, InBuf, sizeof(MMP_INT32) * 1);
        InBuf += sizeof(MMP_INT32);

        out_len = DATA_SWAP32(out_len);

        /* exit if last block (EOF marker) */
        if (out_len == 0)
            break;

        /* read compressed size */
        PalMemcpy(&in_len, InBuf, sizeof(MMP_INT32) * 1);
        InBuf += sizeof(MMP_INT32);
        in_len = DATA_SWAP32(in_len);

        /* sanity check of the size values */
        if (in_len > block_size || out_len > block_size ||
            in_len == 0 || in_len > out_len)
        {
            printf("block size error - data corrupted\n");
            r = 6;
            goto err;
        }

        /* place compressed block at the top of the buffer */
        in = buf + buf_len - ((in_len+3)&~3l);
        out = buf;

        /* read compressed block data */
        PalMemcpy(in, InBuf, sizeof(char) * in_len);
        InBuf += (sizeof(char) * in_len);

        if (in_len < out_len)
        {
            /* decompress - use safe decompressor as data might be corrupted */
            MMP_UINT32 new_len = out_len;

            r = decompress(in, in_len, out, &new_len);
            if (r != UCL_E_OK || new_len != out_len)
            {
                printf("compressed data violation: error %d (%ld/%ld/%ld)\n", r, (long) in_len, (long) out_len, (long) new_len);
                r = 8;
                goto err;
            }
            /* write decompressed block */
            PalMemcpy(pOutBuf, out, out_len * sizeof(char));
            pOutBuf += (out_len * sizeof(char));
        }
        else
        {
            /* write original (incompressible) block */
            PalMemcpy(pOutBuf, in, in_len * sizeof(char));
            pOutBuf += (in_len * sizeof(char));
        }
    }

err:
    FREE(buf);
    //PalFree(OutBuf);
    return r;
}


