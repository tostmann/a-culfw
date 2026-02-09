#ifndef _BOARD_H
#define _BOARD_H

#ifdef ESP32_C3
  #ifndef BOARD_NAME
    #define BOARD_NAME          "ESP32-C3-CUL"
  #endif
  #ifndef BOARD_ID_STR
    #define BOARD_ID_STR        "ESP32-C3-CUL"
  #endif

  // CC1101 Pins for ESP32-C3
  #ifndef SPI_SCLK
    #define SPI_SCLK  6
  #endif
  #ifndef SPI_MISO
    #define SPI_MISO  5
  #endif
  #ifndef SPI_MOSI
    #define SPI_MOSI  7
  #endif
  #ifndef SPI_SS
    #define SPI_SS    10
  #endif
  #ifndef CC1100_CS_PIN
    #define CC1100_CS_PIN SPI_SS
  #endif

  #ifndef GDO0_PIN
    #define GDO0_PIN  2
  #endif
  #ifndef GDO2_PIN
    #define GDO2_PIN  3
  #endif

  #ifndef LED_PIN
    #define LED_PIN   4
  #endif
#endif

#ifdef ESP32_C6
  #ifndef BOARD_NAME
    #define BOARD_NAME          "ESP32-C6-CUL"
  #endif
  #ifndef BOARD_ID_STR
    #define BOARD_ID_STR        "ESP32-C6-CUL"
  #endif

  // CC1101 Pins for ESP32-C6 (Adjusted for DevKitC-1)
  #ifndef SPI_SCLK
    #define SPI_SCLK  21
  #endif
  #ifndef SPI_MISO
    #define SPI_MISO  20
  #endif
  #ifndef SPI_MOSI
    #define SPI_MOSI  19
  #endif
  #ifndef SPI_SS
    #define SPI_SS    18
  #endif
  #ifndef CC1100_CS_PIN
    #define CC1100_CS_PIN SPI_SS
  #endif

  #ifndef GDO0_PIN
    #define GDO0_PIN  2
  #endif
  #ifndef GDO2_PIN
    #define GDO2_PIN  3
  #endif

  #ifndef LED_PIN
    #define LED_PIN   8
  #endif
#endif

#define HAS_UART
#define HAS_FASTRF
#define HAS_ASKSIN
#define HAS_MORITZ
#define HAS_RWE
#define HAS_RFNATIVE
#define HAS_INTERTECHNO
#define HAS_SOMFY_RTS
#define HAS_MBUS

#define CDC_BAUD_RATE 115200
#define TTY_BUFSIZE 128

#define GPIO_PIN_SET 1
#define GPIO_PIN_RESET 0

#define CC_INSTANCE 0
#define NUM_SLOWRF 1

#define E2END 511

#define RCV_BUCKETS 4

#define USB_IsConnected 0

#endif
