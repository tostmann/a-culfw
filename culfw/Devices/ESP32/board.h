#ifndef _BOARD_H
#define _BOARD_H

#ifdef ESP32_C3
#define BOARD_NAME          "ESP32-C3-CUL"
#define BOARD_ID_STR        "ESP32-C3-CUL"

// CC1101 Pins for ESP32-C3
// Moved from 4,5,6,7 to 6,5,7,10 to free up 4 for LED
#define SPI_SCLK  6
#define SPI_MISO  5
#define SPI_MOSI  7
#define SPI_SS    10
#define CC1100_CS_PIN SPI_SS

#define GDO0_PIN  2
#define GDO2_PIN  3

#define LED_PIN   4 // User requested LED on GPIO 4
#endif

#ifdef ESP32_C6
#define BOARD_NAME          "ESP32-C6-CUL"
#define BOARD_ID_STR        "ESP32-C6-CUL"

// CC1101 Pins for ESP32-C6 (Adjusted for DevKitC-1)
#define SPI_SCLK  21
#define SPI_MISO  20
#define SPI_MOSI  19
#define SPI_SS    18
#define CC1100_CS_PIN SPI_SS

#define GDO0_PIN  2
#define GDO2_PIN  3

#define LED_PIN   8 // Default RGB LED is usually on 8
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
