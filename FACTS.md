*PLATFORMIO*

is in $HOME/.platformio/penv/bin

call "pio" only from top of Project directory!!
pio run -d culfw/

BEFOLGE STRICT: dass Du keine Blockierenden Kommandos senden darfst um unseren Dialog flüssig fortzusetzen, füge im Zweifel immer "timeout" voran zur Prüfung:
timeout 3s pio monitor
timeout 3s cat /dev/ttyACM0


all binaries go into binaries/ - maintain the manifest inside this directory
