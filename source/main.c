#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <switch.h>

#include "nxusb.h"


typedef enum
{
    func_usb_int,
    func_usb_exit,
    func_usb_open_file_read,
    func_usb_open_file_write,
    func_usb_touch_file,
    func_usb_rename_file,
    func_usb_delete_file,
    func_usb_read_file,
    func_usb_write_to_file,
    func_usb_get_file_size,
    func_usb_close_file,
    func_usb_open_dir,
    func_usb_delete_dir,
    func_usb_rename_dir,
    func_usb_touch_dir,
    func_usb_get_dir_size_from_path,
    func_usb_get_dir_size_recursively_from_path,
} func;

const char *func_str[] =
{
    "usb_int",
    "usb_exit",
    "usb_open_file",
    "usb_touch_file",
    "usb_rename_file",
    "usb_delete_file",
    "usb_read_file",
    "usb_write_to_file",
    "usb_get_file_size",
    "usb_close_file",
    "usb_open_dir",
    "usb_delete_dir",
    "usb_rename_dir",
    "usb_touch_dir",
    "usb_get_dir_size_from_path",
    "usb_get_dir_size_recursively_from_path",
};


uint64_t poll_intput(void)
{
    hidScanInput();
    return hidKeysDown(CONTROLLER_P1_AUTO);
}

void print_message_display(const char* message, ...)
{
    char new_message[FS_MAX_PATH];
    va_list arg;
    va_start(arg, message);
    vsprintf(new_message, message, arg);
    va_end(arg);

    printf("%s", new_message);
    consoleUpdate(NULL);
}

void print_message_clear_display(const char *message, ...)
{
    char new_message[FS_MAX_PATH];
    va_list arg;
    va_start(arg, message);
    vsprintf(new_message, message, arg);
    va_end(arg);

    consoleClear();
    printf("%s", new_message);
    consoleUpdate(NULL);
}

void print_message_loop_lock(const char* message, ...)
{
    char new_message[FS_MAX_PATH];
    va_list arg;
    va_start(arg, message);
    vsprintf(new_message, message, arg);
    va_end(arg);

    printf("%s", new_message);
    consoleUpdate(NULL);
    while (appletMainLoop())
    {
        if (poll_intput() & KEY_B)
            break;
    }
}

void check_error_code(UsbReturnCode err)
{
    consoleClear();

    if (usb_failed(err))
        print_message_loop_lock("you got the error code %u\n\n", err);
    else
        print_message_loop_lock("Success!!!\n\n");
}

void app_init(void)
{
    consoleInit(NULL);
}

void app_exit(void)
{
    consoleExit(NULL);
}

void keyboard(char *buffer)
{
    SwkbdConfig config;
    swkbdConfigMakePresetDefault(&config);
    swkbdConfigSetStringLenMax(&config, 0x100);
    swkbdCreate(&config, 0);
    swkbdShow(&config, buffer, 0x100);
    swkbdClose(&config);
}

void print_debug_menu(uint8_t cursor, uint8_t max)
{
    consoleClear();

    for (uint8_t i = 0; i < max; i++)
    {
        if (cursor == i)
            printf("> %s\n\n", func_str[i]);
        else
            printf("  %s\n\n", func_str[i]);
    }

    consoleUpdate(NULL);
}

uint32_t move_cursor_up(uint32_t cursor, uint32_t cursor_max)
{
    if (cursor == 0)
        cursor = cursor_max - 1;
    else
        cursor--;
    return cursor;
}

uint32_t move_cursor_down(uint32_t cursor, uint32_t cursor_max)
{
    if (cursor == cursor_max - 1)
        cursor = 0;
    else
        cursor++;
    return cursor;
}

int main(int argc, char *argv[])
{
    app_init();

    uint8_t cursor      = 0;
    uint8_t cursor_max  = 16;
    print_debug_menu(cursor, cursor_max);

    while (appletMainLoop())
    {
        uint64_t input = poll_intput();

        if (input & KEY_DOWN)
        {
            cursor = move_cursor_down(cursor, cursor_max);
            print_debug_menu(cursor, cursor_max);
        }
        
        if (input & KEY_UP)
        {
            cursor = move_cursor_up(cursor, cursor_max);
            print_debug_menu(cursor, cursor_max);
        }

        if (input & KEY_A)
        {
            char text[0x100] = {0};
            char text2[0x100] = {0};

            uint8_t buf[0x800000];  // 8MiB.
            uint8_t mode = 0;
            size_t size = 0;

            switch (cursor)
            {
                case func_usb_int:
                {
                    check_error_code(usb_init());
                    break;
                }
                case func_usb_exit:
                {
                    usb_exit();
                    break;
                }
                case func_usb_open_file_read:
                {
                    keyboard(text);
                    check_error_code(usb_open_file(text, UsbFileOpenMode_Read));
                    break;
                }
                case func_usb_open_file_write:
                {
                    keyboard(text);
                    check_error_code(usb_open_file(text, UsbFileOpenMode_Write));
                    break;
                }
                case func_usb_touch_file:
                {
                    keyboard(text);
                    check_error_code(usb_touch_file(text));
                    break;
                }
                case func_usb_rename_file:
                {
                    keyboard(text);
                    keyboard(text2);
                    check_error_code(usb_rename_file(text, text2));
                    break;
                }
                case func_usb_delete_file:
                {
                    keyboard(text);
                    usb_delete_file(text);
                    break;
                }
                case func_usb_read_file:
                {
                    check_error_code(usb_read_file(buf, 0x20, 0));
                    break;
                }
                case func_usb_write_to_file:
                {
                    *buf = 99;
                    check_error_code(usb_write_to_file(buf, 0x20, 0));
                    break;
                }
                case func_usb_get_file_size:
                {
                    keyboard(text);
                    check_error_code(usb_get_file_size(text, &size));
                    break;
                }
                case func_usb_close_file:
                {
                    usb_close_file();
                    break;
                }
                case func_usb_open_dir:
                {
                    keyboard(text);
                    check_error_code(usb_open_dir(text));
                    break;
                }
                case func_usb_delete_dir:
                {
                    keyboard(text);
                    check_error_code(usb_delete_dir(text));
                    break;
                }
                case func_usb_rename_dir:
                {
                    keyboard(text);
                    keyboard(text2);
                    check_error_code(usb_rename_dir(text, text2));
                    break;
                }
                case func_usb_touch_dir:
                {
                    keyboard(text);
                    check_error_code(usb_touch_dir(text));
                    break;
                }
                case func_usb_get_dir_size_from_path:
                {
                    keyboard(text);
                    check_error_code(usb_get_dir_size_from_path(text, &size));
                    break;
                }
                case func_usb_get_dir_size_recursively_from_path:
                {
                    keyboard(text);
                    check_error_code(usb_get_dir_size_recursively_from_path(text, &size));
                    break;
                }
            }
            print_debug_menu(cursor, cursor_max);
        }

        if (input & KEY_B || input & KEY_PLUS)
            break;
    }

    app_exit();
    return 0;
}