#!/bin/bash
# flash.sh - Flash RP2040 and open serial monitor

set -e

# --- Step 1: Force BOOTSEL mode ---
echo "Rebooting Pico into BOOTSEL mode..."
picotool reboot -uf || true
sleep 3

# --- Step 2: Wait until device is in BOOTSEL mode ---
echo "Waiting for Pico to appear in BOOTSEL mode..."
bootsel_found=false
for i in $(seq 1 20); do
    sleep 1
    if picotool info 2>&1 | grep -qE "Program Information|Device"; then
        bootsel_found=true
        break
    fi
done

if [ "$bootsel_found" = false ]; then
    echo "Error: Pico not detected in BOOTSEL mode. Check USB cable."
    exit 1
fi

# --- Step 3: Flash firmware ---
echo "Flashing carrier_receiver_baseband.elf..."
picotool load build/carrier_receiver_baseband.elf

# --- Step 4: Reboot into normal mode ---
echo "Rebooting into normal mode..."
picotool reboot
sleep 3