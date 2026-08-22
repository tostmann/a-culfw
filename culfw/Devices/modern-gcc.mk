# Flags the historic culfw sources need to build with a current avr-gcc.
# Included by every device makefile; PlatformIO sets the same two flags in
# culfw/platformio.ini.
#
# -fcommon
#   GCC 10 made -fno-common the default. culfw declares variables such as
#   output_flush_func (ttydata.h) and packetCheckValues (rf_receive_bucket.h)
#   as tentative definitions in headers, so every translation unit that
#   includes them emits its own. Without -fcommon the link fails with
#   "multiple definition of ...".
#
# -Wno-error=implicit-function-declaration
#   GCC 14 turned this long-standing warning into an error. A handful of files
#   (pcf8833.c calling fromhex, ...) rely on an implicit declaration. Keeping
#   it a warning is the stopgap; the fix is the missing #include, one file at
#   a time.
CFLAGS += -fcommon
CFLAGS += -Wno-error=implicit-function-declaration
