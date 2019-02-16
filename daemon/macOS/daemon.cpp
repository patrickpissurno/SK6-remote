#include <stdio.h> // Standard input/output definitions
#include <stdlib.h>
#include <string.h> // String function definitions
#include <unistd.h> // for usleep()
#include <getopt.h>
#include <ApplicationServices/ApplicationServices.h>

extern "C" {
    #include "arduino-serial-lib.h"
}

//from /System/Library/Frameworks/Carbon.framework/Versions/A/Frameworks/HIToolbox.framework/Versions/A/Headers/Events.h
const int kVK_LeftArrow = 0x7B;
const int kVK_RightArrow = 0x7C;
const int kVK_Space = 0x31;
const int kVK_Return = 0x24;
const int kVK_ANSI_N = 0x2D;
const int kVK_ANSI_P = 0x23;

const int RECEIVER_HANDSHAKE_CODE = 28719;

const int BUTTON_NEXT = 1;
const int BUTTON_PREV = 2;
const int BUTTON_PAUSE = 3;
const int BUTTON_INFO = 4;
const int BUTTON_AVSYNC = 5;

const int BUTTON_NEXT_HOLD = 10 + BUTTON_NEXT;
const int BUTTON_PREV_HOLD = 10 + BUTTON_PREV;
const int BUTTON_PAUSE_HOLD = 10 + BUTTON_PAUSE;
const int BUTTON_INFO_HOLD = 10 + BUTTON_INFO;
const int BUTTON_AVSYNC_HOLD = 10 + BUTTON_AVSYNC;

void Press(int key, bool shift)
{
    CGKeyCode inputKeyCode = key;

    // Create an HID hardware event source
    CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);

    // Create a new keyboard key press event
    CGEventRef pressEvent = CGEventCreateKeyboardEvent(source, inputKeyCode, 1);

    if (shift)
        CGEventSetFlags(pressEvent, kCGEventFlagMaskShift);

    // Create a new keyboard key release event
    CGEventRef releaseEvent = CGEventCreateKeyboardEvent(source, inputKeyCode, 0);

    // Post keyboard press event and keyboard release event
    CGEventPost(kCGAnnotatedSessionEventTap, pressEvent);
    CGEventPost(kCGAnnotatedSessionEventTap, releaseEvent);

    // Release resources
    CFRelease(releaseEvent);
    CFRelease(pressEvent);
    CFRelease(source);

    usleep(5000);
}

int findSerialPorts(char (*output)[128]){
    char buf[128];
    FILE* fp;

    if ((fp = popen("ls /dev/cu.*", "r")) == NULL) {
        printf("Error opening pipe!\n");
        return -1;
    }

    int length = 0;
    while (fscanf(fp, " %s ", buf) == 1) {
        sprintf(output[length], "%s", buf);
        length += 1;
    }

    if(pclose(fp))  {
        printf("Command not found or exited with error status\n");
        return -1;
    }

    return length;
}

int main(int argc, char* argv[])
{
    const int buf_max = 256;

    char serialport[buf_max];
    char eolchar = '\n';
    int timeout = 5000;
    char buf[buf_max];
    int rc, n;

    printf("Daemon started\n");

    //wait a bit of time for all components to initialize before trying to connect
    sleep(10);

    printf("Locating the Receiver device\n");

    char ports[128][128];
    int ports_count;
    int fd;
    int handshake;
    do {
        sleep(1);

        ports_count = findSerialPorts(ports);
        for(int i = 0; i < ports_count; i++){
            fd = serialport_init(ports[i], 9600);
            if(fd == -1)
                continue;

            memset(buf, 0, buf_max);
            serialport_read_until(fd, buf, eolchar, buf_max, 2100);

            handshake = 0;
            handshake = strtol(buf, NULL, 10);

            if(handshake == RECEIVER_HANDSHAKE_CODE)
                break;
            else {
                serialport_close(fd);
                fd = -1;
            }
        }
    }
    while(ports_count < 1);
    

    // int fd = serialport_init("/dev/cu.wchusbserial1420", 9600);
    if(fd == -1){
        sleep(10);
        exit(1);
    }

    printf("Connected to the Receiver device\n");

    int btn = 0;
    while (true)
    {
        usleep(500 * 1000);
        for(int i = 0; i < 5; i++){
            memset(buf,0,buf_max);
            serialport_read_until(fd, buf, eolchar, buf_max, timeout);

            btn = 0;
            btn = strtol(buf, NULL, 10);

            if(btn == RECEIVER_HANDSHAKE_CODE)
                continue;

            switch(btn){
                case BUTTON_NEXT:
                    Press(kVK_RightArrow, false);
                    break;
                case BUTTON_PREV:
                    Press(kVK_LeftArrow, false);
                    break;
                case BUTTON_PAUSE:
                    Press(kVK_Space, false);
                    break;
                case BUTTON_NEXT_HOLD:
                    Press(kVK_ANSI_N, true);
                    break;
                case BUTTON_PREV_HOLD:
                    Press(kVK_ANSI_P, true);
                    break;
            }
            usleep(5 * 1000);
        }
    }

    serialport_close(fd);

    return 0;
}