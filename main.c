#include <stdint.h>
#include <switch.h>

#include "nxusb.h"


void app_init(void)
{
    if (R_FAILED(usbCommsInitialize()))
        fatalThrow(0xE001);
}

void app_exit(void)
{
    usbCommsExit();
}

uint64_t poll_intput(void)
{
    hidScanInput();
    return hidKeysDown(CONTROLLER_P1_AUTO);
}

int main(int argc, char *argv[])
{
    app_init();

    while (appletMainLoop())
    {
        uint64_t input = poll_intput();
        // call functions here.

        if (input & KEY_B || input & KEY_PLUS)
            break;
    }

    app_exit();
    return 0;
}