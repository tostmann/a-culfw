#ifndef _BOARD_H
#define _BOARD_H

// Feature definitions
#define BOARD_ID_STR            "CUL868"
#define BOARD_ID_USTR           L"CUL868"

#define MULTI_FREQ_DEVICE	// available in multiple versions: 433MHz,868MHz
#define BOARD_ID_STR433         "CUL433"
#define BOARD_ID_USTR433        L"CUL433"

#define HAS_USB                  1
#define USB_BUFSIZE             64      // Must be a supported USB endpoint size
#define USB_MAX_POWER	       100
#define HAS_FHT_80b                     // PROGMEM: 1374b, RAM: 90b
#define HAS_RF_ROUTER                   // PROGMEM: 1248b  RAM: 44b
#define RFR_FILTER                      // PROGMEM:   90b  RAM:  4b
#define HAS_HOERMANN
#define HAS_HOERMANN_SEND               // PROGMEM:  220
#define HAS_CC1101_RX_PLL_LOCK_CHECK_TASK_WAIT	// PROGMEM: 118b
#define HAS_CC1101_PLL_LOCK_CHECK_MSG		// PROGMEM:  22b
#define HAS_CC1101_PLL_LOCK_CHECK_MSG_SW	// PROGMEM:  22b

#undef  RFR_DEBUG                       // PROGMEM:  354b  RAM: 14b
#undef  HAS_FASTRF                      // PROGMEM:  468b  RAM:  1b

#if defined(CUL_V3_ZWAVE)
#  define CUL_V3
#endif

#if defined(CUL_V3) || defined(CUL_V4)
#  define HAS_FHT_8v                    // PROGMEM:  586b  RAM: 23b
#  define HAS_FHT_TF
#  define FHTBUF_SIZE          174      //                 RAM: 174b
#  define RCV_BUCKETS            4      //                 RAM: 25b * bucket
#  define FULL_CC1100_PA                // PROGMEM:  108b
#  define HAS_RAWSEND                   //
#  define HAS_ASKSIN                    // PROGMEM: 1314
#  define HAS_ASKSIN_FUP                // PROGMEM:   78
#  define HAS_HMIP                      // PROGMEM:  ~600  (RX-Sniffer, raw 'P')
#ifndef TTYSBU
#  define HAS_KOPP_FC
#endif
#  define HAS_RWE
#  define HAS_TX3                       // PROGMEM:  168
#  define HAS_INTERTECHNO               // PROGMEM: 1352
#  define HAS_UNIROLL                   // PROGMEM:   92
#  define HAS_MEMFN                     // PROGMEM:  168
#  define HAS_SOMFY_RTS                 // PROGMEM: 1716

#  if defined(_433MHZ)
#    define HAS_TCM97001                  // PROGMEM:  264
#    define HAS_IT
#    define HAS_HOMEEASY
#    if defined(CUL_V3)
#      define HAS_MANCHESTER
#      define HAS_REVOLT
#    endif
#    define HAS_BELFOX                    // PROGMEM:  214

#  endif

#if defined(_868MHZ)
#    define HAS_HMS
#    if defined(CUL_V3)
#      define HAS_ESA                     // PROGMEM:  286
#  endif
#    define HAS_MORITZ                    // PROGMEM: 1696
#endif


#endif

#if defined(CUL_V4)
#  define HAS_ZWAVE                     // PROGMEM:  882
#  define TTY_BUFSIZE           64      // RAM: TTY_BUFSIZE*4
#endif

#if defined(CUL_V3)
#ifdef TTYSBU
#  define TTY_BUFSIZE           64      // RAM: TTY_BUFSIZE*4 (smaller: the SBU UART needs the room)
#else
#  define TTY_BUFSIZE          128      // RAM: TTY_BUFSIZE*4
#endif
#if defined(_868MHZ)
#  define HAS_MBUS                      // PROGMEM: 2536
#  define MBUS_NO_TX                       // PROGMEM:  962
#  define HAS_RFNATIVE                  // PROGMEM:  580
//#  define LACROSSE_HMS_EMU              // PROGMEM: 2206
#ifndef TTYSBU
#  define HAS_KOPP_FC                   // PROGMEM: 3370
#endif
#endif
#endif


#if defined(CUL_V3_ZWAVE)
#  define HAS_ZWAVE                     // PROGMEM:  882
#  undef HAS_MBUS
#  undef HAS_KOPP_FC
#  undef HAS_RFNATIVE
#  define LACROSSE_HMS_EMU              // PROGMEM: 2206
#endif


#ifdef CUL_V2
#  define TTY_BUFSIZE           48
#  define FHTBUF_SIZE           74
#  define RCV_BUCKETS            2 
#  define RFR_SHADOW                    // PROGMEM: 10b    RAM: -(TTY_BUFSIZE+3)
#  define HAS_TX3
#  define NO_RF_DEBUG                   // squeeze out some bytes for hoermann_send
#  undef  HAS_CC1101_RX_PLL_LOCK_CHECK_TASK_WAIT
#endif

#ifdef CUL_V2_HM
#  define CUL_V2
#  define HAS_ASKSIN
#  define TTY_BUFSIZE           64
#  define RCV_BUCKETS            2 
#  undef  HAS_RF_ROUTER
#  undef  HAS_FHT_80b
#  define FHTBUF_SIZE            0
#  undef  BOARD_ID_STR
#  define BOARD_ID_STR            "CUL_HM"
#  undef  BOARD_ID_USTR
#  define BOARD_ID_USTR           L"CUL_HM"
#  define HAS_INTERTECHNO
#  define HAS_HMS
#endif

#ifdef CUL_V2_MAX
#  define CUL_V2
#  define HAS_MORITZ
#  define TTY_BUFSIZE           64
#  define RCV_BUCKETS            2
#  undef  HAS_RF_ROUTER
#  undef  HAS_FHT_80b
#  define FHTBUF_SIZE            0
#  undef  BOARD_ID_STR
#  define BOARD_ID_STR            "CUL_MX"
#  undef  BOARD_ID_USTR
#  define BOARD_ID_USTR           L"CUL_MX"
#  define HAS_INTERTECHNO
#endif

// No features to define below

#include <avr/io.h>
#include <avr/power.h>

#if !defined(clock_prescale_set) && __AVR_LIBC_VERSION__  < 10701UL
#  warning "avr/power.h needs patching for prescaler functions to work."
#  warning "for the m32u4 add __AVR_ATmega32U4__ for cpu types on prescale block"
#  warning "for the m32u2 add __AVR_ATmega32U2__ for cpu types on prescale block"
#endif

#if defined(CUL_V3)      // not sure why libc is missing those ...
#  define PB0 PORTB0
#  define PB1 PORTB1
#  define PB2 PORTB2
#  define PB3 PORTB3
#  define PB6 PORTB6
#  define PD2 PORTD2
#  define PD3 PORTD3
#endif  // CUL_V3

#define SPI_PORT		PORTB
#define SPI_DDR			DDRB
#define SPI_SS			PB0
#define SPI_MISO		PB3
#define SPI_MOSI		PB2
#define SPI_SCLK		PB1

#if defined(CUL_V4)
#  define CC1100_CS_DDR		SPI_DDR
#  define CC1100_CS_PORT        SPI_PORT
#  define CC1100_CS_PIN		SPI_SS
#  define CC1100_OUT_DDR        DDRD
#  define CC1100_OUT_PORT       PORTD
#  define CC1100_OUT_PIN        PD3
#  define CC1100_IN_DDR         DDRD
#  define CC1100_IN_PORT        PIND
#  define CC1100_IN_PIN         PD2
#  define CC1100_INT		INT2
#  define CC1100_INTVECT        INT2_vect
#  define CC1100_ISC		ISC20
#  define CC1100_EICR           EICRA
#  define LED_DDR               DDRC
#  define LED_PORT              PORTC
#  define LED_PIN               PC5
#endif

#if defined(CUL_V3)
#  define CC1100_CS_DDR		SPI_DDR
#  define CC1100_CS_PORT        SPI_PORT
#  define CC1100_CS_PIN		SPI_SS
#  define CC1100_OUT_DDR        DDRD
#  define CC1100_OUT_PORT       PORTD
#ifdef TTYSBU
   /* In SBU mode the software UART owns PD2/PD3, so the CC1101 data lines move
      to PD0/PD1. The pin numbers become runtime expressions on sbu_mode. */
   extern uint8_t sbu_mode;
#  define CC1100_OUT_PIN        (sbu_mode ? PD1 : PD3)
#else
#  define CC1100_OUT_PIN        PD3
#endif
#  define CC1100_OUT_IN         PIND
#  define CC1100_IN_DDR		DDRD
#  define CC1100_IN_PORT        PIND
#ifdef TTYSBU
#  define CC1100_IN_PIN         (sbu_mode ? PD0 : PD2)
#else
#  define CC1100_IN_PIN         PD2
#endif
#  define CC1100_IN_IN          PIND
#  define CC1100_INT		INT2
#  define CC1100_INTVECT        INT2_vect
#  define CC1100_ISC		ISC20
#  define CC1100_EICR           EICRA
#  define LED_DDR               DDRE
#  define LED_PORT              PORTE
#  define LED_PIN               6
#endif

#if defined(CUL_V2)
#  define CC1100_CS_DDR		DDRC
#  define CC1100_CS_PORT        PORTC
#  define CC1100_CS_PIN		PC5
#  define CC1100_IN_DDR		DDRC
#  define CC1100_IN_PORT        PINC
#  define CC1100_IN_PIN         PC7
#  define CC1100_OUT_DDR	DDRC
#  define CC1100_OUT_PORT       PORTC
#  define CC1100_OUT_PIN        PC6
#  define CC1100_INT		INT4
#  define CC1100_INTVECT        INT4_vect
#  define CC1100_ISC		ISC40
#  define CC1100_EICR           EICRB
#  define LED_DDR               DDRC
#  define LED_PORT              PORTC
#  define LED_PIN               PC4
#endif

#if defined(CUL_V3)
#  define CUL_HW_REVISION "CUL_V3"
#elif defined(CUL_V4)
#  define CUL_HW_REVISION "CUL_V4"
#else
//#  define CUL_HW_REVISION "CUL_V2"    // No more mem for this feature
#endif

#define MARK433_PORT            PORTB
#define MARK433_PIN             PINB
#define MARK433_BIT             6
#define MARK915_PORT            PORTB
#define MARK915_PIN             PINB
#define MARK915_BIT             5


/* ------------------------------------------------------------------------
 * SLIM_HM_BUILD — schlanker Build mit Fokus auf HomeMatic (BidCos + HmIP).
 *
 * Schaltet Protokolle ab, die neben HomeMatic nicht gebraucht werden, und
 * schafft damit Luft fuer Erweiterungen. Gemessen auf CUL_V3 (atmega32u4):
 *   voll:    30250 B Flash (inkl. Bootloader), 2345 B RAM (91,6 % voll)
 *   schlank: 17784 B Flash,                     992 B RAM (38,8 %)
 *   => rund 12,5 KB Flash und 1,35 KB RAM frei.
 * Erhalten bleiben ASKSIN(+FUP), HMIP, FHT, RF_ROUTER, RAWSEND, MEMFN, USB.
 * Die Groessenangaben je Protokoll stehen in den Kommentaren oben.
 *
 * Aufruf:  make ... EXTRA_CFLAGS=-DSLIM_HM_BUILD
 * ---------------------------------------------------------------------- */
#ifdef SLIM_HM_BUILD
#  undef HAS_MBUS          /* 2536 B */
#  undef HAS_KOPP_FC       /* 3370 B */
#  undef HAS_SOMFY_RTS     /* 1716 B */
#  undef HAS_MORITZ        /* 1696 B */
#  undef HAS_INTERTECHNO   /* 1352 B */
#  undef HAS_RFNATIVE      /*  580 B */
#  undef HAS_RWE
#  undef HAS_UNIROLL       /*   92 B */
#  undef HAS_HOERMANN
#  undef HAS_HOERMANN_SEND /*  220 B */
#  undef HAS_TX3           /*  168 B */
#  undef HAS_ESA           /*  286 B */
#  undef HAS_HMS
#endif

#endif // __BOARD_H__
