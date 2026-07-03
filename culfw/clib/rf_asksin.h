#ifndef _RF_ASKSIN_H
#define _RF_ASKSIN_H

#include <stdint.h>                     // for uint8_t

#include "board.h"                      // for HAS_ASKSIN_FUP

#define ASKSIN_WAIT_TICKS_CCA	188	//125 Hz

#ifndef HAS_ASKSIN_FUP
#define MAX_ASKSIN_MSG 30
#else
#define MAX_ASKSIN_MSG 50
#endif

#ifdef HAS_HMIP
#undef  MAX_ASKSIN_MSG
#define MAX_ASKSIN_MSG 64       // HmIP-Frames bis 61 Byte teilen sich den Puffer
#endif

#ifndef USE_RF_MODE
extern uint8_t asksin_on;
#endif

void rf_asksin_init(void);
void rf_asksin_task(void);
void asksin_func(char *in);

#ifdef HAS_HMIP
// HmIP-RX-Sniffer teilt sich PHY + Empfangs-Task mit AskSin (PHY ist
// byte-identisch).  hmip_on=1 laesst rf_asksin_task() zusaetzlich rohe
// 'P'-Zeilen ausgeben; die Laengen-Klassifikation entscheidet A/P/beide.
extern uint8_t hmip_on;
void hmip_func(char *in);
#endif

#endif
