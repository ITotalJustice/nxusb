/*
*   TotalJustice
*/

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <switch.h>

#include "nxusb.h"


/*
*   Core Functions.
*/

typedef struct
{
    uint64_t magic;
    uint8_t macro;
    uint8_t minor;
    uint8_t major;
    uint8_t padding[0x5];
} nxusb_header;
nxusb_header g_host;  // will store the client info.
nxusb_header g_client;  // will store the client info.


UsbRet usb_init(void)
{
    if (R_FAILED(usbCommsInitialize()))
        return UsbReturnCode_FailedToInitComms;
    
    g_host.magic = NXUSB_MAGIC;
    g_host.major = NXUSB_VERSION_MAJOR;
    g_host.minor = NXUSB_VERSION_MINOR;
    g_host.macro = NXUSB_VERSION_MACRO;

    UsbRet ret;

    ret = usb_write(&g_host, 0x10);
    if (usb_failed(ret))
        return ret;

    ret = usb_get_result();
    if (usb_failed(ret))
        return ret;

    ret = usb_read(&g_client, 0x10);
    if (usb_failed(ret))
        return ret;
    
    if (g_client.magic != NXUSB_MAGIC)
        return UsbReturnCode_WrongClientMagic;

    return UsbReturnCode_Success;
}

UsbRet usb_read(void *out, size_t size)
{
    void *buf = memalign(0x1000, size);
    size_t ret = usbCommsRead(buf, size);
    memcpy(out, buf, size);
    free(buf);
    
    if (ret != size)
        return UsbReturnCode_WrongSizeRead;
    return UsbReturnCode_Success;
}

UsbRet usb_write(const void *in, size_t size)
{
    void *buf = memalign(0x1000, size);
    memcpy(buf, in, size);
    size_t ret = usbCommsWrite(buf, size);
    free(buf);

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

void usb_get_client_version(uint8_t *macro, uint8_t *minor, uint8_t *major)
{
    *macro = g_client.macro;
    *minor = g_client.minor;
    *major = g_client.major;
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
    usbCommsExit();
}



/*
*   File Functions.
*/

UsbRet usb_open_file(const char *name, uint8_t mode)
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

    ret = usb_poll(UsbMode_RenameFile, str1_len + str2_len + 0x10);
    if (usb_failed(ret))
        return ret;

    const struct
    {
        size_t l1;
        size_t l2;
        const char *str1;
        const char *str2;
    } send = { str1_len, str2_len, curr_name, new_name };

    ret = usb_write(&send, str1_len + str2_len + 0x10);
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

UsbRet usb_get_file_size(const char *name, uint64_t *out)
{
    if (name == NULL)
        return UsbReturnCode_EmptyField;

    size_t size = strlen(name);
    if (size >= USB_FILE_NAME_MAX)
        return UsbReturnCode_FileNameTooLarge;

    UsbRet ret;

    ret = usb_poll(UsbMode_GetFileSize, size);
    if (usb_failed(ret))
        return ret;

    ret = usb_write(name, size);
    if (usb_failed(ret))
        return ret;

    return usb_read(out, sizeof(uint64_t));
}

void usb_close_file(void)
{
    usb_poll(UsbMode_CloseFile, 0);
}



/*
*   Dir Functions.
*/

UsbRet usb_open_dir(const char *path)
{
    if (path == NULL)
        return UsbReturnCode_EmptyField;

    size_t size = strlen(path);
    if (size >= USB_FILE_NAME_MAX)
        return UsbReturnCode_FileNameTooLarge;

    UsbRet ret;

    ret = usb_poll(UsbMode_OpenDir, size);
    if (usb_failed(ret))
        return ret;

    ret = usb_write(path, size);
    if (usb_failed(ret))
        return ret;

    return usb_get_result();
}

UsbRet usb_delete_dir(const char *path)
{
    if (path == NULL)
        return UsbReturnCode_EmptyField;

    size_t size = strlen(path);
    if (size >= USB_FILE_NAME_MAX)
        return UsbReturnCode_FileNameTooLarge;

    UsbRet ret;

    ret = usb_poll(UsbMode_DeleteDir, size);
    if (usb_failed(ret))
        return ret;

    ret = usb_write(path, size);
    if (usb_failed(ret))
        return ret;

    return usb_get_result();
}

UsbRet usb_rename_dir(const char *curr_name, const char *new_name)
{
    if (curr_name == NULL || new_name == NULL)
        return UsbReturnCode_EmptyField;

    size_t str1_len = strlen(curr_name);
    size_t str2_len = strlen(new_name);

    if (str1_len >= USB_FILE_NAME_MAX || str2_len >= USB_FILE_NAME_MAX)
        return UsbReturnCode_FileNameTooLarge;

    UsbRet ret;

    ret = usb_poll(UsbMode_RenameFile, str1_len + str2_len + 0x10);
    if (usb_failed(ret))
        return ret;

    const struct
    {
        size_t l1;
        size_t l2;
        const char *str1;
        const char *str2;
    } send = { str1_len, str2_len, curr_name, new_name };

    ret = usb_write(&send, str1_len + str2_len + 0x10);
    if (usb_failed(ret))
        return ret;
    
    return usb_get_result();
}

UsbRet usb_touch_dir(const char *path)
{
    if (path == NULL)
        return UsbReturnCode_EmptyField;

    size_t size = strlen(path);
    if (size >= USB_FILE_NAME_MAX)
        return UsbReturnCode_FileNameTooLarge;

    UsbRet ret;

    ret = usb_poll(UsbMode_TouchFile, size);

    if (usb_failed(ret))
        return ret;

    ret = usb_write(path, size);
    if (usb_failed(ret))
        return ret;

    return usb_get_result();
}

UsbRet usb_get_dir_total_from_path(const char *path, uint64_t *out)
{

}

UsbRet usb_get_dir_total(uint64_t *out)
{

}

UsbRet usb_read_dir(usb_file_entry_t *out, size_t size)
{

}

UsbRet usb_get_dir_size(size_t *out)
{
    
}

UsbRet usb_get_dir_size_recursively(size_t *out)
{

}

UsbRet usb_get_dir_size_from_path(const char *path, size_t *out)
{
    if (path == NULL)
        return UsbReturnCode_EmptyField;

    size_t size = strlen(path);
    if (size >= USB_FILE_NAME_MAX)
        return UsbReturnCode_FileNameTooLarge;

    UsbRet ret;

    ret = usb_poll(UsbMode_GetDirSizeFromPath, size);
    if (usb_failed(ret))
        return ret;

    ret = usb_write(path, size);
    if (usb_failed(ret))
        return ret;

    return usb_read(out, sizeof(uint64_t));
}

UsbRet usb_get_dir_size_recursively_from_path(const char *path, size_t *out)
{
    if (path == NULL)
        return UsbReturnCode_EmptyField;

    size_t size = strlen(path);
    if (size >= USB_FILE_NAME_MAX)
        return UsbReturnCode_FileNameTooLarge;

    UsbRet ret;

    ret = usb_poll(UsbMode_GetDirSizeFromPathRecursively, size);
    if (usb_failed(ret))
        return ret;

    ret = usb_write(path, size);
    if (usb_failed(ret))
        return ret;

    return usb_read(out, sizeof(uint64_t));
}



/*
*   Device Functions.
*/

UsbRet usb_open_device(const char *name)
{

}

UsbRet usb_get_device_total(uint64_t *out)
{

}

UsbRet usb_read_device()
{

}