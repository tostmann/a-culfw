#ifndef _CULFW_H_
#define _CULFW_H_

#include "board.h"

// Wichtige globale Variablen aus der clib bekannt machen
extern uint16_t credit_10ms;

// Prototypen für die Hauptschleife (normalerweise in den clib-Headern)
void CC1100_task(void);
void call_culfw_loop(void);

#endif
