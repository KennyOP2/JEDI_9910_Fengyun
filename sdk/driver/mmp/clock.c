#include "host/host.h"

//=============================================================================
// MMP_PCR_CLOCK_REG_42
void 
HOST_PCR_EnableClock(
    void)
{
    HOST_WriteRegisterMask(MMP_PCR_CLOCK_REG_42, 0xFFFF, MMP_PCR_EN_PCLK);
    HOST_WriteRegisterMask(MMP_PCR_CLOCK_REG_40, 0xFFFF, MMP_PCR_EN_DIV_PCLK);
    HOST_WriteRegisterMask(MMP_HOST_BUS_CONTROLLER_REG_202, 0xFFFF, MMP_HOST_BUS_EN_MMIO_FPC);
}
// MMP_ISP_CLOCK_REG_30, MMP_ISP_CLOCK_REG_32
void 
HOST_ISP_EnableClock(
    void)
{
    HOST_WriteRegisterMask(MMP_ISP_CLOCK_REG_32, 0xFFFF, MMP_ISP_EN_N5CLK | MMP_ISP_EN_M5CLK | MMP_ISP_EN_ICLK);
    HOST_WriteRegisterMask(MMP_ISP_CLOCK_REG_30, 0xFFFF, MMP_ISP_EN_DIV_ICLK);
    HOST_WriteRegisterMask(MMP_HOST_BUS_CONTROLLER_REG_202, 0xFFFF, MMP_HOST_BUS_EN_MMIO_ISP);
}

void 
HOST_ISP_DisableClock(
    void)
{
    HOST_WriteRegisterMask(MMP_ISP_CLOCK_REG_32, 0x0000, MMP_ISP_EN_N5CLK | MMP_ISP_EN_M5CLK | MMP_ISP_EN_ICLK);
    HOST_WriteRegisterMask(MMP_ISP_CLOCK_REG_30, 0x0000, MMP_ISP_EN_DIV_ICLK);
    HOST_WriteRegisterMask(MMP_HOST_BUS_CONTROLLER_REG_202, 0x0000, MMP_HOST_BUS_EN_MMIO_ISP);
}

void 
HOST_ISP_Reset(
    void)
{
	HOST_WriteRegisterMask(MMP_ISP_CLOCK_REG_32, 0xFFFF, MMP_ISP_RESET | MMP_ISPGBL_RESET);
	MMP_Sleep(1);
	HOST_WriteRegisterMask(MMP_ISP_CLOCK_REG_32, 0x0000, MMP_ISP_RESET | MMP_ISPGBL_RESET);
	MMP_Sleep(1);
}
