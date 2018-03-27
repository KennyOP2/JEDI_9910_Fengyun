#include "pal/pal.h"
#include "usbdrv.h"
#include "buffer.h"

//#define LOG_TIME

#define HIWORD(x)   ((x>>16)&0xffff)
#define LOWORD(x)   (x&0xffff)
#define NTFS_OEM_ID (0x202020205346544eULL)  /* "NTFS    " */

static SXA_CACHE_ARRAY *cachep = 0;
static SXA_PAGE *dirty_head = 0;
static int volcount = 0;  // volume counter
static SXA_BLKDEV_INFO device_array[SXA_MAX_DEVICE] = {0};

/****************************************************************************
 * _get32bit
 * read char and change to 32bit int
 ***************************************************************************/
unsigned int _get32bit(void *ptr)
{
    unsigned char *cptr=(unsigned char*)ptr;
    unsigned long ret;

    ret=(unsigned long)cptr[3] & 0x00ff;    /* 0x00ff needed because of DSPs */
    ret<<=8;
    ret|=(unsigned long)cptr[2] & 0x00ff;
    ret<<=8;
    ret|=(unsigned long)cptr[1] & 0x00ff;
    ret<<=8;
    ret|=(unsigned long)cptr[0] & 0x00ff;

    return ret;
}

/****************************************************************************
 * _get64bit
 * read char and change to 64bit int
 ***************************************************************************/
unsigned long long int _get64bit(void *ptr)
{
    unsigned char *cptr=(unsigned char*)ptr;
    unsigned long long ret;

    ret =(unsigned long long)cptr[7] & 0x00ff;  /* 0x00ff needed because of DSPs */
    ret<<=8;
    ret|=(unsigned long long)cptr[6] & 0x00ff;
    ret<<=8;
    ret|=(unsigned long long)cptr[5] & 0x00ff;
    ret<<=8;
    ret|=(unsigned long long)cptr[4] & 0x00ff;
    ret<<=8;
    ret|=(unsigned long long)cptr[3] & 0x00ff;
    ret<<=8;
    ret|=(unsigned long long)cptr[2] & 0x00ff;
    ret<<=8;
    ret|=(unsigned long long)cptr[1] & 0x00ff;
    ret<<=8;
    ret|=(unsigned long long)cptr[0] & 0x00ff;

    return ret;
}

/****************************************************************************
 * _drvreadsector
 * read a sector from device without consider partition
 * RETURN error code
 ***************************************************************************/
static int _drvreadsector(int devnode, void *data, unsigned long long sector)
{
    int retry=SXA_RDWR_RETRY;
    while (retry--)
    {
        /* read sector */
        int ret=usb_readmultiplesector(devnode, data, sector, 1);
        if (!ret) return 0;
    }

    return -1;  /* error after retrying finished */
}

/****************************************************************************
 * _readmultisector
 * read a sector from device of partition
 * RETURN error code
 ***************************************************************************/
static int _readmultipage(int devnode, int partnode, void *data, unsigned long long offset, int count)
{
    int retry=SXA_RDWR_RETRY;
    unsigned long long start_sector, num_sectors, sector;

    if (!count) return 0;
    if (!device_array[devnode].num_partitions) return -1;

    start_sector = device_array[devnode].partitions[partnode].start_sector;
    num_sectors = device_array[devnode].partitions[partnode].num_sectors;
    sector = start_sector + offset*SXA_SECTORS_PER_PAGE;
    count *= SXA_SECTORS_PER_PAGE;
    if((sector+count)>(start_sector+num_sectors)) return -1;

    while (retry--)
    {
        /* read sector */
        int ret=usb_readmultiplesector(devnode, data, sector, count);
        if (!ret) return 0;
    }

    return -1;  /* error after retrying finished */
}

/****************************************************************************
 * _drvwritesector
 * write a sector from device without consider partition
 * RETURN error code
 ***************************************************************************/
/*
static int _drvwritesector(int devnode, void *data, unsigned long sector)
{
    int retry=SXA_RDWR_RETRY;
    while (retry--)
    {
        int ret=usb_writemultiplesector(devnode, data, sector, 1);
        if (!ret) return 0;
    }

    return -1;
}
*/

/****************************************************************************
 * _writemultisector
 * write a sector from device of partition
 * RETURN error code
 ***************************************************************************/
static int _writemultisector(int devnode, int partnode, void *data, unsigned long long offset, int count)
{
    int retry=SXA_RDWR_RETRY;
    unsigned long long start_sector, num_sectors, sector;

    if (!count) return 0;
    if (!device_array[devnode].num_partitions) return -1;
    start_sector = device_array[devnode].partitions[partnode].start_sector;
    num_sectors = device_array[devnode].partitions[partnode].num_sectors;
    sector = start_sector + offset;
    count /= SXA_SECTOR_SIZE;
    if((sector+count*SXA_SECTORS_PER_PAGE)>(start_sector+num_sectors)) return -1;
    while (retry--)
    {
#ifdef LOG_TIME
        int                     t0, t1;

        /* write sector */
        t0 = PalGetClock();
#endif // LOG_TIME
        int ret=usb_writemultiplesector(devnode, data, sector, count);
#ifdef LOG_TIME
        t1 = PalGetDuration(t0);
        printf("usb write2\t%d %d 0x%X\n", t1, count, (unsigned int)sector);
#endif // LOG_TIME
        if (!ret) return 0;
    }

    return -1;  /* error after retrying finished */
}

static int _writemultipage(int devnode, int partnode, void *data, unsigned long long offset, int count)
{
    int retry=SXA_RDWR_RETRY;
    unsigned long long start_sector, num_sectors, sector;

    if (!count) return 0;
    if (!device_array[devnode].num_partitions) return -1;

    start_sector = device_array[devnode].partitions[partnode].start_sector;
    num_sectors = device_array[devnode].partitions[partnode].num_sectors;
    sector = start_sector + offset*SXA_SECTORS_PER_PAGE;
    count *= SXA_SECTORS_PER_PAGE;
    if((sector+count)>(start_sector+num_sectors)) return -1;

    while (retry--)
    {
#ifdef LOG_TIME
        int                     t0, t1;

        /* write sector */
        t0 = PalGetClock();
#endif // LOG_TIME
        //printf("17: %d 0x%X %d %d\r\n", devnode, data, (unsigned int)sector, count);
        int ret=usb_writemultiplesector(devnode, data, sector, count);
#ifdef LOG_TIME
        t1 = PalGetDuration(t0);
        printf("usb write\t%d %d 0x%X\n", t1, count, (unsigned int)sector);
#endif // LOG_TIME
        if (!ret) return 0;
    }

    return -1;  /* error after retrying finished */
}

/****************************************************************************
 * _cache_init
 * initial sector cache
 * RETURN cache array pointer
 ***************************************************************************/
static SXA_CACHE_ARRAY* _cache_init(void)
{
    int ret=0, i,j;
    SXA_CACHE_ARRAY *cp;

    // initialize cache
    cp = (SXA_CACHE_ARRAY*)PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(SXA_CACHE_ARRAY));
    if(!cp)
        goto err_out;

    PalMemset(cp, 0, sizeof(SXA_CACHE_ARRAY));
    dirty_head = 0;

    for(i=0;i<SXA_CACHE_LENGTH;i++)
    {
        for(j=0;j<SXA_CACHE_ASSOCIATION;j++)
        {
            cp->item[i].page[j].data = (unsigned char*)PalHeapAlloc(PAL_HEAP_DEFAULT, SXA_PAGE_SIZE);
            if(!cp->item[i].page[j].data)
                goto err_out;
        }
        for(j=0;j<SXA_CACHE_ASSOCIATION;j++)
        {
            cp->item[i].page[j].ass_next = &cp->item[i].page[(j+1)%SXA_CACHE_ASSOCIATION]; // link the assiciation
            cp->item[i].page[(j+1)%SXA_CACHE_ASSOCIATION].ass_prev = &cp->item[i].page[j]; // link the assiciation
        }
        cp->item[i].top = &cp->item[i].page[0]; // initial the replace item pointer
        cp->item[i].tail = &cp->item[i].page[SXA_CACHE_ASSOCIATION-1]; // initial the last replace item pointer
    }
    return cp;

err_out:
    if(cp)
    {
        for(i=0;i<SXA_CACHE_LENGTH;i++)
        {
            for(j=0;j<SXA_CACHE_ASSOCIATION;j++)
            {
                if(cp->item[i].page[j].data)
                    PalHeapFree(PAL_HEAP_DEFAULT, cp->item[i].page[j].data);
            }
        }
        PalHeapFree(PAL_HEAP_DEFAULT, cp);
        cp = 0;
    }
    return cp;
}

/****************************************************************************
 * _cache_term
 * terminate sector cache
 * RETURN no
 ***************************************************************************/
static void _cache_term(SXA_CACHE_ARRAY *cp)
{
    int i,j;
    if(cp)
    {
        for(i=0;i<SXA_CACHE_LENGTH;i++)
        {
            for(j=0;j<SXA_CACHE_ASSOCIATION;j++)
            {
                if(cp->item[i].page[j].data)
                    PalHeapFree(PAL_HEAP_DEFAULT, cp->item[i].page[j].data);
            }
        }
        PalHeapFree(PAL_HEAP_DEFAULT, cp);
    }
}

/****************************************************************************
 * _add_dirty
 * add the page to dirty link
 ***************************************************************************/
static void _add_dirty(SXA_PAGE *page)
{
    if (!page->dirty)
    {
        if(dirty_head != 0)
        {
            page->dirty_next = dirty_head;
            page->dirty_prev = 0;
            dirty_head->dirty_prev = page;
        }
        dirty_head = page;
        page->dirty = 1;
    }
}

/****************************************************************************
 * _remove_dirty
 * remove the link of the page
 * RETURN next page pointer
 ***************************************************************************/
static SXA_PAGE* _remove_dirty(SXA_PAGE *page)
{
    SXA_PAGE *retpage = 0;

    retpage = page->dirty_next;
    if(page->dirty_next && page->dirty_prev)
    {
        page->dirty_prev->dirty_next = page->dirty_next;
        page->dirty_next->dirty_prev = page->dirty_prev;
    }
    else if(page->dirty_next)
    {
        page->dirty_next->dirty_prev = 0;
        dirty_head = page->dirty_next;
    }
    else if(page->dirty_prev)
    {
        page->dirty_prev->dirty_next = 0;
    }
    else
        dirty_head = 0;

    page->dirty_next = 0;
    page->dirty_prev = 0;
    page->dirty = 0;

    return retpage;
}

/****************************************************************************
 * _writeback_cache_entry
 * write back dirty cache entry
 * RETURN error code
 ***************************************************************************/
static int _writeback_page(SXA_PAGE *page)
{
    int ret;
    int devnode, partnode;

    while(page->lock)
    {
        printf("wait unlock1\r\n");
        PalSleep(1); // wait unlocked
    }
    page->lock = 1;
    devnode = HIWORD(page->device);
    partnode = LOWORD(page->device);

//printf("10: %d %d 0x%X 0x%X\r\n", devnode, partnode, page->data, page->offset);

    ret = _writemultipage(devnode, partnode, page->data, page->offset, 1);
    page->lock = 0; // release lock

    return ret;
}

/****************************************************************************
 * _find_cache
 * find cache entry which meet the device and sector id
 * RETURN cache entry pointer
 ***************************************************************************/
static SXA_PAGE* _find_cache(int device, unsigned long long offset)
{
    int i;
    int index = offset & (SXA_CACHE_LENGTH-1);
    SXA_PAGE *page, *top, *tail;

    if (!cachep) return 0;

    top = cachep->item[index].top;
    tail = cachep->item[index].tail;
    for(i=0;i<SXA_CACHE_ASSOCIATION;i++)
    {
        page = &cachep->item[index].page[i];
        if(page->valid && page->device == device && page->offset == offset)
        {
            if(tail != page)
            {
                if(SXA_CACHE_ASSOCIATION==2)
                {
                    cachep->item[index].top = page->ass_next;  // only chage will be ok
                    cachep->item[index].tail = page;
                }
                else
                {
                    if(top == page)
                        cachep->item[index].top = top->ass_next;

                    page->ass_prev->ass_next = page->ass_next;   // remove page
                    page->ass_next->ass_prev = page->ass_prev;
                    page->ass_next = tail->ass_next;             // relink page to tail
                    page->ass_prev = tail;
                    page->ass_next = tail->ass_next;
                    tail->ass_next->ass_prev = page;
                    tail->ass_next = page;
                    cachep->item[index].tail = page;
                }
            }
            while(page->lock)
            {
                printf("wait unlock2\r\n");
                PalSleep(1); // wait unlocked
            }
            // TEST
            //printf("_find_cache=0X%08X\r\n", page);
            // TEST
            return page;
        }
    }
    // TEST
    //printf("_find_cache=0X%08X\r\n", 0);
    // TEST
    return 0;
}

/****************************************************************************
 * _alloc_cache
 * locate cache entry with sector index
 * RETURN cache entry pointer
 ***************************************************************************/
static SXA_PAGE* _alloc_cache(int device, unsigned long long offset)
{
    int index = offset & (SXA_CACHE_LENGTH-1);
    SXA_PAGE *page, *top, *tail;

    if (!cachep) return 0;

    top = cachep->item[index].top;
    tail = cachep->item[index].tail;
    page = top;

    while(page->lock)
    {
        printf("wait unlock3\r\n");
        PalSleep(1); // wait unlocked
    }

    if(SXA_CACHE_ASSOCIATION==2)
    {
        cachep->item[index].top = tail;
        cachep->item[index].tail = top;
    }
    else
    {
        cachep->item[index].top = top->ass_next;
        page->ass_prev->ass_next = page->ass_next;   // remove page
        page->ass_next->ass_prev = page->ass_prev;
        page->ass_next = tail->ass_next;             // relink page to tail
        page->ass_prev = tail;
        page->ass_next = tail->ass_next;
        tail->ass_next->ass_prev = page;
        tail->ass_next = page;
        cachep->item[index].tail = page;
    }

    if (page->dirty == 1)
    {
        //printf("_alloc_cache write\r\n");
        _writeback_page(page);
        _remove_dirty(page);
    }

    page->device = device;
    page->offset = offset;
    page->valid = 1;
    return page;
}

/****************************************************************************
 * _writeback_cache
 * write back dirty cache entries which meet the device/sectors,
 * the dirty entrys are all link togather with dirty_head pointer
 * RETURN error code
 ***************************************************************************/
static int _writeback_cache(int device, unsigned long long start_page, int count)
{
    int ret = 0;
    SXA_PAGE *page = dirty_head;

    //printf("_writeback_cache\r\n");

    while (page)
    {
        if(device==page->device && page->offset>=start_page && page->offset<start_page+count)
        {
            ret = _writeback_page(page);
            page = _remove_dirty(page);
        }
        else
            page = page->dirty_next;
    }
    return ret;
}

/****************************************************************************
 * _invalid_cache
 * invalid the cache entry which between start_sector and start_sector+count
 * to avoid miss-match while multi-sectors write (multi-sectors write will not
 * cache in cache_array
 * RETURN error code
 ***************************************************************************/
static int _invalid_cache(int device, unsigned long long start_page, int count)
{
    int ret = 0;
    int start_index = start_page & (SXA_CACHE_LENGTH-1);
    int i,j;
    SXA_PAGE *page = dirty_head;

    // check dirty list, if exist, remove it
    while (page)
    {
        // TEST
        if (page == page->dirty_next)
        {
            printf("cyclic link!!!\r\n");
        }
        // TEST
        if(device==page->device && page->offset>=start_page && page->offset<start_page+count)
            page = _remove_dirty(page);
        else
            page = page->dirty_next;
    }

    // check cache, if hit, invalid it
    if(count>=SXA_CACHE_LENGTH)  // size large than whole cache, compare all
    {
        for(i=0;i<SXA_CACHE_LENGTH;i++)
        {
            for(j=0;j<SXA_CACHE_ASSOCIATION;j++)
            {
                SXA_PAGE *page = &cachep->item[i].page[j];
                if(page->valid && page->device == device &&
                   page->offset >= start_page && page->offset <= start_page+count)
                    page->valid = 0;
            }
        }
    }
    else
    {
        if(start_index+count<SXA_CACHE_LENGTH)
        {
            for(i=start_index;i<start_index+count;i++)
            {
                for(j=0;j<SXA_CACHE_ASSOCIATION;j++)
                {
                    SXA_PAGE *page = &cachep->item[i].page[j];
                    if(page->valid && page->device == device &&
                       page->offset >= start_page && page->offset <= start_page+count)
                        page->valid = 0;
                }
            }
        }
        else
        {
            for(i=start_index;i<SXA_CACHE_LENGTH;i++)
            {
                for(j=0;j<SXA_CACHE_ASSOCIATION;j++)
                {
                    SXA_PAGE *page = &cachep->item[i].page[j];
                    if(page->valid && page->device == device &&
                       page->offset >= start_page && page->offset <= start_page+count)
                        page->valid = 0;
                }
            }
            for(i=0;i<start_index+count-SXA_CACHE_LENGTH;i++)
            {
                for(j=0;j<SXA_CACHE_ASSOCIATION;j++)
                {
                    SXA_PAGE *page = &cachep->item[i].page[j];
                    if(page->valid && page->device == device &&
                       page->offset >= start_page && page->offset <= start_page+count)
                        page->valid = 0;
                }
            }
        }
    }

    return ret;
}

/****************************************************************************
 * _getpartition
 * getting partition info from devnode
 * RETURN error code
 ***************************************************************************/
static int _getpartition(int devnode)
{
    unsigned char sectorbuffer[SXA_SECTOR_SIZE];
    unsigned char *ptr=sectorbuffer;
    unsigned long long extpartsec=0;
    unsigned long long extpartsecrel=0;
    unsigned long long sectorstart;
    unsigned long long sectornum;
    unsigned int sectorsize;
    int ret = 0;
    int i = 0;

    /* set 1st sector position */
    sectorstart=0;

    /* get its physical */
    ret=usb_getphy(devnode, &sectorsize, &sectornum);
    if (ret) return ret;
    if (!sectornum) return -1;

    for(;;)
    {
        /* read BootRecord or MasterBootRecord */
        ret=_drvreadsector(devnode, ptr,sectorstart);
        if (ret) return ret;

        /* 0x55aa must be at the end of sector */
        if ((ptr[0x1fe]!=0x55) || (ptr[0x1ff]!=0xaa))
        {
            return 0; /* no more signature found */
        }

        /* check if a boot record found */
        if ((ptr[0]==0xeb) || (ptr[0]==0xe9))
        {
            unsigned long long oem_id = _get64bit(&ptr[0x3]);
            if (oem_id == NTFS_OEM_ID)
            {
                SXA_BLKDEV_INFO *dev = &device_array[devnode];
                dev->partitions[0].num_sectors = sectornum;
                dev->partitions[0].sector_size = sectorsize;
                dev->partitions[0].start_sector = 0;
                dev->partitions[0].type = (unsigned int)7;
                dev->num_partitions = 1;
            }
            return 0; /* no more MBR found */
        }
        else
        {   /* check for MasterBootRecord */
            int partentry;
            int found=0;

            for (partentry=0; partentry<=0x30; partentry+=0x10) /* 4 times 16 bytes entry */
            {
                unsigned char sysind=ptr[partentry+0x04+0x1be];
                unsigned long long secstart;

                if (!sysind) continue;

                /* start,sizein sector for partition */
                secstart=_get32bit(&ptr [partentry+0x08+0x1be] );
                sectorstart=secstart+extpartsecrel+extpartsec; /* add relativ */
                sectornum  =_get32bit(&ptr [partentry+0x0C+0x1be] );


                /* check if extended */
                if (sysind==SXA_SYSIND_EXTWIN || sysind==SXA_SYSIND_EXTDOS)
                {
                    sectorstart=secstart+extpartsec; /* calc in this way */

                    found=1;

                    if (!extpartsec)
                    {
                        extpartsec=secstart; /* 1st extended partition */
                        extpartsecrel=0;    /* current relative position */
                    }
                    else
                    {
                        extpartsecrel=secstart; /* chained extended partition */
                    }

                    break; /* if extended partition is found stop searching next entry */
                }


                if (sectornum)
                {
                    SXA_BLKDEV_INFO *dev = &device_array[devnode];
                    dev->partitions[i].num_sectors = sectornum;
                    dev->partitions[i].sector_size = sectorsize;
                    dev->partitions[i].start_sector = sectorstart;
                    dev->partitions[i].type = (unsigned int)sysind;
                    i++;
                    dev->num_partitions = i;
                    if(i==SXA_MAX_PARTITION) return 0;
                }
            }
            if (!found) return 0; /* no more entry found */
        }
    }
    return 0;
}


#ifdef  CONFIG_SXA_DEBUG_PERFORMANCE
void            sxa_reset_rw_statistics();

#ifndef         SXA_RESET_RW_STATISTICS
#define         SXA_RESET_RW_STATISTICS()       reset_all_statistics()
#endif

#ifndef         SXA_RESET_START_TIME0
#define         SXA_RESET_START_TIME0()         do {t_start0 = PalGetClock();} while (0)
#endif

#ifndef         SXA_RESET_START_TIME
#define         SXA_RESET_START_TIME(x)         do {x = PalGetClock();} while (0)
#endif

#ifndef         SXA_ACC_TIME0
#define         SXA_ACC_TIME0(x)                do {x += PalGetDuration(t_start0);} while (0)
#endif

#ifndef         SXA_ACC_TIME
#define         SXA_ACC_TIME(x, y)              do {x += PalGetDuration(y);} while (0)
#endif

#ifndef         SXA_ACC_2
#define         SXA_ACC_2(x, y)                 do {x += y;} while (0)
#endif

extern int                  t_blkdev_print;
extern int                  t_volumeread_head;
extern int                  t_volumeread_body;
extern int                  t_volumeread_tail;
extern int                  n_volumeread_N;
extern int      t_volumewrite_head;
extern int      t_volumewrite_body;
extern int      t_volumewrite_tail;
extern int      n_volumewrite_N;
static int      t_start0;
#else
#undef          SXA_RESET_RW_STATISTICS
#define         SXA_RESET_RW_STATISTICS()       do {} while (0)

#undef          SXA_RESET_START_TIME0
#define         SXA_RESET_START_TIME0()         do {} while (0)

#undef          SXA_RESET_START_TIME
#define         SXA_RESET_START_TIME(x)         do {} while (0)

#undef          SXA_ACC_TIME0
#define         SXA_ACC_TIME0(x)                do {} while (0)

#undef          SXA_ACC_TIME
#define         SXA_ACC_TIME(x, y)              do {} while (0)

#undef          SXA_ACC_2
#define         SXA_ACC_2(x, y)                 do {} while (0)
#endif

/****************************************************************************
 * _volumeread
 * volume read
 * RETURN read size
 ***************************************************************************/
static int _volumeread(SXA_VOLUME *vol, void *buf, int count)
{
    int ret;
    int retsize = 0;
    int N, relpos, repeat = 1;
    unsigned long long page_offset, page_mask;
    SXA_PAGE *page;
    unsigned char *buffer = (unsigned char *)buf;
    int device = ((vol->devnode<<16) | (vol->partnode&0xffff));
    unsigned long long pos = vol->offset;
    unsigned int rdsize;


    page_mask = SXA_PAGE_SIZE - 1;

    while (count)
    {
        relpos = (int)(pos&page_mask);
        page_offset = pos >> SXA_PAGE_SHIFT;

        SXA_RESET_START_TIME0();

        if (relpos) // not page align, check in cache or not
        {
            page = _find_cache(device, page_offset);
            if(!page)
            {
                //printf("[A0]\r\n");
                page = _alloc_cache(device, page_offset);
                ret=_readmultipage(vol->devnode, vol->partnode, page->data, page_offset, 1);
                if (ret) return retsize;
            }
            else
            {
                //printf("[F0]\r\n");
            }
            if(count+relpos>SXA_PAGE_SIZE)
                rdsize = SXA_PAGE_SIZE - relpos;
            else
                rdsize = (unsigned int)count;
            PalMemcpy(buffer, &page->data[relpos], rdsize);

            SXA_ACC_TIME0(t_volumeread_head);
            SXA_ACC_2(n_volumeread_N, 1);
        }
        else  // align sector
        {
            if(count>=SXA_PAGE_SIZE)  // number of full page
            {
                // calculate number of page requested
                N=(unsigned int)(count>>SXA_PAGE_SHIFT);

                if(N<SXA_CACHE_THRESHOLD+1 && repeat==0)
                {
                    // cache all
                    int i;
                    for(i=0;i<N;i++)
                    {
                        page = _find_cache(device, page_offset+i);
                        if(!page)
                        {
                            page = _alloc_cache(device, page_offset+i);
                            ret=_readmultipage(vol->devnode, vol->partnode, page->data, page_offset+i, 1);
                            if (ret) return retsize;
                            //printf("[A1]\r\n");
                        }
                        else
                        {
                            //printf("[F1]\r\n");
                        }
                        PalMemcpy(buffer+i*SXA_PAGE_SIZE, page->data, SXA_PAGE_SIZE);
                    }
                    SXA_ACC_2(n_volumeread_N, i);
                }
                else
                {
                    //printf("[NO CACHE]\r\n");
                    // do not cache
                    if (N>SXA_MAX_TRANSFER_PAGE) N=SXA_MAX_TRANSFER_PAGE;
                    ret = _writeback_cache(device, page_offset, N);
                    if (ret) return retsize;

                    ret=_readmultipage(vol->devnode, vol->partnode, buffer, page_offset, N);
                    if (ret) return retsize;
                    repeat = 1;
                    SXA_ACC_2(n_volumeread_N, 1);
                }
                rdsize = N*SXA_PAGE_SIZE;
                SXA_ACC_TIME0(t_volumeread_body);
            }
            else // left part
            {
                page = _find_cache(device, page_offset);
                if(!page)
                {
                    //printf("[A2]\r\n");
                    page = _alloc_cache(device, page_offset);
                    ret=_readmultipage(vol->devnode, vol->partnode, page->data, page_offset, 1);
                    if (ret) return retsize;
                }
                else
                {
                    //printf("[F2]\r\n");
                }
                rdsize = (unsigned int)count;
                PalMemcpy(buffer, page->data, rdsize);
                SXA_ACC_TIME0(t_volumeread_tail);
                SXA_ACC_2(n_volumeread_N, 1);
            }
        }
        buffer += rdsize;
        count -= rdsize;
        pos += rdsize;
        retsize += rdsize;
    }
    vol->offset = pos;
    return retsize;
}

extern int                  t_volumewrite_head;
extern int                  t_volumewrite_body;
extern int                  t_volumewrite_tail;
extern int                  n_volumewrite_N;

/****************************************************************************
 * _volumewrite
 * volume write
 * RETURN write size
 ***************************************************************************/
static unsigned int _volumewrite(SXA_VOLUME *vol, void *buf, unsigned int count)
{
    int ret;
    int retsize = 0;
    int N, relpos, repeat = 0;
    unsigned long long page_offset, page_mask;
    SXA_PAGE *page;
    unsigned char *buffer = (unsigned char *)buf;
    int device = ((vol->devnode<<16) | (vol->partnode&0xffff));
    unsigned long long pos = vol->offset;
    unsigned int wrsize;

    page_mask = SXA_PAGE_SIZE - 1;

    while (count)
    {
        SXA_RESET_START_TIME0();

        relpos = (int)(pos&page_mask);
        page_offset = pos >> SXA_PAGE_SHIFT;

        if (relpos) // not sector align, check in cache or not
        {
            page = _find_cache(device, page_offset);
            if(!page)
            {
                //printf("[B0]\r\n");
#if 0
                if ((relpos % SXA_SECTOR_SIZE == 0) && (count % SXA_SECTOR_SIZE == 0))
                {
                    wrsize = (unsigned int)count;
                    //printf("11: %d %d 0x%X 0x%X %d %d\r\n", vol->devnode, vol->partnode, buffer, (unsigned int)page_offset*SXA_SECTORS_PER_PAGE+relpos/SXA_SECTOR_SIZE, count, relpos);
                    ret=_writemultisector(vol->devnode, vol->partnode, buffer, page_offset*SXA_SECTORS_PER_PAGE+relpos/SXA_SECTOR_SIZE, wrsize);
                    if(ret) return retsize;
                }
                else
#endif
                {
                    page = _alloc_cache(device, page_offset);
                    ret=_readmultipage(vol->devnode, vol->partnode, page->data, page_offset, 1);
                    if (ret) return retsize;

                    if(count+relpos>SXA_PAGE_SIZE)
                        wrsize = SXA_PAGE_SIZE - relpos;
                    else
                        wrsize = (unsigned int)count;
                    PalMemcpy(&page->data[relpos], buffer, wrsize);

#ifdef SXA_WRITE_THROUGH
                    ret = _writeback_page(page);
                    if(ret) return retsize;
#else
                    _add_dirty(page);
#endif
                }
            }
            else
            {
                //printf("[G0]\r\n");

                if(count+relpos>SXA_PAGE_SIZE)
                    wrsize = SXA_PAGE_SIZE - relpos;
                else
                    wrsize = (unsigned int)count;
                PalMemcpy(&page->data[relpos], buffer, wrsize);

#ifdef SXA_WRITE_THROUGH
                ret = _writeback_page(page);
                if(ret) return retsize;
#else
                _add_dirty(page);
#endif
            }
            SXA_ACC_TIME0(t_volumewrite_head);
            SXA_ACC_2(n_volumewrite_N, 1);
        }
        else  // align sector
        {
            if(count>=SXA_PAGE_SIZE)  // number of full sector
            {
                // calculate number of page requested
                N=(unsigned int)(count>>SXA_PAGE_SHIFT);

                if(N<SXA_CACHE_THRESHOLD+1 && repeat==0)
                {
                    // cache all
                    int i;
                    for(i=0;i<N;i++)
                    {
                        page = _find_cache(device, page_offset+i);
                        if(!page)
                        {
                            page = _alloc_cache(device, page_offset+i);
                            //printf("[B1]\r\n");
                        }
                        else
                        {
                            //printf("[G1]\r\n");
                        }

                        PalMemcpy(page->data, buffer+i*SXA_PAGE_SIZE, SXA_PAGE_SIZE);
#ifdef SXA_WRITE_THROUGH
                        ret = _writeback_page(page);
                        if(ret) return retsize;
#else
                        _add_dirty(page);
#endif
                    }
                    SXA_ACC_2(n_volumewrite_N, i);
                }
                else
                {
                    //printf("[NO CACHE2]\r\n");
                    // do not cache
                    if (N>SXA_MAX_TRANSFER_PAGE) N=SXA_MAX_TRANSFER_PAGE;
                    _invalid_cache(device, page_offset, N);
                    ret=_writemultipage(vol->devnode, vol->partnode, buffer, page_offset, N);
                    if (ret) return retsize;
                    repeat = 1;

                    SXA_ACC_2(n_volumewrite_N, 1);
                }
                wrsize = N*SXA_PAGE_SIZE;

                SXA_ACC_TIME0(t_volumewrite_body);
            }
            else // left part
            {
                page = _find_cache(device, page_offset);
                if(!page)
                {
                    //printf("[B2]\r\n");
#if 0
                    if (count % SXA_SECTOR_SIZE == 0)
                    {
                        wrsize = (unsigned int)count;
                        //printf("1: %d %d 0x%X 0x%X %d\r\n", vol->devnode, vol->partnode, buffer, (unsigned int)page_offset*SXA_SECTORS_PER_PAGE, count);
                        ret=_writemultisector(vol->devnode, vol->partnode, buffer, page_offset*SXA_SECTORS_PER_PAGE, wrsize);
                        if(ret) return retsize;
                    }
                    else
#endif
                    {
                        page = _alloc_cache(device, page_offset);
                        ret=_readmultipage(vol->devnode, vol->partnode, page->data, page_offset, 1);
                        if (ret) return retsize;

                            wrsize = (unsigned int)count;
                            PalMemcpy(page->data, buffer, wrsize);
#ifdef SXA_WRITE_THROUGH
                            ret = _writeback_page(page);
                            if(ret) return retsize;

#else
                            _add_dirty(page);
#endif
                    }
                }
                else
                {
                    //printf("[G2]\r\n");

                    wrsize = (unsigned int)count;
                    PalMemcpy(page->data, buffer, wrsize);
#ifdef SXA_WRITE_THROUGH
                    ret = _writeback_page(page);
                    if(ret) return retsize;
#else
                    _add_dirty(page);
#endif
                }
                SXA_ACC_TIME0(t_volumewrite_tail);
                SXA_ACC_2(n_volumewrite_N, 1);
            }
        }
        buffer += wrsize;
        count -= wrsize;
        pos += wrsize;
        retsize += wrsize;
    }
    vol->offset = pos;

    return retsize;
}

/************************************************************
 * public function
 ************************************************************/

/************************************************************
 * sxa_blkdev_get_info
 * get device and partition information
 * RETURN
 * On success, 0 is returned.
 * On error, none zero is returned.
 ************************************************************/
int
sxa_blkdev_get_info(
    unsigned int                        devnode,
    SXA_BLKDEV_INFO*                    info)
{
    int i, j, ret = 0;
//printf("sxa_blkdev_get_info 1\r\n");
    devnode = SXA_NTFS_GETDEVICE(devnode) - PAL_DEVICE_TYPE_USB0;
    // check if device and partition valid or not
    if(devnode<0 || devnode>SXA_MAX_DEVICE-1)
        return -1;
//printf("sxa_blkdev_get_info 2 %d %d\r\n", devnode, device_array[devnode].num_partitions);
    if (device_array[devnode].num_partitions == 0)
    {
//printf("sxa_blkdev_get_info 3\r\n");
        // initial usb
         ret = usb_initfunc(devnode);
         if (ret) return ret;

         // initial device struct
         PalMemset(&device_array[devnode], 0, sizeof(SXA_BLKDEV_INFO));

         // read partition
         ret = _getpartition(devnode);
    }
    PalMemcpy(info, &device_array[devnode], sizeof(SXA_BLKDEV_INFO));

    // keep record only on NTFS partition is exist
    j = 0;
    for (i=0;i<device_array[devnode].num_partitions;i++)
    {
        if (device_array[devnode].partitions[i].type == SXA_BLKDEV_PARTITION_TYPE_NTFS)
        {
            j++;
            //printf("j=%d\r\n", j);
        }
    }
    if (j == 0)
        device_array[devnode].num_partitions = 0;

    // TEST
    #if (0)
    if (info->num_partitions > 2)
    {
        info->num_partitions = 2;

        memset(&info->partitions[2], 0, sizeof(*info) - ((char *)&info->partitions[2] - (char*)info));
    }
    #endif
    //info->partitions[2].type = 0;
    // TEST
//printf("sxa_blkdev_get_info 4 %d\r\n", ret);
    return ret;
}

/************************************************************
 * sxa_blkdev_open
 * open a device with partition
 * and return a volume id (fd)
 * HIWORD(dev) is PAL_DEVICE_TYPE, LOWORD(dev) is the partition number
 * partition number is start from 1, note: internal is start from 0
 * RETURN
 * On success, none zero blkdev handler is returned.
 * On error, 0 is returned.
 ************************************************************/
int
sxa_blkdev_open(
    unsigned int                        devnode,
    unsigned int                        flags)
{
    int device = HIWORD(devnode)-PAL_DEVICE_TYPE_USB0;
    int subdev = LOWORD(devnode)-1;
    SXA_VOLUME *vol;
//printf("sxa_blkdev_open 1\r\n");

    // check if device and partition valid or not
    if(device<0 || device>SXA_MAX_DEVICE-1)
        return 0;

    // check device initialized or not
    if(subdev<0 || subdev>= device_array[device].num_partitions)
        return 0;

    // initial cache if not ready
    if(!cachep)
        cachep = _cache_init();

    if(device_array[device].partitions[subdev].vol)  // already opened
        return 0;
//printf("sxa_blkdev_open 2\r\n");
    vol = (SXA_VOLUME*)PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(SXA_VOLUME));
    if(!vol) return 0;
    vol->devnode = device;
    vol->partnode = subdev;
    vol->flags = flags;
    vol->offset = 0;
    device_array[device].partitions[subdev].vol = vol;
    volcount++;

    return (int)vol;
}

/************************************************************
 * sxa_blkdev_close
 * close with volume id
 * RETURN
 * On success, 0 is returned.
 * On error, none zero is returned.
 ************************************************************/
int
sxa_blkdev_close(
    int                     fd)
{
    int ret = 0;
    int i,j,k;
    SXA_VOLUME *vol = (SXA_VOLUME*)fd;

    //printf("sxa_blkdev_close fd=0x%X\r\n", vol);

    for (i=0;i<SXA_MAX_DEVICE;i++)
    {
        //printf("i=%d num_partitions=%d\r\n", i, device_array[i].num_partitions);
        k = 0;
        for (j=0;j<device_array[i].num_partitions;j++)
        {
            //printf("j =%d, vol=0x%X\r\n", j, device_array[i].partitions[j].vol);
            if (device_array[i].partitions[j].vol==vol)
            {
                //printf("PalHeapFree SXA_VOLUME=0x%X\r\n", vol);
                PalHeapFree(PAL_HEAP_DEFAULT, vol);
                device_array[i].partitions[j].vol = 0;
                volcount--;
            }

            if (device_array[i].partitions[j].vol==0)
                k++;
        }
        if (device_array[i].num_partitions == k)
            device_array[i].num_partitions = 0;

        //printf("+++i=%d num_partitions=%d\r\n", i, device_array[i].num_partitions);
    }
    //printf("volcount=%d\r\n", volcount);

    if (volcount==0)
    {
        _cache_term(cachep);  // also release cache
        cachep = 0;
    }

    return ret;
}

/************************************************************
 * sxa_blkdev_lseek
 * seek function
 * whence 0: seek set
 *        1: seek cur
 *        2: seek end
 * RETURN
 * On success, the resulting byte offset from the begging of the file is returned.
 * On error, -1ULL is returned.
 ************************************************************/
unsigned long long
sxa_blkdev_lseek(
    int                     fd,
    signed long long        offset,
    int                     whence)
{
    SXA_VOLUME *vol = (SXA_VOLUME*)fd;
    SXA_PARTITION *part;

    if(!device_array[vol->devnode].num_partitions)
        return -1ULL;

    part = &device_array[vol->devnode].partitions[vol->partnode];
    if(whence==0) // SEEK_SET
    {
        if(offset<0)
            return -1ULL;
        if(offset>(signed long long)part->num_sectors*part->sector_size)
            return -1ULL;
        vol->offset = offset;
    }
    else if(whence==1) // SEEK_CUR
    {
        signed long long newoffset = vol->offset + offset;
        if(newoffset<0)
            return -1ULL;
        if(newoffset>(signed long long)part->num_sectors*part->sector_size)
            return -1ULL;
        vol->offset = newoffset;
    }
    else if(whence==2) // SEEK_END
    {
        signed long long newoffset = part->num_sectors*part->sector_size + offset;
        if(newoffset<0)
            return -1ULL;
        if(newoffset>(signed long long)part->num_sectors*part->sector_size)
            return -1ULL;
        vol->offset = newoffset;
    }
    else
        return -1ULL;

    return vol->offset;
}

/************************************************************
 * sxa_blkdev_read
 * read function
 * RETURN
 * On success, the number of bytes read is returned.
 * On error, -1 is returned.
 ************************************************************/
int
sxa_blkdev_read(
    int                     fd,
    void*                   buf,
    int                     count)
{
    int ret;
    SXA_VOLUME *vol = (SXA_VOLUME*)fd;
    SXA_PARTITION *part;

    if (count<0) return 0;
    if (count==0) return 0;

    if(!device_array[vol->devnode].num_partitions)
        return -1;

    part = &device_array[vol->devnode].partitions[vol->partnode];

    if (vol->offset+count>part->num_sectors*part->sector_size)
        count = (unsigned int)(part->num_sectors*part->sector_size - vol->offset);

    if (count==0) return 0;

    ret = _volumeread(vol, buf, count);

    return ret;
}

/************************************************************
 * sxa_blkdev_write
 * write function
 * On success, the number of bytes write is returned.
 * On error, -1 is returned.
 ************************************************************/
int
sxa_blkdev_write(
    int                     fd,
    const void*             buf,
    int                     count)
{
    int ret;
    SXA_VOLUME *vol = (SXA_VOLUME*)fd;
    SXA_PARTITION *part;

    if (count<0) return 0;
    if (count==0) return 0;

    if(!device_array[vol->devnode].num_partitions)
        return -1;

    part = &device_array[vol->devnode].partitions[vol->partnode];

    if (vol->offset+count>part->num_sectors*part->sector_size)
        count = (unsigned int)(part->num_sectors*part->sector_size - vol->offset);

    if (count==0) return 0;

    ret = _volumewrite(vol, (void*)buf, count);

    return ret;
}

/************************************************************
 * sxa_blkdev_pread
 * position read function
 * On success, the number of bytes read is returned.
 * On error, -1 is returned.
 ************************************************************/
int
sxa_blkdev_pread(
    int                     fd,
    void*                   buf,
    int                     count,
    signed long long        pos)
{
    int ret;
    SXA_VOLUME *vol = (SXA_VOLUME*)fd;
    SXA_PARTITION *part;
#ifdef LOG_TIME
    int                     t0, t1;
    t0 = PalGetClock();
#endif // LOG_TIME

    SXA_RESET_START_TIME0();
    //printf("%s %08X %d\r\n", __FUNCTION__, (long) pos, (long) count);
    SXA_ACC_TIME0(t_blkdev_print);

    if (count<0) return 0;
    if (count==0) return 0;

    if(!device_array[vol->devnode].num_partitions)
        return -1;

    part = &device_array[vol->devnode].partitions[vol->partnode];

    if (pos+count>part->num_sectors*part->sector_size)
        count = (unsigned int)(part->num_sectors*part->sector_size - pos);

    if (count==0) return 0;

    vol->offset = pos;
    ret = _volumeread(vol, buf, count);
#ifdef LOG_TIME
    t1 = PalGetDuration(t0);

    printf("pread\t%d\t%d\n", t1, count);
#endif // LOG_TIME

    return ret;
}

/************************************************************
 * sxa_blkdev_pwrite
 * position write function
 * On success, the number of bytes write is returned.
 * On error, -1 is returned.
 ************************************************************/
int
sxa_blkdev_pwrite(
    int                     fd,
    const void*             buf,
    int                     count,
    signed long long        pos)
{
    int ret;
    SXA_VOLUME *vol = (SXA_VOLUME*)fd;
    SXA_PARTITION *part;
#ifdef LOG_TIME
    int                     t0, t1;
    t0 = PalGetClock();
#endif // LOG_TIME

    SXA_RESET_START_TIME0();
    //printf("%s %08X %d\r\n", __FUNCTION__, (long) pos, (long) count);
    SXA_ACC_TIME0(t_blkdev_print);

    if (count<0) return 0;
    if (count==0) return 0;

    if(!device_array[vol->devnode].num_partitions)
        return -1;

    part = &device_array[vol->devnode].partitions[vol->partnode];

    if (pos+count>part->num_sectors*part->sector_size)
        count = (unsigned int)(part->num_sectors*part->sector_size - pos);

    if (count==0) return 0;

    vol->offset = pos;
    ret = _volumewrite(vol, (void*)buf, count);
#ifdef LOG_TIME
    t1 = PalGetDuration(t0);

    printf("pwrite\t%d\t%d\n", t1, count);
#endif // LOG_TIME

    return ret;
}

/************************************************************
 * sxa_blkdev_sync
 * flush cache
 * RETURN
 * On success, 0 is returned.
 * On error, -1 is returned.
 ************************************************************/
int
sxa_blkdev_sync(
    int                     fd)
{
    int ret = 0;
#ifndef SXA_WRITE_THROUGH
    SXA_VOLUME *vol = (SXA_VOLUME*)fd;
    SXA_PAGE *page = dirty_head;

    //printf("sxa_blkdev_sync\r\n");

    // check dirty list, if exist, writeback
    while (page)
    {
        ret=_writeback_page(page);
        page = _remove_dirty(page);
    }
#endif
    return ret;
}


int
sxa_blkdev_stat(
    int                     fd,
    struct stat*            buf)
{
    return -1;
}


int
sxa_blkdev_ioctl(
    int                     fd,
    int                     request,
    void*                   arg)
{
    return -1;
}


