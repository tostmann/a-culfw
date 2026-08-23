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

# --- build isolation -------------------------------------------------------
# Historically every device set OBJDIR = . while compiling sources from
# ../../clib and the other shared trees, so objects landed next to their source
# in a directory all devices share, and make reused them by timestamp: build
# device A then device B, and B silently linked A's objects, compiled against
# A's board.h.
#
# The 24 AVR devices no longer do this — they use OBJDIR = build/_/_, which puts
# every object under the device's own directory. The two dummy levels absorb the
# ../.. from the source paths, so the pattern rules stay unchanged.
#
# This target remains for the three ARM devices (CUBe, CUL-HM-CFG, MapleCUN),
# which still compile into the shared trees and therefore still need the sweep
# before a build.
.PHONY: clean-shared-objs
clean-shared-objs:
	@for d in ../../clib ../../lufa ../../at91lib ../../avr-uip ../../Wiznet ../../STM32; do \
		[ -d "$$d" ] && find "$$d" -name '*.o' -delete 2>/dev/null; \
	done; true
