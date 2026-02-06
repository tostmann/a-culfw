Import("env")
import os
import shutil
import subprocess

def merge_bin(source, target, env):
    # 1. Namen aus dem Environment ziehen
    env_name = env['PIOENV']
    variant = env.subst("$BOARD_MCU")
    build_dir = env.subst("$BUILD_DIR")
    
    # Pfade definieren
    app_offset = "0x10000"
    output_bin = os.path.join(build_dir, f"{env_name}-factory.bin")
    firmware_path = os.path.join(build_dir, f"{env.subst('$PROGNAME')}.bin")

    # 2. Flash-Parameter (mit Fallback)
    flash_freq = env.subst("$BOARD_FLASH_FREQ") or "40m"
    flash_mode = env.subst("$BOARD_FLASH_MODE") or "dio"
    # esptool merge_bin requires a fixed size or 'keep'
    flash_size = "4MB" 

    print(f"--- Erzeuge Factory Image für {env_name} ({variant}) ---")

    # 3. Argumente für esptool
    cmd_parts = ["--flash_mode", flash_mode, "--flash_freq", flash_freq, "--flash_size", flash_size]

    # Bootloader, Partitions, etc.
    extra_images = env.get("FLASH_EXTRA_IMAGES", [])
    for image in extra_images:
        cmd_parts.extend([str(image[0]), str(image[1])])

    # App
    cmd_parts.extend([app_offset, firmware_path])

    # 4. esptool Pfad & Ausführung
    platform = env.PioPlatform()
    esptool_path = os.path.join(platform.get_package_dir("tool-esptoolpy") or "", "esptool.py")
    
    if not os.path.exists(esptool_path):
        esptool_path = "esptool.py"

    cmd = [env.subst("$PYTHONEXE"), esptool_path, "--chip", variant, "merge_bin", "-o", output_bin] + cmd_parts

    print(f"Ausführung: {' '.join(cmd)}")

    try:
        subprocess.check_call(cmd)
        print(f"Factory Image erfolgreich erstellt: {output_bin}")
    except Exception as e:
        print(f"FEHLER bei Factory Image Erstellung: {e}")
        return

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", merge_bin)
