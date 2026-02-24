#!/bin/bash
# Fake maximize for Hyprland floating WM

WINDOW=$(hyprctl activewindow -j)
ADDR=$(echo "$WINDOW" | jq -r '.address')
IS_FAKEMAX=$(echo "$WINDOW" | jq -r '.tags // [] | map(select(. == "fakemax")) | length > 0')
IS_FLOATING=$(echo "$WINDOW" | jq -r '.floating')

if [ "$IS_FAKEMAX" == "true" ]; then
    SAVED=$(cat /tmp/hypr_fakemax_$ADDR 2>/dev/null)
    if [ -n "$SAVED" ]; then
        X=$(echo "$SAVED" | cut -d',' -f1)
        Y=$(echo "$SAVED" | cut -d',' -f2)
        W=$(echo "$SAVED" | cut -d',' -f3)
        H=$(echo "$SAVED" | cut -d',' -f4)
        hyprctl --batch "dispatch resizeactive exact $W $H; dispatch moveactive exact $X $Y"
        rm -f /tmp/hypr_fakemax_$ADDR
    fi
    hyprctl dispatch tagwindow -- -fakemax

elif [ "$IS_FLOATING" == "false" ]; then
    # Tiled window - restore previous floating geometry if available
    SAVED_FLOAT=$(cat /tmp/hypr_float_$ADDR 2>/dev/null)
    hyprctl dispatch togglefloating
    if [ -n "$SAVED_FLOAT" ]; then
        X=$(echo "$SAVED_FLOAT" | cut -d',' -f1)
        Y=$(echo "$SAVED_FLOAT" | cut -d',' -f2)
        W=$(echo "$SAVED_FLOAT" | cut -d',' -f3)
        H=$(echo "$SAVED_FLOAT" | cut -d',' -f4)
        hyprctl --batch "dispatch resizeactive exact $W $H; dispatch moveactive exact $X $Y"
        rm -f /tmp/hypr_float_$ADDR
    fi

else
    # Floating window - save geometry and fake maximize
    X=$(echo "$WINDOW" | jq -r '.at[0]')
    Y=$(echo "$WINDOW" | jq -r '.at[1]')
    W=$(echo "$WINDOW" | jq -r '.size[0]')
    H=$(echo "$WINDOW" | jq -r '.size[1]')
    echo "$X,$Y,$W,$H" > /tmp/hypr_float_$ADDR
    echo "$X,$Y,$W,$H" > /tmp/hypr_fakemax_$ADDR

    MON=$(hyprctl monitors -j | jq -r '.[] | select(.focused == true)')
    MON_W=$(echo "$MON" | jq -r '.width')
    MON_H=$(echo "$MON" | jq -r '.height')
    MON_X=$(echo "$MON" | jq -r '.x')
    MON_Y=$(echo "$MON" | jq -r '.y')
    RESERVED_TOP=$(echo "$MON" | jq -r '.reserved[1]')

    GAPS_OUT=20
    NEW_X=$((MON_X + GAPS_OUT))
    NEW_Y=$((MON_Y + RESERVED_TOP + GAPS_OUT))
    NEW_W=$((MON_W - GAPS_OUT * 2))
    NEW_H=$((MON_H - RESERVED_TOP - GAPS_OUT * 2))

    hyprctl --batch "dispatch resizeactive exact $NEW_W $NEW_H; dispatch moveactive exact $NEW_X $NEW_Y"
    hyprctl dispatch tagwindow +fakemax
fi
