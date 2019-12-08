/*
*   TotalJustice
*/

#include <stdint.h>
#include <stdbool.h>
#include <switch/services/usb.h>

#include "nxusb.h"


/*
*   Core Functions.
*/

UsbRet usb_read(void *out, size_t size)
{
    size_t ret = usbCommsRead(out, size);
    if (ret != size)
        return UsbReturnCode_WrongSizeRead;
    return UsbReturnCode_Success;
}

UsbRet usb_write(const void *in, size_t size)
{
    size_t ret = usbCommsWrite(in, size);
    if (ret != size)
        return UsbReturnCode_WrongSizeWritten;
    return UsbReturnCode_Success;
}

UsbRet usb_poll(uint8_t mode, size_t size)
{
    struct
    {
        uint8_t m;
        uint8_t p[0x7];
        size_t size;
    } poll = { mode, size, {0} };

    if (usb_failed(usb_write(&poll, USB_POLL_SIZE)))
        return UsbReturnCode_PollError;
    return UsbReturnCode_Success;
}

void usb_exit(void)
{
    usb_poll(UsbMode_Exit, 0);
}

UsbRet usb_get_result(void)
{
    UsbRet ret;
    usb_read(&ret, sizeof(UsbRet));
}

bool usb_failed(UsbRet ret)
{
    if (ret == UsbReturnCode_Success)
        return false;
    return true;
}

bool usb_succeeded(UsbRet ret)
{
    if (ret != UsbReturnCode_Success)
        return true;
    return false;
}


/*
*   File Functions.
*/

UsbRet usb_open_file(const char *name)
{
    size_t size = strlen(name);
    if (size >= USB_FILE_NAME_MAX)
        return UsbReturnCode_FileNameTooLarge;

    if (usb_failed(usb_poll(UsbMode_OpenFile, size)))
        return UsbReturnCode_PollError;
    if (usb_failed(usb_write(name, size)))
        return UsbReturnCode_WrongSizeWritten;
    return usb_get_result();
}

UsbRet usb_touch_file(const char *name)
{
    size_t size = strlen(name);
    if (size >= USB_FILE_NAME_MAX)
        return UsbReturnCode_FileNameTooLarge;
    if (usb_failed(usb_poll(UsbMode_TouchFile, size)))
        return UsbReturnCode_PollError;
    if (usb_failed(usb_write(name, size)))
        return UsbReturnCode_WrongSizeWritten;
    return usb_get_result();
}

UsbRet usb_delete_file(const char *name)
{
    size_t size = strlen(name);
    if (usb_failed(usb_poll(UsbMode_DeleteFile, size)))
        return UsbReturnCode_PollError;
    if (usb_failed(usb_write(name, size)))
        return UsbReturnCode_WrongSizeWritten;
    return usb_get_result();
}

UsbRet usb_read_file(void *out, size_t size, uint64_t offset)
{
    if (usb_failed(usb_poll(UsbMode_ReadFile, size + 0x10)))
        return UsbReturnCode_PollError;

    struct
    {
        void *d;
        size_t s;
        uint64_t off;
    } in = { out, size, offset};

    return usb_write(&in, size + 0x10);
}

UsbRet usb_write_to_file(const void *in, size_t size, uint64_t offset)
{
    if (usb_failed(usb_poll(UsbMode_WriteFile, size + 0x10)))
        return UsbReturnCode_PollError;

    struct
    {
        void *d;
        size_t s;
        uint64_t off;
    } s = { in, size, offset};

    return usb_write(&s, size + 0x10);
}

void usb_close_file(void)
{
    usb_poll(UsbMode_CloseFile, 0);
}