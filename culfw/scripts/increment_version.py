import os
import re
import subprocess
import datetime

Import("env")

if env.GetOption("clean"):
    Return()

# Pfade
VERSION_FILE = os.path.join(env['PROJECT_DIR'], "version.h")
COUNTER_FILE = os.path.join(env['PROJECT_DIR'], ".build_number")


# ----------------------------------------------------------------------------
# Pre-build git snapshot (lokal, nur ausserhalb von CI).
#
# Vor jedem Build wird der dirty Working-Tree als Commit
# "build snapshot v<base>.<prev_build>" auf der aktuellen Branch festgehalten.
# So existiert fuer jeden gebauten Stand ein Rollback-Punkt im lokalen Repo.
# .build_number ist gitignored => der Counter-Bump landet nicht im Commit.
# ----------------------------------------------------------------------------

def _is_ci():
    return any(k in os.environ for k in ("GITHUB_RUN_NUMBER", "CI_PIPELINE_IID"))


def _peek_prev_build_number():
    """Aktueller Counter-Stand auf der Platte (= zuletzt gebauter Build)."""
    if not os.path.exists(COUNTER_FILE):
        return 0
    try:
        with open(COUNTER_FILE, 'r') as f:
            content = f.read().strip()
            if content.isdigit():
                return int(content)
    except Exception:
        pass
    return 0


def _git(args, project_dir, capture=False):
    try:
        return subprocess.run(
            ["git"] + args,
            cwd=project_dir,
            capture_output=capture,
            text=True,
            check=False,
        )
    except FileNotFoundError:
        return None  # git nicht installiert


def maybe_build_snapshot(base_ver, project_dir):
    if _is_ci():
        return
    # in einem git-Repo?
    r = _git(["rev-parse", "--git-dir"], project_dir, capture=True)
    if r is None or r.returncode != 0:
        return
    # dirty WT?
    r = _git(["status", "--porcelain"], project_dir, capture=True)
    if r is None or r.returncode != 0 or not r.stdout.strip():
        return
    prev_build = _peek_prev_build_number()
    prev_version = f"{base_ver}.{prev_build}"
    if _git(["add", "-A"], project_dir).returncode != 0:
        print(f"[increment_version] git add failed; skip snapshot")
        return
    cm = _git(
        ["commit", "-m", f"build snapshot v{prev_version}", "--no-verify"],
        project_dir, capture=True,
    )
    if cm.returncode != 0:
        # i.d.R. "nothing to commit" - nach add waren alle Aenderungen evtl. gitignored
        print(f"[increment_version] snapshot skipped: "
              f"{(cm.stdout or cm.stderr).strip()}")
        return
    print(f"\033[96m[increment_version] snapshot committed @ v{prev_version}\033[0m")

def get_base_version():
    """Liest BASE_VERSION aus version.h"""
    ver_str = "0.00"
    if os.path.exists(VERSION_FILE):
        with open(VERSION_FILE, 'r') as f:
            content = f.read()
            # Suche nach: #define BASE_VERSION "1.26"
            match = re.search(r'#define\s+BASE_VERSION\s+"(.*)"', content)
            if match:
                ver_str = match.group(1)
    return ver_str

def get_and_increment_build_number():
    # 1. GitHub Actions (NEU)
    if "GITHUB_RUN_NUMBER" in os.environ:
        return int(os.environ["GITHUB_RUN_NUMBER"])
        
    # 2. GitLab CI (Alt, kann bleiben)
    if "CI_PIPELINE_IID" in os.environ:
        return int(os.environ["CI_PIPELINE_IID"])

    # 3. Fallback: Lokal zählen (wie bisher)
    num = 1
    if os.path.exists(COUNTER_FILE):
        try:
            with open(COUNTER_FILE, 'r') as f:
                content = f.read().strip()
                if content.isdigit():
                    num = int(content) + 1
        except:
            pass

    # Speichern nur lokal sinnvoll
    with open(COUNTER_FILE, 'w') as f:
        f.write(str(num))
    return num

# --- MAIN ---

# 1. Daten sammeln
base_ver = get_base_version()

# 1a. Snapshot des dirty WT VOR dem Counter-Bump (lokal, nicht in CI).
#     Label = vorherige Version (= zuletzt gebauter Stand, jetzt mit Edits).
maybe_build_snapshot(base_ver, env['PROJECT_DIR'])

build_num = get_and_increment_build_number()
build_date = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

# 2. Finale Version bauen: "1.26" + "." + "42" -> "1.26.42"
full_version = f"{base_ver}.{build_num}"

# 3. Ausgabe für dich
print("\033[96m")
print("*******************************************")
print("* FIRMWARE VERSION UPDATE                 *")
print("*******************************************")
print(f"* Base Version: {base_ver}")
print(f"* Build Number: {build_num}")
print(f"* New Version:  {full_version}")
print(f"* Build Date:   {build_date}")
print("*******************************************")
print("\033[0m")

# 4. Injecting: Wir überschreiben das Makro VERSION für den Compiler
# Die Backslashes sind wichtig für C-Strings: \"1.26.42\"
env.Append(CPPDEFINES=[
    ("VERSION", f'\\"{full_version}\\"'),
    ("BUILD_NUMBER", f'\\"{build_num}\\"'),
    ("BUILD_DATE", f'\\"{build_date}\\"')
])
