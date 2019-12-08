/*
*   TotalJustice
*/

#include <stdint.h>
#include <string.h>
#include <switch/services/usb.h>

#include "nxusb.h"


/*
*   Core Functions.
*/

size_t usb_read(void *out, size_t size)
{
    return usbCommsRead(out, size);
}

size_t usb_write(const void *in, size_t size)
{
    return usbCommsWrite(in, size);
}

size_t usb_poll(uint8_t mode, size_t size)
{
    usb_poll_t poll = { mode, size, {0} };
    int ret = usb_write(&poll, 0x10);
    if (ret != 0x10)
        return 0;
    return ret;
}

void usb_exit(void)
{
    usb_poll(UsbMode_Exit, 0);
}


/*
*
*/

size_t usb_open_file(const char *name)
{
    size_t size = strlen(name);
    if (size >= USB_FILE_NAME_MAX)
        return 0;
    if (!usb_poll(UsbMode_OpenFile, size))
        return 0;
    return usb_write(name, size);
}

size_t usb_read_file(void *out, size_t size, uint64_t offset)
{
    if (!usb_poll(UsbMode_ReadFile, size + 0x10))
        return 0;

    struct
    {
        void *d;
        size_t s;
        uint64_t off;
    } in = { out, size, offset};
    return usb_write(&in, size + 0x10);
}

void usb_write_to_file(const void *in, size_t size, uint64_t offset)
{
    if (!usb_poll(UsbMode_WriteFile, size + 0x10))
        return 0;

    struct
    {
        void *d;
        size_t s;
        uint64_t off;
    } s = { in, size, offset};
    usb_write(&s, size + 0x10);
}

void usb_close_file(void)
{
    usb_poll(UsbMode_CloseFile, 0);
}