#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <switch.h>

#include "nxusb.h"


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
    if (usb_failed(err))
        print_message_loop_lock("you got the error code %u\n\n", err);
    else
        print_message_loop_lock("Success!!!\n\n");
}

void app_init(void)
{
    consoleInit(NULL);
    check_error_code(usb_init());
}

void app_exit(void)
{
    consoleExit(NULL);
    usb_exit();
}

int main(int argc, char *argv[])
{
    app_init();

    /*
    while (appletMainLoop())
    {
        uint64_t input = poll_intput();
        // call functions here.

        if (input & KEY_B || input & KEY_PLUS)
            break;
    }
    */

    app_exit();
    return 0;
}