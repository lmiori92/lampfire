/**
 *
 *  LampFire: a simple but effective fire effect using incandescent lamps
 *
 *  The attiny85 microcontroller is used but I think all attinyXX are compatible
 *  or compatible with minor changes.
 *
 *  Power supply: +5V (any USB charger should be fine)
 *
 *  Fuse configuration:
 *  -U lfuse:w:0xe2:m -U hfuse:w:0xd4:m -U efuse:w:0xff:m
 *
 *  Using usbasp programmer:
 *    avrdude -p attiny85 -c usbasp -U lfuse:w:0xe2:m -U hfuse:w:0xd4:m -U efuse:w:0xff:m -B 10
 *    Note: -B switch is used to allow working commnication on slow internal default clock (1MHz)
 *
 *  Compiler: avr-gcc
 *  IDE: of your choice; I have used Eclipse with the AVR plugins.
 *
 *  Changelog:
 *
 *      28.11.2020 - Initial version with 2 channels
 *
 *  Copyright (c) 2020   Lorenzo Miori
 *  Permission is hereby granted, free of charge, to any person
 *  obtaining a copy of this software and associated documentation
 *  files (the "Software"), to deal in the Software without
 *  restriction, including without limitation the rights to use,
 *  copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following
 *  conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *  OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/** PIN definitions for the LAMP 0 output - mapped to PB0 **/
#define LAMP_0_PORT     PORTB
#define LAMP_0_PIN      PB0
#define LAMP_0_DDR      DDRB
#define LAMP_0_DDR_PIN  DDB0

/** PIN definitions for the LAMP 1 output - mapped to PB1 **/
#define LAMP_1_PORT     PORTB
#define LAMP_1_PIN      PB1
#define LAMP_1_DDR      DDRB
#define LAMP_1_DDR_PIN  DDB1

/** Helper macros to turn the lamps on and off **/
#define LAMP_0_ON   (LAMP_0_PORT |= (_BV(LAMP_0_PIN)))
#define LAMP_0_OFF  (LAMP_0_PORT &= ~(_BV(LAMP_0_PIN)))
#define LAMP_1_ON   (LAMP_1_PORT |= (_BV(LAMP_1_PIN)))
#define LAMP_1_OFF  (LAMP_1_PORT &= ~(_BV(LAMP_1_PIN)))

/** Minimal duty cycle to apply to the outputs.
 * This allows to avoid turning the lamps off completely.
 * A fire is not disappearing normally :-) */
#define LAMP_MIN_PWM        (50)

/** Enable a random delay to be awaited before computing
 * the next duty cycle values
 */
#define LAMP_HAS_RANDOM_DELAY

/** Random generator seed */
uint16_t randreg;

/**
 * Pseudo random number generator.
 * I have found this piece of code in the Internet, but I
 * couldn't find the original author of the snipped.
 * I assume it is in the public domain; also its logic is nothing
 * more than a shifter.
 *
 * @return
 */
static uint16_t pseudorandom16(void){

    uint16_t newbit = 0;

    if (randreg == 0)
    {
        randreg = 1;
    }
    if (randreg & 0x8000) newbit = 1;
    if (randreg & 0x4000) newbit ^= 1;
    if (randreg & 0x1000) newbit ^= 1;
    if (randreg & 0x0008) newbit ^= 1;

    randreg = (randreg << 1) + newbit;

    return randreg;
}

/**
 * Function to compute the pseudo-random duty cycle value.
 * It applies a minimum value i.e. the lamp will not completely
 * turn off at any time.
 * @return the generated duty cycle value
 */
static uint8_t get_duty_cycle(void)
{
    uint16_t lamp_duty_cycle = pseudorandom16();
    lamp_duty_cycle %= 255U;

    if (lamp_duty_cycle < LAMP_MIN_PWM)
    {
        lamp_duty_cycle = LAMP_MIN_PWM;
    }

    return (uint8_t)lamp_duty_cycle;
}

/**
 * Entry point mapped to the reset vector
 * @return will never return as it loops indefinitely.
 */
int main(void)
{
    /* ensure power supply is stable waiting 100ms */
    _delay_ms(100);

    /* initialize lamps as outputs */
    LAMP_0_DDR  |= _BV(LAMP_0_DDR_PIN);  /* output  */
    LAMP_1_DDR  |= _BV(LAMP_1_DDR_PIN);  /* output  */

    /* startup test */
    LAMP_0_ON;
    _delay_ms(750);
    LAMP_0_OFF;
    LAMP_1_ON;
    _delay_ms(750);
    LAMP_1_OFF;

    /* initialize the PWM for LAMP 0 and LAMP 1 */
    TCCR0A |= _BV(COM0A1);  /* Clear on compare match; set at bottom */
    TCCR0A |= _BV(COM0B1);  /* Clear on compare match; set at bottom */
    /* Set fast PWM mode */
    TCCR0A |= _BV(WGM00);
    TCCR0A |= _BV(WGM01);
    /* Set clock prescaler to 1024 (8MHz / 1024) */
    TCCR0B |= _BV(CS00);
    TCCR0B |= _BV(CS02);

    /* set the minimal duty cycle first */
    OCR0A = LAMP_MIN_PWM;
    OCR0B = LAMP_MIN_PWM;

    /* main application loop */
    while(1)
    {
        /* compute the random duty cycle value and set it to the timer */
        OCR0A = get_duty_cycle();
        OCR0B = get_duty_cycle();

#ifdef LAMP_HAS_RANDOM_DELAY
        /* random cycle delay */
        uint16_t random_cycle_delay = pseudorandom16();
        random_cycle_delay %= 50;
        for (uint8_t i = 0; i < random_cycle_delay; i++)
        {
            _delay_ms(1U);
        }
#endif
    }

   return 0;
}
