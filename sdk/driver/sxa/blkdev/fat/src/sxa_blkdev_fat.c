#include    "pal/pal.h"
#include    "sxa/sxa_blkdev.h"
#include    "common/fat.h"


extern PAL_CLOCK_T          t_read  = 0;
extern PAL_CLOCK_T          t_write = 0;
extern PAL_CLOCK_T          t_seek  = 0;
extern int                  n_read  = 0;
extern int                  n_write = 0;
extern int                  n_seek  = 0;

//#define     LOOPFILE        L"C:/NTFS.IMG"
//#define     LOOPFILE        L"C:/ntfs-128mb.img"
#define     LOOPFILE        L"C:/sdb1.img"

int
sxa_blkdev_get_info(
    unsigned int                devnode,
    struct SXA_BLKDEV_INFO*     info)
{
    extern F_DRIVER*        sd_initfunc(unsigned long driver_param);
    extern F_DRIVER*        m_pSDDriver;
    F_PARTITION             partitionInfo[4];
    int                     i;
    unsigned long           start_sec_num = 0;
    

    memset(info, 0, sizeof(*info));

    if (SXA_NTFS_GETDEVICE(devnode) == PAL_DEVICE_TYPE_SD)
    {
        if (!m_pSDDriver)
        {
            f_createdriver(&m_pSDDriver, sd_initfunc, F_AUTO_ASSIGN);
        }

        if (m_pSDDriver && !f_getpartition(m_pSDDriver, sizeof(partitionInfo) / sizeof(partitionInfo[0]), partitionInfo))
        {
            for (i = 0; i < sizeof(partitionInfo) / sizeof(partitionInfo[0]); i++)
            {
                if (partitionInfo[i].secnum)
                {
                    info->partitions[info->num_partitions].start_sector = start_sec_num;
                    info->partitions[info->num_partitions].num_sectors  = partitionInfo[i].secnum;
                    info->partitions[info->num_partitions].type         = partitionInfo[i].system_indicator;
                    info->partitions[info->num_partitions].sector_size  = 512;
                    info->num_partitions++;
                }

                start_sec_num += partitionInfo[i].secnum;
            }
        }
    }

    return (info->num_partitions >= 0) ? 0 : -1;
}

int
sxa_blkdev_open(
    unsigned int            devnode,
    unsigned int            flags)
{
    PAL_FILE*               fp;
    unsigned int            pos;

    fp = PalWFileOpen(LOOPFILE, PAL_FILE_RBP, NULL);

    #if (0)
    PalFileSeek(fp, 0, 2, NULL);
    pos = PalFileTell(fp, NULL);
    printf("fp = %02x, pos = %d\r\n", ((unsigned int *)fp)[1], pos);
    PalFileSeek(fp, 125928448/*249970688*/, 0, NULL);
    pos = PalFileTell(fp, NULL);
    printf("fp = %02x, pos = %d\r\n", ((unsigned int *)fp)[1], pos);
    PalFileSeek(fp, 0, 0, NULL);
    pos = PalFileTell(fp, NULL);
    printf("fp = %02x, pos = %d\r\n", ((unsigned int *)fp)[1], pos);
    #endif

    return  (fp ? (int) fp : -1);
}

int 
sxa_blkdev_close(
    int                     fd)
{
    return PalFileClose((PAL_FILE *) fd, NULL);
}


unsigned long long
sxa_blkdev_lseek(
    int                     fd,
    signed long long        offset,
    int                     whence)
{
    PalFileSeek((PAL_FILE *)fd, offset, whence, NULL);
    return PalFileTell((PAL_FILE *)fd, NULL);
}

int
sxa_blkdev_read(
    int                     fd,
    void*                   buf,
    int                     count)
{
    return PalFileRead(buf, 1, count, (PAL_FILE *)fd, NULL);
}

int
sxa_blkdev_write(
    int                     fd,
    const void*             buf,
    int                     count)
{
    unsigned int t0;
    unsigned int d1;
    int          ret;

    //t0 = PalGetClock();
    ret = PalFileWrite(buf, 1, count, (PAL_FILE *)fd, NULL);
    //d1 = PalGetDuration(t0);

    //printf("    >> sxa_blkdev_write(%d) - %d\r\n", (int) count, (int) d1);

    return ret;
}

int
sxa_blkdev_pread(
    int                     fd,
    void*                   buf,
    int                     count,
    signed long long        pos)
{
    unsigned int t0;
    unsigned int d1;
    unsigned int d2;
    int          ret;

    t0 = PalGetClock();
    PalFileSeek((PAL_FILE *)fd, pos, PAL_SEEK_SET, NULL);
    d1 = PalGetDuration(t0);
 
    t0 = PalGetClock();
    ret = PalFileRead(buf, 1, count, (PAL_FILE *)fd, NULL);
    d2 = PalGetDuration(t0);

    n_seek++, t_seek += d1;
    n_read++, t_read += d2;
    //printf("    >> sxa_blkdev_pread(%d, %d) - %d, %d, %d\r\n", (int) count, (int) pos, (int) d1, (int) d2, d1 + d2);
 
    return ret;
}

int
sxa_blkdev_pwrite(
    int                     fd,
    const void*             buf,
    int                     count,
    signed long long        pos)
{
    unsigned int t0;
    unsigned int d1;
    unsigned int d2;
    int          ret;

    static       char       tmp[7];
    static       int        pos0;
    static       int        cmp;

    t0 = PalGetClock();
    PalFileSeek((PAL_FILE *)fd, pos, PAL_SEEK_SET, NULL);
    d1 = PalGetDuration(t0);
    //pos0 = PalFileTell((PAL_FILE *)fd, NULL);
 
    t0 = PalGetClock();
    ret = PalFileWrite(buf, 1, count, (PAL_FILE *)fd, NULL);
    d2 = PalGetDuration(t0);

    n_seek++,  t_seek  += d1;
    n_write++, t_write += d2;

    //printf("    >> sxa_blkdev_pwrite(%d, %d) - %d, %d, %d\r\n", (int) count, (int) pos, (int) d1, (int) d2, d1 + d2);

    /*
    if (ret >= 7)
    {
        memset(tmp, 0xff, sizeof(tmp)); 
        PalFileSeek((PAL_FILE *)fd, pos, PAL_SEEK_SET, NULL);
        pos0 = PalFileTell((PAL_FILE *)fd, NULL);
        count = PalFileRead(tmp, 1, 7, (PAL_FILE *)fd, NULL); 
        PalFileSeek((PAL_FILE *)fd, pos + ret, PAL_SEEK_SET, NULL);
        cmp = memcmp(buf, tmp, 7);
        if (cmp)
        {
            printf("CMP[X] %08X %d %d\r\n", buf, (int)count, (int)pos);
        }
    } 
    */

    return ret;
}
    
int
sxa_blkdev_sync(
    int                     fd)
{
    return PalFileFlush((PAL_FILE *)fd, NULL);
}

int
sxa_blkdev_stat(
    int                     fd,
    struct stat*            buf)
{
    return 0;
}

int
sxa_blkdev_ioctl(
    int                     fd,
    int                     request,
    void*                   arg)
{
    return 0;
}
