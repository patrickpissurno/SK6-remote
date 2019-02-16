#include <IRremote.h>
#include "TimerOne.h"

// Define sensor pin
const int RECV_PIN = 2;

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

const int BUTTON_REPEAT = 6;

const int BUTTON_HOLD_COUNT_REQUIRED = 8;

int last_pressed_button = -1;
int repeat_button_count = 0;

// Define IR Receiver and Results Objects
IRrecv irrecv(RECV_PIN);
decode_results results;

void sendHandshake();
void handleCode();

void setup()
{
    // Serial Monitor @ 9600 baud
    Serial.begin(9600);

    // Enable the IR Receiver
    irrecv.enableIRIn();
    
    // Send handshake code
    sendHandshake();

    // Enable interrupt
    Timer1.initialize(1000000); //1 second period
    Timer1.attachInterrupt(sendHandshake);
}

void loop()
{
    if (irrecv.decode(&results))
    {
        handleCode();
        irrecv.resume();
    }
}

void sendHandshake(){
    Serial.println(RECEIVER_HANDSHAKE_CODE);
}

void handleCode(){
    int button;
    switch(results.value){
        case 0x3434E01F:
        case 0x569D0D0B:
            button = BUTTON_NEXT;
            break;
        case 0x3434609F:
        case 0x97E03D8D:
            button = BUTTON_PREV;
            break;
        case 0xF059B22B:
        case 0x3434F20D:
            button = BUTTON_PAUSE;
            break;
        case 0x3434C53A:
        case 0xA6ACB32A:
            button = BUTTON_INFO;
            break;
        case 0x34349B64:
        case 0x8386D2C6:
            button = BUTTON_AVSYNC;
            break;
        case 0x4AB0F7B6:
            button = BUTTON_REPEAT;
            break;
        default:
            return;
    }

    if(button != BUTTON_REPEAT){
        last_pressed_button = button;
        repeat_button_count = button == -1 ? 0 : 1;
    }
    else if(repeat_button_count < BUTTON_HOLD_COUNT_REQUIRED){
        repeat_button_count += 1;
        return;
    }
    else if(repeat_button_count > BUTTON_HOLD_COUNT_REQUIRED)
        return;
    else {
        button = last_pressed_button + 10;
        repeat_button_count += 1;
    }

    Serial.println(button);
}
