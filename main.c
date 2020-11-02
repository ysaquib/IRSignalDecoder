#include <stdint.h>
#include <stdbool.h>
#include <tm4c123gh6pm.h>
#include <sysctl.h>
#include "Serial.h"
#include "timer.h"

/**
 * main.c
 */



void PLLInit()
{
    SYSCTL_RCC2_R |= 0x80000000;
    SYSCTL_RCC2_R |= 0x00000800;
    SYSCTL_RCC_R = (SYSCTL_RCC_R & ~0x000007C0) + 0x00000540;
    SYSCTL_RCC2_R &= ~0x00000070;
    SYSCTL_RCC2_R &= ~0x00002000;
    SYSCTL_RCC2_R |= 0x40000000;
    SYSCTL_RCC2_R = (SYSCTL_RCC2_R & ~0x1FC00000) + (4 << 22);
    while ((SYSCTL_RIS_R &0x00000040)==0){};
    SYSCTL_RCC2_R &= ~0x00000800;
}

void PWMeasure_Init(void)
{
//    SYSCTL_RCGCGPIO_R |= 0x3F; // enable clock for PORT B
//    SYSCTL_RCGCGPIO_R |= 0x0A;

    SYSCTL_RCGCTIMER_R |= 0x01;     // Activate Timer 0
    SYSCTL_RCGCGPIO_R |= 0x12; //Activate Port E and B

    GPIO_PORTE_LOCK_R = 0x4C4F434B; // this value unlocks the GPIOCR register.
    GPIO_PORTE_CR_R = 0xFF;
    GPIO_PORTE_AMSEL_R = 0x00; // disable analog functionality
    GPIO_PORTE_PCTL_R = 0x00000000; // Select GPIO mode in PCTL
    GPIO_PORTE_DIR_R = 0xFF; // Pins E0-E3 are outputs
    GPIO_PORTE_AFSEL_R = 0x00; // Disable alternate functionality
    GPIO_PORTE_DEN_R |= 0xFF; //Enable digital ports


    GPIO_PORTB_DIR_R &= ~0x04; // Make PB6 an input
    GPIO_PORTB_AFSEL_R  |= 0x40; // Alternate Functionality Select for PB6
    GPIO_PORTB_DEN_R |= 0x40; // Enable Digital Functionality
    GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & 0xF0FFFFFF) + 0x07000000;
    TIMER0_CTL_R &= ~0x00000001; // Disable timer 0 for configuration
    TIMER0_CFG_R = 0x00000004; // Configure for 16-bit capture mode
    TIMER0_TAMR_R = 0x00000007;
    TIMER0_CTL_R |= 0x0000000C; // configure for rising edge
    TIMER0_TAILR_R = 0x0000FFFF; // Start value for count down
    TIMER0_TAPR_R = 0xFF; // Activate pre-scale
    TIMER0_IMR_R |= 0x00000004; // Enable Input capture interrupts
    TIMER0_CTL_R |= 0x00000001; // Timer 0A 24-bit, rising edge
    NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF) | 0x40000000;
    NVIC_EN0_R = 1<<19;


}


bool done = false;
uint32_t first = 0;
uint32_t pulseWidth = 0;
uint32_t pulseArray[120];
int ind = 0;

void Timer0A_Handler(void)
{
    TIMER0_ICR_R = 0x00000004; // acknowledge the interrupt
    // calculate the period
    pulseWidth = (first - TIMER0_TAR_R) & 0x00FFFFFF;
    // remember the timer for next time
    first = TIMER0_TAR_R;
    // set flag to say that period has a new value
    pulseArray[ind] = pulseWidth;
    ind++;
    if (ind == 120)
        ind = 0;
    done = 1;

}

struct State
{
    uint32_t clocktick[4];
    bool returns;
    int output[2];
    struct State *NextState[3];
    struct State* outPutDone;

};
typedef struct State stateType;

#define S0 &FSM[0]
#define S1 &FSM[1]
#define S2 &FSM[2]
#define S3 &FSM[3]
#define S4 &FSM[4]
#define S5 &FSM[5]
#define S6 &FSM[6]
#define S7 &FSM[7]

const stateType FSM[8] =
{
//    {{648000,792000,648000,792000}, false, {0,0}, {S1,S1}, S1}, //S0
//    {{324000,396000,162000,198000}, false, {0,0}, {S2,S5}, S2}, //S1
//    {{40320,49280,40320,49280}, false, {0,0}, {S3,S3}, S3}, //S2
//    {{40320,49280,121680,148720}, true, {0,1}, {S2,S2}, S4}, //S3
//    {{40320,49280,40320,49280}, false, {0,0}, {S0,S0}, S0}, //S4
//    {{40320,49280,40320,49280}, false, {0,0}, {S6,S6}, S6}, //S5
//    {{7069680,8640720,7069680,8640720}, false, {0,0}, {S0,S0}, S0} //S6

     {{576000,864000,576000,864000}, false, {0,0}, {S1,S1}, S1}, //S0
     {{288000,396000,144000,216000}, false, {0,0}, {S2,S5}, S2}, //S1
     {{35840,53760,35840,53760}, false, {0,0}, {S3,S3}, S3}, //S2
     {{35840,53760,108160,162240}, true, {0,1}, {S2,S2}, S4}, //S3
     {{35840,53760,35840,53760}, false, {0,0}, {S7,S7}, S7}, //S4
     {{35840,53760,35840,53760}, false, {0,0}, {S6,S6}, S6}, //S5
     {{6284160,9426240,6284160,9426240}, true, {0,0}, {S0,S0}, S0}, //S6
     {{4275200,7001600,4275200,7001600}, false, {0,0}, {S0,S0}, S0} //S7
};

#define BUTTON_POWER        0x00FF629D
#define BUTTON_A            0x00FF22DD
#define BUTTON_B            0x00FF02FD
#define BUTTON_C            0x00FFC23D
#define BUTTON_UP           0x00FF9867
#define BUTTON_DOWN         0x00FF38C7
#define BUTTON_LEFT         0x00FF30CF
#define BUTTON_RIGHT        0x00FF7A85
#define BUTTON_CIRCLE       0x00FF18E7

int main(void)
{
    PLLInit();
    SystickInit();
    SetupSerial();

    PWMeasure_Init();

    volatile uint32_t pw;
    volatile uint32_t PORTB_DATA;
    volatile stateType *curState;
    curState = S0;

    uint32_t sequence = 0;
    int bitsRead = 0;
    int isDone;

    int mode = 1;

    volatile int delay = 0; // for delay purposes
    volatile int x = 0;

    volatile uint32_t ct0;
    volatile uint32_t ct1;
    volatile uint32_t ct2;
    volatile uint32_t ct3;
    int leds = 0x00;
    volatile bool buttonPressed = false;
    volatile bool buttonHeld = false;
    volatile bool powered = false;
    volatile long heldCounter = 0;

    volatile uint32_t button = 0x00000000;



    while (true){

        PORTB_DATA = GPIO_PORTB_DATA_R;
        delay = 0;
        delay = 0;
        isDone = done;
        delay = 0;
        delay = 0;
        pw = pulseWidth;
        delay = 0;
        delay = 0;
        if (isDone == 1)
        {

//          for (i = 0; i < 120; i++)
//          {
//              rest = itoa(pulseArray[i],res);
//              SerialWrite(rest);
//              SerialWriteLine("");
//          }


            ct0 = (curState->clocktick[0]);
            ct1 = (curState->clocktick[1]);
            ct2 = (curState->clocktick[2]);
            ct3 = (curState->clocktick[3]);

            //SerialWriteLine("One");
            if (ct0 <= pw && pw <= ct1)
            {
                if (curState->returns)
                {
                    if (curState == S3)
                    {
                        sequence = sequence << 1;
                        int output = curState->output[0];
                        sequence += output;
                        bitsRead++;
                    }
                    else
                    {
                        buttonHeld = true;
                        //SerialWriteLine("HELD");
                    }
                }

                //SerialWriteLine("Two");


                if (bitsRead == 32)
                {
                    curState = curState->outPutDone;
                    button = sequence;
                    sequence = 0;
                    bitsRead = 0;
                    buttonPressed = true;
                    buttonHeld = true;
                }
                else
                    curState = curState->NextState[0];
                //SerialWriteLine("Three");
                //If bits read = 32 then go to S4
            }
            else if (ct2 <= pw && pw <= ct3)
            {
                //SerialWriteLine("Four");
                if (curState->returns)
                {
                    if (curState == S3)
                    {
                        sequence = sequence << 1;
                        int output = curState->output[1];
                        sequence += output;
                        bitsRead++;
                    }
                    else
                    {
                        buttonHeld = true;
                    }
                }
                //SerialWriteLine("Five");
                //curState = curState->NextState[1];
                if (bitsRead == 32)
                {
                    curState = curState->outPutDone;
                    button = sequence;
                    sequence = 0;
                    bitsRead = 0;
                    buttonPressed = true;
                }
                else
                    curState = curState->NextState[1];
                //SerialWriteLine("Six");
            }
            else
            {
                //SerialWriteLine("Seven0");
                //el++;
                //SerialWriteLine("Seven1");
                curState = S0;
                //buttonHeld = false;
                buttonPressed = false;
                //heldCounter = -1;
                button = 0;
                bitsRead = 0;
                //SerialWriteLine("Seven2");
            }
            done = 0;
            //SerialWriteLine("Eight");
        }

//        if (buttonHeld)
//        {
//            heldCounter++;
//        }

        if (buttonPressed == true)
        {
            //bool circlePressed = false;
            switch(button)
            {
            case BUTTON_POWER:
//                if (mode == 1)
                    leds = 0x01;
//                else if (mode == 2){
//                    powered = !powered;
//                    if (powered){ leds = 0xFF;}
//                    else {leds = 0x00;}
//                }
                SerialWriteLine("BUTTON_POWER");
                break;
            case BUTTON_A:
                if (mode == 1)
                    leds = 0x02;
                else if (mode == 2){}
                SerialWriteLine("BUTTON_A");
                break;
            case BUTTON_B:
//                if (mode == 1)
                    leds = 0x03;
//                else if (mode == 2){}
                SerialWriteLine("BUTTON_B");
                break;
            case BUTTON_C:
//                if (mode == 1)
                    leds = 0x04;
//                else if (mode == 2){}
                SerialWriteLine("BUTTON_C");
                break;
            case BUTTON_UP:
//                if (mode == 1)
                    leds = 0x05;
//                else if (mode == 2){}
                SerialWriteLine("BUTTON_UP");
                break;
            case BUTTON_DOWN:
//                if (mode == 1)
                    leds = 0x06;
//                else if (mode == 2){}
                SerialWriteLine("BUTTON_DOWN");
                break;
            case BUTTON_LEFT:
//                if (mode == 1)
                    leds = 0x07;
//                else if (mode == 2){
//                    if (powered)
//                    {
//                        leds = leds << 1;
//                        if (leds > 0x20) {leds = 0x01;}
//                        if (heldCounter >= 4000000)
//                        {
//                            heldCounter = 1;
//                            leds = leds << 1;
//                            GPIO_PORTE_DATA_R = GPIO_PORTE_DATA_R << 1;
//                            if (leds > 0x20){ leds = 0x01; GPIO_PORTE_DATA_R = 0x01;}
//                        }
//
//                    }
//
//                }
                SerialWriteLine("BUTTON_LEFT");
                break;
            case BUTTON_RIGHT:
//                if (mode == 1)
                    leds = 0x08;
//                else if (mode == 2){
//                    if (powered)
//                    {
//                        leds = leds >> 1;
//                        if (leds == 0x00) leds = 0x20;
//                        if (heldCounter >= 4000000)
//                        {
//                            heldCounter = 2;
//                            leds = leds >> 1;
//                            GPIO_PORTE_DATA_R = GPIO_PORTE_DATA_R >> 1;
//                            if (leds == 0x00){ leds = 0x20; GPIO_PORTE_DATA_R = 0x20;}
//                        }
//                    }
//                }
                SerialWriteLine("BUTTON_RIGHT");
                break;
            case BUTTON_CIRCLE:
//                if (mode == 1)
                    leds = 0x09;
//                else if (mode == 2){
//                    if (powered) leds = 0x01;
//                }
                SerialWriteLine("BUTTON_CIRCLE");
                break;
            default:
                SerialWriteLine("Error: Unrecognized Button");
                break;
            }
            GPIO_PORTE_DATA_R = leds;
//            if (!buttonHeld)
//            {
                buttonPressed = false;
//            }
        } //

    }


}
