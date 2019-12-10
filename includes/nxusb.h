#ifndef _NXUSB_H_
#define _NXUSB_H_

#include <stdint.h>


#define USB_POLL_SIZE       0x10
#define USB_FILE_NAME_MAX   0x200

typedef uint32_t UsbRet;    // return type


typedef enum
{
    UsbMode_Exit                    = 0x0,
    UsbMode_Ping                    = 0x1,

    UsbMode_OpenFile                = 0x10,
    UsbMode_ReadFile                = 0x11,
    UsbMode_WriteFile               = 0x12,
    UsbMode_TouchFile               = 0x13,
    UsbMode_DeleteFile              = 0x14,
    UsbMode_RenameFile              = 0x15,
    UsbMode_GetFileSize             = 0x16,
    UsbMode_CloseFile               = 0x20,

    UsbMode_OpenDir                 = 0x30,
    UsbMode_ReadDir                 = 0x31,
    UsbMode_DeleteDir               = 0x32,
    UsbMode_DeleteDirRecursively    = 0x33,
    UsbMode_GetDirTotal             = 0x34,
    UsbMode_GetDirTotalRecursively  = 0x35,
    UsbMode_RenameDir               = 0x36,
    UsbMode_TouchDir                = 0x37,

    UsbMode_OpenDevice              = 0x40,
    UsbMode_ReadDevices             = 0x41,
    UsbMode_GetTotalDevices         = 0x42,
} UsbMode;

typedef enum
{
    UsbReturnCode_Success               = 0x0,

    UsbReturnCode_PollError             = 0x1,
    UsbReturnCode_WrongSizeRead         = 0x2,
    UsbReturnCode_WrongSizeWritten      = 0x3,

    UsbReturnCode_FileNameTooLarge      = 0x10,
    UsbReturnCode_EmptyField            = 0x11,

    UsbReturnCode_FailedOpenFile        = 0x20,
    UsbReturnCode_FailedRenameFile      = 0x21,
    UsbReturnCode_FailedDeleteFile      = 0x22,

    UsbReturnCode_FailedOpenDir         = 0x30,
    UsbReturnCode_FailedRenameDir       = 0x31,
    UsbReturnCode_FailedDeleteDir       = 0x32,
} UsbReturnCode;

typedef enum
{
    UsbFileEntryType_Dir,
    UsbFileEntryType_File
} UsbFileEntryType;

typedef enum
{
    USBFileExtentionType_None   = 0x0,
    USBFileExtentionType_Ignore = 0x1,

    USBFileExtentionType_Txt    = 0x10,
    USBFileExtentionType_Ini    = 0x11,
    USBFileExtentionType_Html   = 0x12,

    USBFileExtentionType_Zip    = 0x20,
    USBFileExtentionType_7zip   = 0x21,
    USBFileExtentionType_Rar    = 0x22,

    USBFileExtentionType_Mp3    = 0x30,
    USBFileExtentionType_Mp4    = 0x31,
    USBFileExtentionType_Mkv    = 0x32,

    USBFileExtentionType_Nro    = 0x40,
    USBFileExtentionType_Nso    = 0x41,
    
    USBFileExtentionType_Nca    = 0x50,
    USBFileExtentionType_Nsp    = 0x51,
    USBFileExtentionType_Xci    = 0x52,
    USBFileExtentionType_Ncz    = 0x53,
    USBFileExtentionType_Nsz    = 0x54,
    USBFileExtentionType_Xcz    = 0x55,
} USBFileExtentionType;

typedef enum
{
    UsbFileCatagory_Dir,
    UsbFileCatagory_Compressed,
    UsbFileCatagory_Music,
    UsbFileCatagory_Movie,
    UsbFileCatagory_Homebrew,
    UsbFileCatagory_Game,
    UsbFileCatagory_Other
} UsbFileCatagory;

typedef enum
{
    UsbFileSizeType_Dir,
    UsbFileSizeType_Small,
    UsbFileSizeType_Large   // too large for fat32.
} UsbFileSizeType;

typedef enum
{
    UsbFileOpenMode_Read,
    UsbFileOpenMode_ReadBytes,
    UsbFileOpenMode_Write,
    UsbFileOpenMode_WriteBytes,
    UsbFileOpenMode_Append,
    UsbFileOpenMode_AppendBytes
} UsbFileOpenMode; // these will probably be changed because i am not sure on the openfile modes in python.

typedef struct
{
    char name[USB_FILE_NAME_MAX];   // 1kb.
    uint8_t entry_type;             // see UsbFileEntryType.
    uint8_t ext_type;               // USBFileExtentionType.
    uint8_t catagory;               // UsbFileCatagory.
    uint8_t size_type;              // see UsbFileSizeType.
    size_t file_size;               // 0 if dir. (maybe allow for recursive scan for one level deep?).
} usb_file_entry_t;



/*
*   Core Functions.
*/

// read into void *out return size of data read.
// returns UsbReturnCode_Success if the size read is equal to the given size.
UsbRet usb_read(void *out, size_t size);

// write from void *in, return the size of the data written.
// returns UsbReturnCode_Success if the size written is equal to the given size.
UsbRet usb_write(const void *in, size_t size);

// this gets called for every command.
// it will write 0x10 bytes to usb.
// this will tell the usb the mode to be in and how much data to receive / send (depending on mode).
// size can be set to zero for modes that don't require a size, such as exit and closing a file.
// the usb client should handle all of this.
UsbRet usb_poll(uint8_t mode, size_t size);

// this function will be called by other usb functions without decent error handling.
// an example would be on the function usb_open_file, poll and write could succeed, but the actual opening of the file in python might fail.
// this function gets called to read 4 bytes from the python client, which should be 0 (UsbReturnCode_Success) if no errors.
UsbRet usb_get_result(void);

// checks the ret value.
// if the value is not UsbReturnCode_Success, return true.
bool usb_failed(UsbRet ret);

// checks the ret value.
// if the value is UsbReturnCode_Success, return true.
bool usb_succeeded(UsbRet ret);

// call this to exit usb comms.
void usb_exit(void);



/*
*   File Functions.
*/

// sends the name of the file to open, and the mode (read, write).
// 
UsbRet usb_open_file(const char *name, uint8_t mode);

// create a file from the given name.
// calls usb_get_result after.
// should return UsbReturnCode_Success if the file was created OR is the file already exists.
UsbRet usb_touch_file(const char *name);

// 
UsbRet usb_rename_file(const char *curr_name, const char *new_name);

// deletes a file using the given name.
// should return UsbReturnCode_Success if the file was delete OR if the file didn't already exist.
UsbRet usb_delete_file(const char *name);

//
UsbRet usb_read_file(void *out, size_t size, uint64_t offset);

//
UsbRet usb_write_to_file(const void *in, size_t size, uint64_t offset);

// get the file 
UsbRet usb_get_file_size(const char *name, uint64_t *out);

// close the current open file in the python client.
// this should be used as a signal to the end reading writing loop.
void usb_close_file(void);



/*
*   Dir Functions.
*/

#endif