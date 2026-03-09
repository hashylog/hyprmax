#!/usr/bin/env bash

set -euo pipefail

for command in hyprctl jq; do
    if ! command -v "$command" >/dev/null 2>&1; then
        printf 'hyprmax: missing dependency: %s\n' "$command" >&2
        exit 1
    fi
done

STATE_DIR="${XDG_RUNTIME_DIR:-/tmp}/hyprmax"
mkdir -p "$STATE_DIR"

active_window="$(hyprctl activewindow -j)"
window_addr="$(jq -r '.address // empty' <<<"$active_window")"

if [[ -z "$window_addr" || "$window_addr" == "0x0" ]]; then
    printf 'hyprmax: no active window\n' >&2
    exit 1
fi

state_file="$STATE_DIR/${window_addr#0x}.state"
is_fakemax="$(jq -r '.tags // [] | any(. == "hyprmax")' <<<"$active_window")"
is_floating="$(jq -r '.floating' <<<"$active_window")"

dispatch_tag() {
    local operation="$1"

    if [[ "$operation" == -* ]]; then
        hyprctl dispatch tagwindow -- "$operation" >/dev/null
    else
        hyprctl dispatch tagwindow "$operation" >/dev/null
    fi
}

if [[ "$is_fakemax" == "true" ]]; then
    if [[ -f "$state_file" ]]; then
        IFS=',' read -r saved_x saved_y saved_w saved_h < "$state_file"
        hyprctl --batch "dispatch resizeactive exact $saved_w $saved_h; dispatch moveactive exact $saved_x $saved_y" >/dev/null
        rm -f "$state_file"
    fi

    dispatch_tag '-hyprmax'
    exit 0
fi

if [[ "$is_floating" != "true" ]]; then
    hyprctl dispatch setfloating >/dev/null
    exit 0
fi

window_x="$(jq -r '.at[0]' <<<"$active_window")"
window_y="$(jq -r '.at[1]' <<<"$active_window")"
window_w="$(jq -r '.size[0]' <<<"$active_window")"
window_h="$(jq -r '.size[1]' <<<"$active_window")"
printf '%s,%s,%s,%s\n' "$window_x" "$window_y" "$window_w" "$window_h" > "$state_file"

focused_monitor="$(hyprctl monitors -j | jq -c '.[] | select(.focused == true)' | head -n 1)"
if [[ -z "$focused_monitor" ]]; then
    printf 'hyprmax: no focused monitor\n' >&2
    exit 1
fi

monitor_x="$(jq -r '.x' <<<"$focused_monitor")"
monitor_y="$(jq -r '.y' <<<"$focused_monitor")"
monitor_w="$(jq -r '.width' <<<"$focused_monitor")"
monitor_h="$(jq -r '.height' <<<"$focused_monitor")"
reserved_left="$(jq -r '.reserved[0]' <<<"$focused_monitor")"
reserved_top="$(jq -r '.reserved[1]' <<<"$focused_monitor")"
reserved_right="$(jq -r '.reserved[2]' <<<"$focused_monitor")"
reserved_bottom="$(jq -r '.reserved[3]' <<<"$focused_monitor")"

gaps_raw="$(hyprctl getoption general:gaps_out -j | jq -r '.custom // "0"')"
read -r -a gaps <<< "$gaps_raw"

case "${#gaps[@]}" in
    0)
        gap_top=0
        gap_right=0
        gap_bottom=0
        gap_left=0
        ;;
    1)
        gap_top="${gaps[0]}"
        gap_right="${gaps[0]}"
        gap_bottom="${gaps[0]}"
        gap_left="${gaps[0]}"
        ;;
    2)
        gap_top="${gaps[0]}"
        gap_right="${gaps[1]}"
        gap_bottom="${gaps[0]}"
        gap_left="${gaps[1]}"
        ;;
    3)
        gap_top="${gaps[0]}"
        gap_right="${gaps[1]}"
        gap_bottom="${gaps[2]}"
        gap_left="${gaps[1]}"
        ;;
    *)
        gap_top="${gaps[0]}"
        gap_right="${gaps[1]}"
        gap_bottom="${gaps[2]}"
        gap_left="${gaps[3]}"
        ;;
esac

target_x=$((monitor_x + reserved_left + gap_left))
target_y=$((monitor_y + reserved_top + gap_top))
target_w=$((monitor_w - reserved_left - reserved_right - gap_left - gap_right))
target_h=$((monitor_h - reserved_top - reserved_bottom - gap_top - gap_bottom))

if (( target_w <= 0 || target_h <= 0 )); then
    printf 'hyprmax: computed invalid target geometry\n' >&2
    exit 1
fi

hyprctl --batch "dispatch resizeactive exact $target_w $target_h; dispatch moveactive exact $target_x $target_y" >/dev/null
dispatch_tag '+hyprmax'
