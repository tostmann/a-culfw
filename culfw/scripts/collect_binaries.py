Import("env")
import os
import shutil
import json
from datetime import datetime

def collect_binaries(source, target, env):
    # Pfade: Eine Ebene über culfw/ -> Git Root
    project_dir = env['PROJECT_DIR']
    git_root = os.path.dirname(project_dir)
    bin_dir = os.path.join(git_root, "binaries")
    manifest_path = os.path.join(bin_dir, "manifest.json")

    if not os.path.exists(bin_dir):
        os.makedirs(bin_dir)

    # 1. Version aus den dynamischen CPPDEFINES extrahieren
    full_version = "unknown"
    cpp_defines = env.get("CPPDEFINES", [])
    
    # PlatformIO speichert Defines oft als Liste von Tupeln oder Strings
    for define in cpp_defines:
        # Check für Tupel-Format: ("VERSION", '\"1.26.100\"')
        if isinstance(define, tuple) and define[0] == "VERSION":
            full_version = str(define[1]).replace('\\"', '').replace('"', '')
        # Check für String-Format: VERSION=\"1.26.100\"
        elif isinstance(define, str) and "VERSION=" in define:
            full_version = define.split("=")[1].replace('\\"', '').replace('"', '')

    env_name = env['PIOENV']
    board = env.subst("$BOARD")
    mcu = env.subst("$BOARD_MCU")
    
    # 2. Quelldateien bestimmen
    build_dir = env.subst("$BUILD_DIR")
    progname = env.subst("$PROGNAME")
    artifacts = []

    # AVR nutzt .hex, ESP32 nutzt .bin
    is_avr = env.subst("$PIOPLATFORM") == "atmelavr"
    suffix = ".hex" if is_avr else ".bin"

    main_file = os.path.join(build_dir, progname + suffix)
    if os.path.exists(main_file):
        dest_name = f"{env_name}{suffix}"
        shutil.copyfile(main_file, os.path.join(bin_dir, dest_name))
        artifacts.append({
            "file": dest_name, 
            "type": "app" if is_avr else "ota"
        })

    # Spezialfall ESP32 Factory Image
    if not is_avr:
        factory = os.path.join(build_dir, f"{env_name}-factory.bin")
        if os.path.exists(factory):
            shutil.copyfile(factory, os.path.join(bin_dir, f"{env_name}-factory.bin"))
            artifacts.append({"file": f"{env_name}-factory.bin", "type": "factory"})

    # 3. Manifest aktualisieren
    manifest = {}
    if os.path.exists(manifest_path):
        try:
            with open(manifest_path, 'r') as f:
                manifest = json.load(f)
        except:
            pass

    manifest[env_name] = {
        "platform": env.subst("$PIOPLATFORM"),
        "board": board,
        "mcu": mcu,
        "version": full_version,
        "last_build": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        "artifacts": artifacts
    }

    with open(manifest_path, 'w') as f:
        json.dump(manifest, f, indent=4)

    print(f"\033[92m[DONE] Artifacts collected: {bin_dir}/{env_name}{suffix} (v{full_version})\033[0m")

# Hooks registrieren
env.AddPostAction("$BUILD_DIR/${PROGNAME}.hex", collect_binaries)
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", collect_binaries)
