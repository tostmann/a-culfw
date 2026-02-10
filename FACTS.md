
*ZIEL*
Portierung und Testung von culfw auf ESP32 

*PLATFORMIO*

is in $HOME/.platformio/penv/bin

call "pio" only from top of Project directory!!
pio run -d culfw/

BEFOLGE STRICT: dass Du keine Blockierenden Kommandos senden darfst um unseren Dialog flüssig fortzusetzen, füge im Zweifel immer "timeout" voran zur Prüfung:
timeout 3s pio monitor
timeout 3s cat /dev/ttyACM0


all binaries go into binaries/ - maintain the manifest inside this directory

*HARDWARE SETUP*

root@Argus:/opt/ai_builder_data/users/763684262/projects/CULFW32# ls -l /dev/serial/by-id/
total 0
lrwxrwxrwx 1 root root 13 Feb 10 13:39 usb-busware.de_CUL868-if00 -> ../../ttyACM1
lrwxrwxrwx 1 root root 13 Feb 10 12:23 usb-Espressif_USB_JTAG_serial_debug_unit_54:32:04:89:48:D0-if00 -> ../../ttyACM0

usb-busware.de_CUL868-if00 - Referenz CUL auf ATMEGA32u4 Basis mit culfw
usb-Espressif_USB_JTAG_serial_debug_unit_54:32:04:89:48:D0-if00 - Zielhardware XIAO ESP32-C3
