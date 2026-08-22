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
# Every device sets OBJDIR = . while its sources live in ../../clib and the
# other shared trees, so an object file ends up next to its source, in a
# directory all devices share. make then reuses it by timestamp: build device A
# and then device B, and B silently links A's objects, compiled against A's
# board.h. The symptom is a link that fails with "undefined reference" to a
# function the board.h of B does enable — or, worse, one that succeeds and
# produces a wrong image.
#
# Until the object directory is per device, drop the shared objects before a
# build. Costs a full recompile of the shared code each time; that is cheap
# next to a wrong binary.
.PHONY: clean-shared-objs
clean-shared-objs:
	@for d in ../../clib ../../lufa ../../at91lib ../../avr-uip ../../Wiznet ../../STM32; do \
		[ -d "$$d" ] && find "$$d" -name '*.o' -delete 2>/dev/null; \
	done; true
