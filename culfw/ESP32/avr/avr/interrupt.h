#ifndef _AVR_INTERRUPT_H
#define _AVR_INTERRUPT_H
#pragma GCC system_header

#ifdef __cplusplus
extern "C" {
#endif

#ifndef sei
#define sei()
#endif
#ifndef cli
#define cli()
#endif

#ifdef __cplusplus
}
#endif

#endif
