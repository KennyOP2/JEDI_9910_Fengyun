
#include "mmp_sd.h"
#include "common/fat.h"


static F_DRIVER drv;

static int drv_readsector(F_DRIVER *driver,void *data,unsigned long sector)
{
    MMP_INT ret = 0;

    ret = mmpSdReadMultipleSector(sector,1,data);
    if(ret)
    {
        return F_ERR_READ;
    }
    else
    {
        return F_NO_ERROR;
    }
}

static int drv_readmultiplesector(F_DRIVER *driver,void *data,unsigned long sector,int cnt)
{
    MMP_INT ret = 0;

    ret  = mmpSdReadMultipleSector(sector,cnt,data);
    if(ret)
    {
        return F_ERR_READ;
    }
    else
    {
        return F_NO_ERROR;
    }
}

static int drv_writesector(F_DRIVER *driver,void *data,unsigned long sector)
{
    MMP_INT ret = 0;

    ret  = mmpSdWriteMultipleSector(sector,1,data);
    if(ret)
    {
        return F_ERR_WRITE;
    }
    else
    {
        return F_NO_ERROR;
    }
}

static int drv_writemultiplesector(F_DRIVER *driver,void *data,unsigned long sector,int cnt)
{
    MMP_INT ret = 0;

    ret  = mmpSdWriteMultipleSector(sector,cnt,data);
    if(ret)
    {
        return F_ERR_WRITE;
    }
    else
    {
        return F_NO_ERROR;
    }
}

static long drv_getstatus(F_DRIVER *driver)
{
    if(mmpSdGetCardState(SD_INSERTED))
        return F_NO_ERROR;
    else
        return F_ST_MISSING;
}

static int drv_getphy(F_DRIVER *driver,F_PHY *phy)
{
    MMP_INT result = 0;
    MMP_UINT32 sectors = 0;
    MMP_UINT32 blockSize = 0;

    result = mmpSdGetCapacity(&sectors, &blockSize);
    if(result)
        goto end;
        
    phy->number_of_cylinders = 0;
    phy->sector_per_track    = 63;
    phy->number_of_heads     = 255;
    phy->number_of_sectors   = sectors;
    phy->media_descriptor    = 0xf8; /* fixed drive */

end:
    return result;
}

static void drv_release(F_DRIVER *driver)
{
   mmpSdTerminate();
}

F_DRIVER *sd_initfunc(unsigned long driver_param)
{
    MMP_INT ret = 0;

    ret  = mmpSdInitialize();
    if (ret != MMP_RESULT_SUCCESS)
    {
        return 0;
    }
    else
    {
        drv.readsector=drv_readsector;
        drv.writesector=drv_writesector;
        drv.readmultiplesector=drv_readmultiplesector;
        drv.writemultiplesector=drv_writemultiplesector;
        drv.getstatus=drv_getstatus;
        drv.getphy=drv_getphy;
        drv.release=drv_release;
        drv.user_data=0x0001;

        return (&drv);
    }
}
