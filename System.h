/*----------------------------------------------------------------------------------------------------
 * Title: System.h
 * Authors: Nathaniel VerLee, Matthew Pennock, 2020-2021
 * Contributors: Ryan Heacock, Kurt Snieckus, Matthew Pennock, 2020-2021
 *
 * This file organizes the general system features like LEDs, buttons, Temp Sensors, Etc
 * and provides appropriate functions for accessing and modifying them
----------------------------------------------------------------------------------------------------*/

#ifndef SYSTEM_H
#define SYSTEM_H

//----------------------------------------------------------------------------------------------------
// This file includes:
#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
// Defines and Enumerations

#define BTN_PRESSED_LIM 3
#define BTN_LONGPRESS_LIM 80

//--------------------------------------------------
// GPIO Mappings

// Port Mapping for I2C_AFE
#define I2C_AFE_PSEL0 P1SEL0
#define I2C_AFE_SDA BIT2
#define I2C_AFE_SCL BIT3

// GPIO Mapping for I2C Alert 1 and 2
#define I2C_ALRT1_POUT P1OUT
#define I2C_ALRT1_PIN  P1IN
#define I2C_ALRT1_PDIR P1DIR
#define I2C_ALRT1_PREN P1REN
#define I2C_ALRT1 BIT1

#define I2C_ALRT2_POUT P2OUT
#define I2C_ALRT2_PDIR P2DIR
#define I2C_ALRT2_PREN P2REN
#define I2C_ALRT2 BIT5

// GPIO Mappings for Buttons:
#define BTNPWR_POUT P2OUT
#define BTNPWR_PIN  P2IN
#define BTNPWR_PDIR P2DIR
#define BTNPWR_PREN P2REN
#define BTNPWR BIT2

#define BTNPWR_IES P2IES
#define BTNPWR_IE P2IE
#define BTNPWR_IFG P2IFG

#define BTNFLT_POUT P2OUT
#define BTNFLT_PIN  P2IN
#define BTNFLT_PDIR P2DIR
#define BTNFLT_PREN P2REN
#define BTNFLT BIT3

#define BTNFLT_IES P2IES
#define BTNFLT_IE P2IE
#define BTNFLT_IFG P2IFG

// GPIO Mappings for all of the LEDs and LED Enable MOSFET:
#define LEDEN_POUT P3OUT
#define LEDEN_PDIR P3DIR
#define LEDEN BIT4

#define LEDA_POUT P2OUT
#define LEDA_PDIR P2DIR
#define LEDA_GRN BIT0
#define LEDA_RED BIT1

#define LEDB_POUT P4OUT
#define LEDB_PDIR P4DIR
#define LEDB_GRN BIT0
#define LEDB_RED BIT1

// GPIO Mappings for Gate Driver:
#define GTDRV_POUT P3OUT
#define GTDRV_PDIR P3DIR
#define GTDRV_BATMONEN BIT0
#define GTDRV_CPEN BIT1
#define GTDRV_PCHG BIT2

// GPIO Mappings for Debug Pins:
#define DBUGOUT_POUT P4OUT
#define DBUGOUT_PDIR P4DIR
#define DBUGOUT_1 BIT4
#define DBUGOUT_2 BIT5

//----------------------------------------------------------------------------------------------------
// ENUMS

//----------------------------------------------------------------------------------------------------
// This enumeration names the states of the button handler state machine
typedef enum BTNMode_enum
{   NPRESSED,
    PRESSED,
    SHORT_PRESSED,
    LONG_PRESSED,
    LONG_IDLE
} BTNState_t;

//----------------------------------------------------------------------------------------------------
// Possible Modes a BiColor LED can be operating in
typedef enum
{   LEDMode_STATIC,
    LEDMode_BLINK
} LEDMode_t;

//----------------------------------------------------------------------------------------------------
// Enumerations for types of colors a RG BiColor LED can be
typedef enum
{   BiColor_OFF,
    BiColor_RED,
    BiColor_YELLOW,
    BiColor_GREEN
} BiColor_t;

//----------------------------------------------------------------------------------------------------
// STRUCTS

//----------------------------------------------------------------------------------------------------
// Button struct stores relevant registers and counters for each button
typedef struct
{   volatile unsigned char *PXIN;
    unsigned int Pin;
    unsigned int BTN_CT;
    BTNState_t State;
} Button_t;

//----------------------------------------------------------------------------------------------------
// ColorState struct holds the value for each LED pin based on the color chosen
typedef struct
{   uint8_t red;
    uint8_t green;
} ColorState_t;

//----------------------------------------------------------------------------------------------------
// Struct for holding all data needed about a specific BiColor LED
typedef struct
{   //Data for all modes:
    volatile unsigned char *PXOUT;
    unsigned int Pin_Red;
    unsigned int Pin_Green;
    LEDMode_t LED_Mode;

    //Data for Blink Mode:
    BiColor_t LED_Color;
    BiColor_t Next_Color;
    unsigned int Blink_PeriodCT;
    unsigned int Blinks_LIM;
    unsigned int Next_LIM;
    unsigned int Blinks_CT;

} BiColorLED_t;

//----------------------------------------------------------------------------------------------------
// FUNCTION PROTOTYPES

void Init_GPIO(void);
void Init_Sys(void);

void Setup_Buttons(void);
void Setup_LEDs(void);
void Setup_GateDriver(void);

void Set_ChargePump_On(void);
void Set_ChargePump_Off(void);

void Register_Bit_Set(volatile unsigned char *reg, unsigned int bit, unsigned int value);
uint8_t Register_Bit_Get(volatile unsigned char *reg, unsigned int bit);

BTNState_t Button_Handler(Button_t *button);

void Set_LED_Static (BiColorLED_t *led, BiColor_t color);
void Set_LED_Blinks (BiColorLED_t *led, BiColor_t color, unsigned int blinks);
void LED_BlinkHandler(BiColorLED_t *led, unsigned int cycleCT);

//------------------------------------------------------
// (KRS) export LEDs so you can access them from other files
extern BiColorLED_t LEDA;
extern BiColorLED_t LEDB;

extern Button_t BTN_PWR;
extern Button_t BTN_FLT;

#endif
