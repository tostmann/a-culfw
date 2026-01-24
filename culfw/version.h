#define VERSION_1               1
#define VERSION_2               26

/* Das ist unsere Basis-Version (statisch) */
#define BASE_VERSION            "1.26"
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
