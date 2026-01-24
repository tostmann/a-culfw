Import("env")
import shutil
import os

def after_build(source, target, env):
    env_name = env['PIOENV']
    prog_name = env.get("PROGNAME", "firmware")
    project_dir = env['PROJECT_DIR']
    
    # 1. Pfad zur erzeugten Quell-Hex-Datei
    build_dir = env['PROJECT_BUILD_DIR'] + "/" + env_name
    source_hex = os.path.join(build_dir, prog_name + ".hex")
    
    # 2. Zielverzeichnis dynamisch ermitteln (OHNE Hardcoding)
    dest_dir = ""
    
    # Wir durchsuchen die Include-Pfade (CPPPATH) des aktuellen Environments.
    # Da jedes Device "-I Devices/<Name>" in den Build-Flags hat,
    # finden wir so automatisch das richtige Verzeichnis.
    cpp_paths = env.get("CPPPATH", [])
    
    for path in cpp_paths:
        # Pfad absolut machen, um Fehler zu vermeiden
        if not os.path.isabs(path):
            path = os.path.abspath(os.path.join(project_dir, path))
            
        # Relativen Pfad zum Projektverzeichnis berechnen (z.B. "Devices/nanoCUL")
        try:
            rel_path = os.path.relpath(path, project_dir)
        except ValueError:
            continue # Falls Pfad auf anderem Laufwerk liegt
            
        # Wir zerlegen den Pfad (z.B. ['Devices', 'nanoCUL'])
        path_parts = rel_path.split(os.path.sep)
        
        # Wenn "Devices" im Pfad vorkommt, nehmen wir den Ordner direkt danach
        if "Devices" in path_parts:
            idx = path_parts.index("Devices")
            if idx + 1 < len(path_parts):
                # Das ist der Device-Ordner (z.B. "nanoCUL" oder "CUL")
                device_folder = path_parts[idx+1]
                dest_dir = os.path.join("Devices", device_folder)
                break 

    # Fallback: Falls kein Include-Pfad gefunden wurde (sollte nicht passieren)
    if not dest_dir:
        # Versuche Namens-Matching: Wenn Env "nanoCUL868" heißt und Ordner "Devices/nanoCUL" existiert
        # (Nimmt den Ordner, der als Start-String im Env-Namen vorkommt)
        devices_root = os.path.join(project_dir, "Devices")
        if os.path.exists(devices_root):
            for item in os.listdir(devices_root):
                if os.path.isdir(os.path.join(devices_root, item)) and env_name.startswith(item):
                    dest_dir = os.path.join("Devices", item)
                    break

    # 3. Kopieren
    if dest_dir:
        target_filename = env_name + ".hex"
        full_dest_path = os.path.join(project_dir, dest_dir, target_filename)
        
        if os.path.exists(source_hex):
            print(f"Kopiere Firmware nach: {full_dest_path}")
            shutil.copyfile(source_hex, full_dest_path)
    else:
        print(f"WARNUNG: Konnte Zielverzeichnis für {env_name} nicht automatisch bestimmen.")

# Hook direkt an die HEX-Datei hängen (startet erst, wenn objcopy fertig ist)
env.AddPostAction("$BUILD_DIR/${PROGNAME}.hex", after_build)
