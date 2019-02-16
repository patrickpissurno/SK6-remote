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

int main(int argc, char *argv[])
{
    const int buf_max = 256;

    char serialport[buf_max];
    char eolchar = '\n';
    int timeout = 5000;
    char buf[buf_max];
    int rc, n;

    int fd = serialport_init("/dev/cu.wchusbserial1420", 9600);

    int btn = 0;
    while (true)
    {
        usleep(500 * 1000);
        for(int i = 0; i < 5; i++){
            memset(buf,0,buf_max);
            serialport_read_until(fd, buf, eolchar, buf_max, timeout);

            btn = 0;
            btn = strtol(buf, NULL, 10);

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