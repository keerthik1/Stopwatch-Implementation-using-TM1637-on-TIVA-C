#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

// TM1637 Pin Definitions
#define TM1637_CLK_PIN GPIO_PIN_0 // PB0
#define TM1637_DIO_PIN GPIO_PIN_1 // PB1
#define TM1637_PORT GPIO_PORTB_BASE

// Keypad Pin Definitions
#define ROW_PORT GPIO_PORTE_BASE
#define COL_PORT GPIO_PORTD_BASE
#define ROW_PINS (GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4)
#define COL_PINS (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3)


#define GPIO_PORTB_BASE 0x40005000
#define GPIO_PORTD_BASE 0x40007000
#define GPIO_PORTE_BASE 0x40024000
#define GPIO_PORTA_BASE 0x40004000
#define TIMER0_BASE 0x40030000

// TM1637 Commands
#define DATA_COMMAND 0x40
#define ADDR_COMMAND 0xC0
#define CTRL_COMMAND 0x8F

// Digit segment data
const uint8_t digitToSegment[] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F // 0-9
};

// Global variables
volatile uint8_t seconds = 0, minutes = 0;
volatile bool running = false;
volatile bool colon = true;

// Function prototypes
void TM1637_Init(void);
void TM1637_Start(void);
void TM1637_Stop(void);
void TM1637_SendByte(uint8_t byte);
void TM1637_Display(void);
void Keypad_Init(void);
char Keypad_GetKey(void);
void Timer0_Init(void);
void Timer0A_Handler(void);

int main(void) {
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    TM1637_Init();
    Keypad_Init();
    Timer0_Init();
    IntMasterEnable();

    TM1637_Display(); // Initial display "00:00"

    while (1) {
        char key = Keypad_GetKey();
        if (key == 'A') { // Start
            running = true;
        } else if (key == 'B') { // Stop
            running = false;
        } else if (key == 'C') { // Reset
            running = false;
            seconds = 0;
            minutes = 0;
            TM1637_Display();
        }
    }
}

void TM1637_Init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));
    GPIOPinTypeGPIOOutput(TM1637_PORT, TM1637_CLK_PIN | TM1637_DIO_PIN);
    GPIOPinWrite(TM1637_PORT, TM1637_CLK_PIN | TM1637_DIO_PIN, TM1637_CLK_PIN | TM1637_DIO_PIN);
}

void TM1637_Start(void) {
    GPIOPinWrite(TM1637_PORT, TM1637_CLK_PIN, TM1637_CLK_PIN);
    GPIOPinWrite(TM1637_PORT, TM1637_DIO_PIN, TM1637_DIO_PIN);
    SysCtlDelay(1000);
    GPIOPinWrite(TM1637_PORT, TM1637_DIO_PIN, 0);
    SysCtlDelay(1000);
}

void TM1637_Stop(void) {
    GPIOPinWrite(TM1637_PORT, TM1637_CLK_PIN, 0);
    GPIOPinWrite(TM1637_PORT, TM1637_DIO_PIN, 0);
    SysCtlDelay(1000);
    GPIOPinWrite(TM1637_PORT, TM1637_CLK_PIN, TM1637_CLK_PIN);
    SysCtlDelay(1000);
    GPIOPinWrite(TM1637_PORT, TM1637_DIO_PIN, TM1637_DIO_PIN);
}

void TM1637_SendByte(uint8_t byte) {
    int i;
    for (i = 0; i < 8; i++) {
        GPIOPinWrite(TM1637_PORT, TM1637_CLK_PIN, 0);
        SysCtlDelay(1000);
        GPIOPinWrite(TM1637_PORT, TM1637_DIO_PIN, (byte & (1 << i)) ? TM1637_DIO_PIN : 0);
        SysCtlDelay(1000);
        GPIOPinWrite(TM1637_PORT, TM1637_CLK_PIN, TM1637_CLK_PIN);
        SysCtlDelay(1000);
    }
    GPIOPinWrite(TM1637_PORT, TM1637_CLK_PIN, 0);
    SysCtlDelay(1000);
    GPIOPinWrite(TM1637_PORT, TM1637_CLK_PIN, TM1637_CLK_PIN);
    SysCtlDelay(1000);
}

void TM1637_Display(void) {
    uint8_t digits[4] = {minutes / 10, minutes % 10, seconds / 10, seconds % 10};

    TM1637_Start();
    TM1637_SendByte(DATA_COMMAND);
    TM1637_Stop();

    TM1637_Start();
    TM1637_SendByte(ADDR_COMMAND);
    TM1637_SendByte(digitToSegment[digits[0]]);
    TM1637_SendByte(digitToSegment[digits[1]] | (colon ? 0x80 : 0)); // Colon on digit 2
    TM1637_SendByte(digitToSegment[digits[2]]);
    TM1637_SendByte(digitToSegment[digits[3]]);
    TM1637_Stop();

    TM1637_Start();
    TM1637_SendByte(CTRL_COMMAND);
    TM1637_Stop();
}

void Keypad_Init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE) || !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));
    GPIOPinTypeGPIOOutput(ROW_PORT, ROW_PINS);
    GPIOPinWrite(ROW_PORT, ROW_PINS, 0x00);
    GPIOPinTypeGPIOInput(COL_PORT, COL_PINS);
    GPIOPadConfigSet(COL_PORT, COL_PINS, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
}

char Keypad_GetKey(void) {
    const char keys[4][4] = {{'1', '2', '3', 'A'}, {'4', '5', '6', 'B'}, {'7', '8', '9', 'C'}, {'*', '0', '#', 'D'}};
    int row, col;
    for (row = 0; row < 4; row++) {
        GPIOPinWrite(ROW_PORT, ROW_PINS, (1 << (row + 1)));
        uint8_t col_state = GPIOPinRead(COL_PORT, COL_PINS);
        if (col_state) {
            for (col = 0; col < 4; col++) {
                if (col_state & (1 << col)) {
                    GPIOPinWrite(ROW_PORT, ROW_PINS, 0x00);
                    SysCtlDelay(8000000); // Debounce
                    return keys[row][col];
                }
            }
        }
    }
    GPIOPinWrite(ROW_PORT, ROW_PINS, 0x00);
    return 0;
}

void Timer0_Init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() - 1); // 1s at 80 MHz
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER0A);
    TimerEnable(TIMER0_BASE, TIMER_A);
}

void Timer0A_Handler(void) {
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    if (running) {
        seconds++;
        if (seconds >= 60) {
            seconds = 0;
            minutes++;
            if (minutes >= 60) minutes = 0;
        }
        colon = !colon; // Blink colon
        TM1637_Display();
    }
}
