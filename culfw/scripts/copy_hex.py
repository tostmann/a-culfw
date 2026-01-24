Import("env")
import shutil
import os

env_name = env['PIOENV']
project_dir = env['PROJECT_DIR']
prog_name = env.get("PROGNAME", "firmware")

def find_destination_dir():
    cpp_paths = env.get("CPPPATH", [])
    for path in cpp_paths:
        if not os.path.isabs(path):
            path = os.path.abspath(os.path.join(project_dir, path))
        try:
            rel_path = os.path.relpath(path, project_dir)
            path_parts = rel_path.split(os.path.sep)
            if "Devices" in path_parts:
                idx = path_parts.index("Devices")
                if idx + 1 < len(path_parts):
                    return os.path.join("Devices", path_parts[idx+1])
        except ValueError:
            continue

    # Fallback Suche
    devices_root = os.path.join(project_dir, "Devices")
    if os.path.exists(devices_root):
        for item in os.listdir(devices_root):
            if os.path.isdir(os.path.join(devices_root, item)):
                if env_name.startswith(item):
                    return os.path.join("Devices", item)
    return None

dest_dir = find_destination_dir()

if dest_dir:
    # Wir prüfen auf .hex (AVR) UND .bin (STM32)
    extensions = [".hex", ".bin"]
    
    for ext in extensions:
        target_filename = env_name + ext
        full_dest_path = os.path.join(project_dir, dest_dir, target_filename)

        if env.GetOption("clean"):
            if os.path.exists(full_dest_path):
                try:
                    os.remove(full_dest_path)
                    print(f"Removed custom output: {full_dest_path}")
                except OSError:
                    pass
        else:
            # Build Hook
            def make_copy_func(dest, extension):
                def copy_action(source, target, env):
                    build_dir = env['PROJECT_BUILD_DIR'] + "/" + env_name
                    source_file = os.path.join(build_dir, prog_name + extension)
                    
                    if os.path.exists(source_file):
                        print(f"Kopiere Firmware nach: {dest}")
                        os.makedirs(os.path.dirname(dest), exist_ok=True)
                        shutil.copyfile(source_file, dest)
                return copy_action

            # Registriere Hook für den jeweiligen Dateityp
            env.AddPostAction(f"$BUILD_DIR/${{PROGNAME}}{ext}", make_copy_func(full_dest_path, ext))

else:
    if not env.GetOption("clean"):
        print(f"WARNUNG: Konnte Zielverzeichnis für {env_name} nicht bestimmen.")
