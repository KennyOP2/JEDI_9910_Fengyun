#if !defined(__SXA_BLKDEV_H__)
#define __SXA_BLKDEV_H__

#include    <sys/stat.h>
#include    "ntfs-3g/inc/sxa.h"

#define SXA_BLKDEV_PARTITION_TYPE_NTFS          0x07


/*
struct SXA_BLKDEV_INFO
{
    int                     num_partitions;
    struct
    {
        unsigned int        type;                   // TBD - should be the partition type
        unsigned int        start_sector;           //
        unsigned int        num_sector;             //
        unsigned int        sector_size;            // 
    } partitions[9];
};
*/

#define SXA_NTFS_MKDEVNODE(blkdev, partition)           (((blkdev) << 16) | partition)
#define SXA_NTFS_GETDEVICE(devnode)                     (((devnode) >> 16) & 0x0000ffff)
#define SXA_NTFS_GETPARTITION(devnode)                  ((devnode) & 0x0000ffff)

#define SXA_MAX_PARTITION  10

typedef struct SXA_VOLUME_TAG {
    unsigned int devnode;
    unsigned int partnode;
	unsigned int flags;
	signed long long offset;
} SXA_VOLUME;

typedef struct {
	unsigned int type;
	unsigned int flag;
	unsigned long long start_sector;
	unsigned long long num_sectors;
	unsigned int sector_size;
	SXA_VOLUME* vol;
} SXA_PARTITION;

typedef struct SXA_BLKDEV_INFO {
	int num_partitions;
    SXA_PARTITION partitions[SXA_MAX_PARTITION];
} SXA_BLKDEV_INFO;

int
sxa_blkdev_get_info(
    unsigned int                devnode,
    struct SXA_BLKDEV_INFO*     info);

/**
 * DESCRIPTION
 *		open a bock device
 *
 * INPUT(S)
 *		devnode		HIWORD(dev) is PAL_DEVICE_TYPE, LOWORD(dev) is the partition number (one-based)
 *		flags			reeserved and should be set to 0 for now.
 *
 * RETURN VALUE
 *		a file descriptor, or -1 if an error occured.
 */
int
sxa_blkdev_open(
    unsigned int            devnode,
    unsigned int            flags);

/**
 * DESCRIPTION
 *		close an opened file descriptor
 *
 * INPUT(S)
 *		fd				file descriptor
 *
 * RETURN VALUE
 *		0					on success
 *    negative	on any error 
 */
int 
sxa_blkdev_close(
    int                     fd);


/**
 * DESCRIPTION
 *		to position the offset of a open file
 *
 * INPUT(S)
 *		fd
 *    offset
 *		whence
 *
 * RETURN VALUE
 *		0					on success
 *    negative	on any error 
 */
unsigned long long
sxa_blkdev_lseek(
    int                     fd,
    signed long long        offset,
    int                     whence);

int
sxa_blkdev_read(
    int                     fd,
    void*                   buf,
    int                     count);

int
sxa_blkdev_write(
    int                     fd,
    const void*             buf,
    int                     count);

int
sxa_blkdev_pread(
    int                     fd,
    void*                   buf,
    int                     count,
    signed long long        pos);

int
sxa_blkdev_pwrite(
    int                     fd,
    const void*             buf,
    int                     count,
    signed long long        pos);
    
int
sxa_blkdev_sync(
    int                     fd);

int
sxa_blkdev_stat(
    int                     fd,
    struct stat*            buf);

int
sxa_blkdev_ioctl(
    int                     fd,
    int                     request,
    void*                   arg);
#endif /* !defined(__SXA_BLKDEV_H__) */
