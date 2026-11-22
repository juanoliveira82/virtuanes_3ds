#!/usr/bin/env bash
#
# Runs inside the linuxserver/azahar container; started by run.sh.
#
#   container.sh gui|headless TIMEOUT "AZAHAR ARGS" APP [STEP...]
#
# APP is a .3dsx, or a .cia, which gets installed to the emulated SD card
# first and then booted from there like the Home Menu would.
#
set -uo pipefail

MODE="$1"
TIMEOUT="$2"
AZAHAR_ARGS="$3"
APP="$4"
shift 4

install_cia() {
    local titles="/config/.local/share/azahar-emu/sdmc/Nintendo 3DS"
    if ! azahar -i "$APP" >/tmp/azahar_install.txt 2>&1; then
        echo "Installing $(basename "$APP") failed:" >&2
        cat /tmp/azahar_install.txt >&2
        exit 1
    fi
    # The most recently installed application's content.
    APP=$(find "$titles" -path '*/title/00040000/*/content/*.app' -printf '%T@ %p\n' \
        | sort -n | tail -1 | cut -d' ' -f2-)
    echo "installed: ${APP#"$titles"/*/*/}"
}

if [ "$MODE" = gui ]; then
    case "$APP" in *.cia) install_cia ;; esac
    # shellcheck disable=SC2086
    exec azahar $AZAHAR_ARGS "$APP"
fi

# Azahar's default keyboard mapping for the 3DS buttons.
declare -A KEYS=(
    [a]=a [b]=s [x]=z [y]=x [l]=q [r]=w [start]=m [select]=n
    [up]=t [down]=g [left]=f [right]=h
)

FB=/tmp/fb
SDMC=/config/.local/share/azahar-emu/sdmc
LOG=/config/.local/share/azahar-emu/log/azahar_log.txt
mkdir -p "$FB"

# With single window mode off, Azahar shows the 3DS screens in their own
# 400x480 window (top screen above bottom screen, at 1x), which is what
# gets screenshotted.
CONF=/config/.config/azahar-emu/qt-config.ini
sed -i '/^singleWindowMode=/d; /^singleWindowMode\\default=/d; s/^\[UI\]$/[UI]\nsingleWindowMode=false/' "$CONF"

Xvfb :99 -screen 0 1280x720x24 -fbdir "$FB" -nolisten tcp >/dev/null 2>&1 &
export DISPLAY=:99 QT_QPA_PLATFORM=xcb
for _ in $(seq 50); do
    [ -e /tmp/.X11-unix/X99 ] && break
    sleep 0.1
done

case "$APP" in *.cia) install_cia ;; esac

# shellcheck disable=SC2086
azahar $AZAHAR_ARGS "$APP" >/tmp/azahar_stdout.txt 2>&1 &
AZ=$!

# Tells what the emulator was doing when a run failed or timed out, and
# saves the files that the Azahar log doesn't have. It runs before the
# emulator is stopped, so that its windows and processes can still be seen.
#
# The text goes to the error output (the log of the CI step). The files go
# to /out: failure_screen.png is the whole virtual display, and sdmc/ has
# the text and settings files that the app wrote to the emulated SD card
# (for example virtuanes_trace.txt from a `make TRACE=1` build).
diagnose() {
    echo "--- diagnostics (exit status $1) ---" >&2
    if kill -0 "$AZ" 2>/dev/null; then
        echo "Azahar is still running." >&2
    else
        echo "Azahar is not running any more." >&2
    fi

    echo "Windows on the virtual display:" >&2
    for w in $(xdotool search --name '' 2>/dev/null); do
        echo "  $w name '$(xdotool getwindowname "$w" 2>/dev/null)' class '$(xdotool getwindowclassname "$w" 2>/dev/null)'" >&2
    done

    echo "Last lines of Azahar's own output:" >&2
    tail -n 40 /tmp/azahar_stdout.txt >&2 2>/dev/null

    echo "The emulated SD card:" >&2
    (cd "$SDMC" 2>/dev/null && find . -maxdepth 4 -not -path './Nintendo 3DS/*/*/*/*' | head -n 60) >&2

    mkdir -p /out/sdmc
    find "$SDMC" -maxdepth 4 -not -path "$SDMC/Nintendo 3DS/*" \( -name '*.txt' -o -name '*.cfg' \) \
        -exec cp -t /out/sdmc {} + 2>/dev/null
    xwdtopnm "$FB/Xvfb_screen0" 2>/dev/null | pnmtopng -force >/out/failure_screen.png 2>/dev/null
    echo "--- end of diagnostics ---" >&2
}

finish() {
    local status=$?
    if [ "$status" -ne 0 ]; then
        diagnose "$status"
    fi
    kill "$AZ" 2>/dev/null
    for _ in $(seq 50); do
        kill -0 "$AZ" 2>/dev/null || break
        sleep 0.1
    done
    kill -9 "$AZ" 2>/dev/null
    cp "$LOG" /out/azahar_log.txt 2>/dev/null
    echo "log: azahar_log.txt"
}
trap finish EXIT
# This script is PID 1 in the container, which ignores an untrapped SIGTERM.
trap 'exit 124' TERM INT

( sleep "$TIMEOUT"; echo "timeout after ${TIMEOUT}s"; kill $$ ) &
WATCHDOG=$!

# Top-level Azahar windows: the main window is titled "Azahar <version>",
# the render window "Azahar". The render window only appears once the app
# has booted, which can take a while with software OpenGL, or until a
# debugger attaches with -g.
azahar_windows() {
    xdotool search --maxdepth 1 --onlyvisible --class Azahar 2>/dev/null
}
WIN=""
MAIN=""
for _ in $(seq $((TIMEOUT * 5))); do
    for w in $(azahar_windows); do
        if [ "$(xdotool getwindowname "$w")" = Azahar ]; then WIN=$w; else MAIN=$w; fi
    done
    [ -n "$WIN" ] && break
    sleep 0.2
done
if [ -z "$WIN" ]; then
    echo "Azahar did not open its render window. Windows:" >&2
    for w in $(xdotool search --name ''); do
        echo "  $w '$(xdotool getwindowname "$w")'" >&2
    done
    cat /tmp/azahar_stdout.txt >&2
    exit 1
fi
[ -n "$MAIN" ] && xdotool windowmove "$MAIN" 420 0
xdotool windowmove "$WIN" 0 0 windowsize "$WIN" 400 480 windowfocus --sync "$WIN" 2>/dev/null

shot() {
    if xwdtopnm "$FB/Xvfb_screen0" 2>/dev/null | pamcut 0 0 400 480 | pnmtopng -force >"/out/$1.png" 2>/dev/null; then
        echo "shot: $1.png"
    else
        echo "shot: $1.png failed" >&2
    fi
}

for step in "$@"; do
    IFS=: read -r cmd arg ms <<<"$step"
    if [[ "$cmd" =~ ^(key|down|up)$ ]]; then
        k="${KEYS[$arg]:-}"
        if [ -z "$k" ]; then
            echo "unknown button: $arg" >&2
            continue
        fi
    fi
    case "$cmd" in
        wait)
            # In the background, so that the timeout can interrupt it.
            sleep "$arg" &
            wait $! ;;
        key)
            xdotool windowfocus "$WIN" keydown "$k"
            sleep "$(awk "BEGIN { print ${ms:-150} / 1000 }")"
            xdotool keyup "$k" ;;
        down)
            xdotool windowfocus "$WIN" keydown "$k" ;;
        up)
            xdotool keyup "$k" ;;
        tap)
            # Touch the screen at (arg, ms) in the 400x480 window; the
            # bottom screen is at y 240-479, x 40-359.
            xdotool mousemove --window "$WIN" "$arg" "$ms" mousedown 1
            sleep 0.15
            xdotool mouseup 1 ;;
        shot)
            shot "$arg" ;;
        *)
            echo "unknown step: $step" >&2 ;;
    esac
    if ! kill -0 "$AZ" 2>/dev/null; then
        echo "Azahar exited" >&2
        break
    fi
done

# No steps: just run (e.g. while a debugger is attached) until the
# emulator exits or the timeout hits.
if [ $# -eq 0 ]; then
    wait "$AZ"
fi
kill "$WATCHDOG" 2>/dev/null
