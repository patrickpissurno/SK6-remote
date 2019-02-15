#include <stdio.h>    // Standard input/output definitions
#include <stdlib.h>
#include <string.h>   // String function definitions
#include <unistd.h>   // for usleep()
#include <getopt.h>
#include <ApplicationServices/ApplicationServices.h>

#include "arduino-serial-lib.h"

//from /System/Library/Frameworks/Carbon.framework/Versions/A/Frameworks/HIToolbox.framework/Versions/A/Headers/Events.h
const int kVK_LeftArrow = 0x7B;
const int kVK_RightArrow = 0x7C;
const int kVK_Space = 0x31;
const int kVK_Return = 0x24;
const int kVK_ANSI_N = 0x2D;
const int kVK_ANSI_P = 0x23;

void Press(int key, bool shift) {
    CGKeyCode inputKeyCode = key;

    // Create an HID hardware event source
    CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);

    // Create a new keyboard key press event
    CGEventRef pressEvent = CGEventCreateKeyboardEvent(source, inputKeyCode, 1);

    if(shift)
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
    sleep(2);
    Press(kVK_LeftArrow, false);
    Press(kVK_ANSI_N, true);
    printf("Hello world!\n");
    return 0;
}