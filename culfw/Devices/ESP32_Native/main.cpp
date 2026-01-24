#include <Arduino.h>
// Wir binden C-Header in C++ ein
extern "C" {
  #include "board.h"
  #include "culfw.h"
}

void setup() {
    // Initialisierung des Systems
    // Hier rufen wir später die culfw-Init Funktionen auf
    Serial.begin(38400);
    Serial.println("ESP32 Native CULFW Booting...");
    
    // culfw init calls (Beispiele)
    // wdt_enable(WDTO_2S);
    // clock_prescale_set(clock_div_1);
    // LED_ON();
    // spi_init();
    // cc1100_init();
}

void loop() {
    // Die Hauptschleife der culfw
    // call_culfw_loop(); 
    delay(10);
}
