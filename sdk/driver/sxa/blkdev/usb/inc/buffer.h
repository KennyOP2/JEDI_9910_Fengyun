#ifndef BUFFER_H
#define BUFFER_H

//#include <sys/stat.h>
#include "pal/pal.h"
#include "sxa/sxa_blkdev.h"

//#define SXA_WRITE_THROUGH

#define SXA_CACHE_LENGTH      64
#define SXA_CACHE_ASSOCIATION 16
#define SXA_MAX_DEVICE        (PAL_DEVICE_TYPE_USB7 - PAL_DEVICE_TYPE_USB0 + 1)
#define SXA_MAX_VOLUME        10
#define SXA_SECTOR_SIZE       512
#define SXA_SECTOR_SHIFT      9
#define SXA_PAGE_SIZE         4096
#define SXA_PAGE_SHIFT        12
#define SXA_SECTORS_PER_PAGE  SXA_PAGE_SIZE / SXA_SECTOR_SIZE
#define SXA_CACHE_THRESHOLD   1

#define SXA_RDWR_RETRY        3
#define SXA_MAX_PARTITION     10
#define SXA_MAX_TRANSFER_PAGE 64

/* these values for extended partition */
#define SXA_SYSIND_EXTWIN     0x0f
#define SXA_SYSIND_EXTDOS     0x05

typedef struct SXA_PAGE_TAG {
    struct SXA_PAGE_TAG *dirty_next;      // dirty link list
    struct SXA_PAGE_TAG *dirty_prev;      // dirty link list
    struct SXA_PAGE_TAG *ass_next;        // association link list
    struct SXA_PAGE_TAG *ass_prev;        // association link list
    unsigned int        device;
    unsigned long long  offset;
    unsigned int        dirty;
    unsigned int        lock;
    unsigned int        valid;
    unsigned char       *data;
} SXA_PAGE;

typedef struct {
    struct SXA_PAGE_TAG *top;      // pointer to the replace one
    struct SXA_PAGE_TAG *tail;     // pointer to the last replace one
    SXA_PAGE            page[SXA_CACHE_ASSOCIATION];
} SXA_CACHE_ENTRY;

typedef struct {
    SXA_CACHE_ENTRY item[SXA_CACHE_LENGTH];
} SXA_CACHE_ARRAY;

#if (0)
typedef struct SXA_VOLUME_TAG {
    unsigned int     devnode;
    unsigned int     partnode;
    unsigned int     flags;
    signed long long offset;
} SXA_VOLUME;

typedef struct {
    unsigned int       type;
    unsigned int       flag;
    unsigned long long start_sector;
    unsigned long long num_sectors;
    unsigned int       sector_size;
    SXA_VOLUME         *vol;
} SXA_PARTITION;

typedef struct {
    int           num_partitions;
    SXA_PARTITION partition[SXA_MAX_PARTITION];
} SXA_DEVICE;

int sxa_blkdev_get_info( unsigned int devnode, SXA_DEVICE *info);
int sxa_blkdev_open( unsigned int device, unsigned int flags); // HIWORD(dev) is PAL_DEVICE_TYPE, LOWORD(dev) is the partition number
int sxa_blkdev_close( int fd);
unsigned long long sxa_blkdev_lseek(int fd, signed long long offset, int whence);
int sxa_blkdev_read(int fd, void *buf, int count);
int sxa_blkdev_write( int fd, void *buf, int count);
int sxa_blkdev_pread(int fd, unsigned long long pos, void *buf, int count);
int sxa_blkdev_pwrite( int fd, unsigned long long pos, void *buf, int count);
int sxa_blkdev_sync( int fd);
int sxa_blkdev_stat(int fd, struct stat *buf);
int sxa_blkdev_ioctl( int fd, int request, void *arg);
#endif
#endif //BUFFER_H