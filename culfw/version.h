/* VERSION_1 / VERSION_2 are NOT the firmware version. fncollection.c uses
   them as the EEPROM magic at EE_MAGIC_OFFSET: on boot the two bytes are
   compared against the stored ones, and a mismatch makes the device rewrite
   its whole EEPROM with the defaults. Raising them therefore wipes every
   user's configuration on update. Change them only when an EEPROM layout
   change makes that reset the intended behaviour -- never to track a release
   number. The release number lives in BASE_VERSION below, which is why the
   two have been out of step since 1.27. */
#define VERSION_1               1
#define VERSION_2               26

/* Das ist unsere Basis-Version (statisch) */
#define BASE_VERSION            "1.27"
#define FW_NAME                 "a-culfw"

/* Der finale VERSION String wird normalerweise vom Build-Script 
   per Compiler-Flag (-D) gesetzt (z.B. "1.26.42").
   
   Falls wir aber OHNE Skript kompilieren (Fallback), bauen wir
   uns hier einen Standard-String zusammen.
*/
#ifndef VERSION
    #define VERSION             BASE_VERSION ".00-manual"
#endif

#ifndef BUILD_DATE
    #define BUILD_DATE          "unknown"
#endif

#ifndef BUILD_NUMBER
    #define BUILD_NUMBER        "0"
#endif
