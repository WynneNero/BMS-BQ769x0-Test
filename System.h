/*----------------------------------------------------------------------------------------------------
 * Title: System.h
 * Authors: Nathaniel VerLee, 2020-2021
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
// Enumerations
//typedef enum {RED, YELLOW, GREEN} ColorState;

//----------------------------------------------------------------------------------------------------
//LED Feedback Colors
typedef enum {OFF, RED, YELLOW, GREEN} Color_t;

//----------------------------------------------------------------------------------------------------
// LED State (0 = off, 1 = on)
typedef struct
{
    uint8_t red;
    uint8_t green;
} LedState_t;

//----------------------------------------------------------------------------------------------------
//
typedef enum
{
    LED_MODE_OFF,
    LED_MODE_ON,
    LED_MODE_BLINK
} LEDMode_t;

//----------------------------------------------------------------------------------------------------
//
typedef struct
{
    uint8_t mode;
    uint8_t numBlinks;
    uint8_t LED_Blinks_CT;
} LedMode_t;



//----------------------------------------------------------------------------------------------------
// LED Control Struct
typedef struct
{
    volatile unsigned char *pxout;
    unsigned int redPin;
    unsigned int greenPin;
} Led_t;




enum LEDMode {LED_OFF, LED_BLINK, LED_ON};

//----------------------------------------------------------------------------------------------------
// Function Prototypes
void Init_GPIO(void);
void Init_Sys(void);

void Setup_LEDs(void);
void Setup_Buttons(void);
void Setup_GateDriver(void);
void Set_ChargePump_On(void);
void Set_ChargePump_Off(void);

//void Set_LED_Color(int LED, int Color);
//void Set_LED_State(int LED, int State);
void Register_Bit_Set(volatile unsigned char *reg,
                      unsigned int bit,
                      unsigned int value);

void Set_Led_State(const Led_t *led, Color_t color);


//----------------------------------------------------------------------------------------------------
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

#endif
