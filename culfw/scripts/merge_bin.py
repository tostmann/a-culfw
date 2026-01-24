Import("env")
import os
import shutil
import subprocess

def merge_bin(source, target, env):
    # 1. Namen aus dem Environment ziehen
    env_name = env['PIOENV'] # z.B. "XIAO_ESP32C3"
    variant = env.subst("$BOARD_MCU")
    build_dir = env.subst("$BUILD_DIR")
    
    # Pfade definieren
    app_offset = "0x10000"
    output_bin = os.path.join(build_dir, f"{env_name}-factory.bin")
    firmware_path = os.path.join(build_dir, f"{env.subst('$PROGNAME')}.bin")

    # 2. Flash-Parameter (mit Fallback)
    flash_freq = env.subst("$BOARD_FLASH_FREQ") or "40m"
    flash_mode = env.subst("$BOARD_FLASH_MODE") or "dio"

    print(f"--- Erzeuge Factory Image für {env_name} ({variant}) ---")

    # 3. Argumente für esptool
    cmd_parts = ["--flash_mode", flash_mode, "--flash_freq", flash_freq]

    # Bootloader, Partitions, etc.
    extra_images = env.get("FLASH_EXTRA_IMAGES", [])
    for image in extra_images:
        cmd_parts.extend([image[0], str(image[1])])

    # App
    cmd_parts.extend([app_offset, firmware_path])

    # 4. esptool Pfad & Ausführung
    platform = env.PioPlatform()
    esptool_script = os.path.join(platform.get_package_dir("tool-esptoolpy"), "esptool.py")
    
    cmd = [env.subst("$PYTHONEXE"), esptool_script, "--chip", variant, "merge_bin", "-o", output_bin] + cmd_parts

    try:
        subprocess.check_call(cmd, stdout=subprocess.DEVNULL)
    except subprocess.CalledProcessError:
        print(f"FEHLER: Factory Image Erstellung fehlgeschlagen.")
        return

    # 5. Kopieren in den Zielordner
    project_dir = env['PROJECT_DIR']
    dest_dir = os.path.join(project_dir, "Devices", "ESP32_Native")
    
    if os.path.isdir(dest_dir):
        target_file = os.path.join(dest_dir, f"{env_name}-factory.bin")
        shutil.copyfile(output_bin, target_file)
        print(f"Factory Image erstellt: {target_file}")

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", merge_bin)
