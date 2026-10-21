/* Board support for the radio co-processor of the RWE / innogy SmartHome Controller
 * 2.0 (SHC2).
 *
 * The S500B-EXT extension board of that box carries an ATmega328P driving a CC110L
 * (26 MHz reference, 868 MHz), reachable from the Actions S500 host over a 57600 baud
 * UART. Same pin-out as Devices/SHC (SHC v1); differences to Devices/nanoCUL:
 *
 *   - 11.0592 MHz crystal instead of 8/16 MHz  (see the clock note below)
 *   - CC110L CS on PB2, GDO0 on PD2, GDO2 on PD3 -> RX interrupt is INT1, not INT0
 *   - no user LED, no 433 MHz variant
 *
 * IMPORTANT - the SPI bus is shared: the host programs this ATmega328P in-circuit
 * over the very same MOSI/MISO/SCK lines that carry the CC110L traffic. On the S500
 * these are DSI pads; left in DSI mode they pull the lines low and the ATmega cannot
 * talk to the transceiver reliably. The host has to switch them to high-impedance
 * GPIO inputs before the firmware uses the radio, see README.
 */

#ifndef _BOARD_H
#define _BOARD_H

#include <avr/io.h>
#include <stdint.h>

/* The board is clocked by an 11.0592 MHz crystal, so HAS_16MHZ_CLOCK must stay off:
 * that path calls clock_prescale_set(clock_div_2) and would leave us at 5.5296 MHz.
 * 57600/38400 baud are exact at 11.0592 MHz, with and without U2X.
 *
 * The SlowRF time base (Timer1 pulse measurement, my_delay_us, Timer0 tick) is
 * scaled to the real core clock, see clib/timebase.h. Timer0 then ticks at
 * 11059200/1024/86 = 125.58 Hz instead of 125 Hz (+0.47 %). */
#define CULFW_CORE_HZ           F_CPU
/* #define HAS_16MHZ_CLOCK */

/* if you are using a CC1101 module for 868MHz disable the next line */
#if defined (shc2CUL433)
#define HAS_CC1100_433
#endif

#define SPI_PORT		PORTB
#define SPI_DDR			DDRB
#define SPI_SS			2
#define SPI_MISO		4
#define SPI_MOSI		3
/* die aufgelötete gelbe LED ist an PB5/SCLK angeschlossen! */
#define SPI_SCLK		5

/* CC1100 chip select: ATmega328P pin 14 = PB2, pulled up on the board */
#define CC1100_CS_DDR		DDRB
#define CC1100_CS_PORT          PORTB
#define CC1100_CS_PIN		PB2


/* CC1101 GDO0 Tx / Temperature Sensor */
#if 0
#define CC1100_OUT_DDR		DDRC
#define CC1100_OUT_PORT         PORTC
#define CC1100_OUT_PIN          PC0
#define CC1100_OUT_IN           PINC
#define CCTEMP_MUX              CC1100_OUT_PIN
#else
/* GDO0: ATmega328P pin 32 = PD2 */
#define CC1100_OUT_DDR		DDRD
#define CC1100_OUT_PORT         PORTD
#define CC1100_OUT_PIN          PD2
#define CC1100_OUT_IN           PIND
#define CCTEMP_MUX              CC1100_OUT_PIN
#endif

/* CC1101 GDO2 Rx Interrupt */
/* GDO2 (RX interrupt): ATmega328P pin 1 = PD3 = INT1 - note this is INT1, not INT0
 * as on the nanoCUL, because GDO0 and GDO2 are swapped on this board. */
#define CC1100_IN_DDR		DDRD
#define CC1100_IN_PORT          PIND
#define CC1100_IN_PIN           PD3
#define CC1100_IN_IN            PIND

#define CC1100_INT		INT1
#define CC1100_INTVECT          INT1_vect
#define CC1100_ISC		ISC10
#define CC1100_EICR             EICRA

/* externe LED */
/* This board has no user LED. PB1 is unused here, so the LED macros point at it;
 * nothing is driven that would collide with another function. */
#define LED_DDR                 DDRB
#define LED_PORT                PORTB
#define LED_PIN                 1

//#define LED_ON_DDR              DDRB
//#define LED_ON_PORT             PORTB
//#define LED_ON_PIN              1


#define BOARD_ID_STR            "shc2CUL868"
#define BOARD_ID_STR433         "shc2CUL433"

/* define this device as a 433 MHz one */
/* this isn't done like a CUL by reading a port pin but instead a fixed value of 0 for mark433_pin is used */ 
#define MULTI_FREQ_DEVICE
#define MARK433_PIN 			mark433_pin
#define MARK433_BIT             0
extern const uint8_t mark433_pin;

#define HAS_UART
#define UART_BAUD_RATE          57600

/* ATMega328P has only one UART, no need to define the UART to use */
//#define USART_RX_vect           USART0_RX_vect
//#define USART_UDRE_vect         USART0_UDRE_vect

#define TTY_BUFSIZE             128


#define RCV_BUCKETS            2      //                 RAM: 25b * bucket
#define FULL_CC1100_PA                // PROGMEM:  108b
#define HAS_RAWSEND                   //
#define HAS_FASTRF                    // PROGMEM:  468b  RAM:  1b
#define HAS_ASKSIN
/* Intertechno Senden einschalten */
#define HAS_INTERTECHNO



#define HAS_CC1101_RX_PLL_LOCK_CHECK_TASK_WAIT
#define HAS_CC1101_PLL_LOCK_CHECK_MSG
#define HAS_CC1101_PLL_LOCK_CHECK_MSG_SW
/* HAS_MBUS requires about 1kB RAM, if you want to use it you
   should consider disabling other unneeded features
   to avoid stack overflows
*/
//#define HAS_MBUS

#  define HAS_TX3
#  define HAS_UNIROLL

//#  define HAS_SOMFY_RTS
#  define HAS_RFNATIVE

//#  define HAS_MEMFN

#if defined (shc2CUL433)
/* Intertechno Empfang einschalten */
#  define HAS_IT
#  define HAS_REVOLT
#  define HAS_TCM97001
#  define HAS_HOMEEASY
#  define HAS_BELFOX
#  define HAS_MANCHESTER
#endif

#if defined (shc2CUL868)
#  define HAS_ASKSIN_FUP
#  define HAS_MORITZ
#  define HAS_RWE
#  define HAS_ESA
//#  define HAS_HOERMANN
//#  define HAS_HOERMANN_SEND
#  define HAS_HMS
#  define OFF_LACROSSE_HMS_EMU          // if you like HMS emulation for LaCrosse temp devices

//#define HAS_SOMFY_RTS
//#define HAS_FHT_80b                     // PROGMEM: 1374b, RAM: 90b
//#define HAS_FHT_8v                    // PROGMEM:  586b  RAM: 23b
//#define HAS_FHT_TF
//#define FHTBUF_SIZE          174      //                 RAM: 174b
//#define HAS_KOPP_FC
//#define HAS_ZWAVE                     // PROGMEM:  882
#endif

#endif
