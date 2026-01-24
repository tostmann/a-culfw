Import("env")
# Setzt den Namen der Firmware-Datei auf den Namen des Environments
env.Replace(PROGNAME=env['PIOENV'])
