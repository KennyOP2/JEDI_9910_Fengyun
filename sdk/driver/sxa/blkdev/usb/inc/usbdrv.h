#ifndef USBDRV_H
#define USBDRV_H

int usb_initfunc(int devnode);
int usb_release(int devnode);
int usb_getphy(int devnode, unsigned int *psector_size, unsigned long long *pnum_sectors);
int usb_readmultiplesector(int devnode, void *data, unsigned long long sector, int cnt);
int usb_writemultiplesector(int devnode, void *data, unsigned long long sector, int cnt);

#endif //USBDRV_H
