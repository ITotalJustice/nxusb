/*
*   TotalJustice
*/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <switch.h>

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
        size_t sz;
    } poll = { mode, {0}, size };

    if (usb_failed(usb_write(&poll, USB_POLL_SIZE)))
        return UsbReturnCode_PollError;
    return UsbReturnCode_Success;
}

UsbRet usb_get_result(void)
{
    UsbRet ret;
    usb_read(&ret, sizeof(UsbRet));
    return ret;
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

void usb_exit(void)
{
    usb_poll(UsbMode_Exit, 0);
}


/*
*   File Functions.
*/

UsbRet usb_open_file(const char *name)
{
    if (name == NULL)
        return UsbReturnCode_EmptyField;

    size_t size = strlen(name);
    if (size >= USB_FILE_NAME_MAX)
        return UsbReturnCode_FileNameTooLarge;

    UsbRet ret;

    ret = usb_poll(UsbMode_OpenFile, size);
    if (usb_failed(ret))
        return ret;

    ret = usb_write(name, size);
    if (usb_failed(ret))
        return ret;

    return usb_get_result();
}

UsbRet usb_touch_file(const char *name)
{
    if (name == NULL)
        return UsbReturnCode_EmptyField;

    size_t size = strlen(name);
    if (size >= USB_FILE_NAME_MAX)
        return UsbReturnCode_FileNameTooLarge;

    UsbRet ret;

    ret = usb_poll(UsbMode_TouchFile, size);

    if (usb_failed(ret))
        return ret;

    ret = usb_write(name, size);
    if (usb_failed(ret))
        return ret;

    return usb_get_result();
}

UsbRet usb_rename_file(const char *curr_name, const char *new_name)
{
    if (curr_name == NULL || new_name == NULL)
        return UsbReturnCode_EmptyField;

    size_t str1_len = strlen(curr_name);
    size_t str2_len = strlen(new_name);

    if (str1_len >= USB_FILE_NAME_MAX || str2_len >= USB_FILE_NAME_MAX)
        return UsbReturnCode_FileNameTooLarge;

    UsbRet ret;

    ret = usb_poll(UsbMode_RenameFile, str1_len + str2_len + 2 + 0x10);
    if (usb_failed(ret))
        return ret;

    const struct
    {
        size_t l1;
        size_t l2;
        const char *str1;
        const char *str2;
    } send = { str1_len, str2_len, curr_name, new_name };

    ret = usb_write(&send, str1_len + str2_len + 2 + 0x10);
    if (usb_failed(ret))
        return ret;
    
    return usb_get_result();
}

UsbRet usb_delete_file(const char *name)
{
    if (name == NULL)
        return UsbReturnCode_EmptyField;

    UsbRet ret;
    size_t size = strlen(name);

    ret = usb_poll(UsbMode_DeleteFile, size);
    if (usb_failed(ret))
        return ret;

    ret = usb_write(name, size);
    if (usb_failed(ret))
        return ret;

    return usb_get_result();
}

UsbRet usb_read_file(void *out, size_t size, uint64_t offset)
{
    if (out == NULL)
        return UsbReturnCode_EmptyField;

    UsbRet ret;

    ret = usb_poll(UsbMode_ReadFile, size);
    if (usb_failed(ret))
        return ret;

    const struct
    {
        size_t sz;
        uint64_t off;
    } send = {size, offset};

    ret = usb_write(&send, 0x10);
    if (usb_failed(ret))
        return ret;

    return usb_read(out, size);
}

UsbRet usb_write_to_file(const void *in, size_t size, uint64_t offset)
{
    if (in == NULL)
        return UsbReturnCode_EmptyField;

    UsbRet ret;

    ret = usb_poll(UsbMode_WriteFile, size);
    if (usb_failed(ret))
        return ret;

    const struct
    {
        size_t sz;
        uint64_t off;
    } send = { size, offset};

    ret = usb_write(&send, 0x10);
    if (usb_failed(ret))
        return ret;
    
    return usb_write(in, size);
}

void usb_close_file(void)
{
    usb_poll(UsbMode_CloseFile, 0);
}