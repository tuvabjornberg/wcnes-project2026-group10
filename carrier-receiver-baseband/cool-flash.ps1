# Reboot into BOOTSEL mode
picotool reboot -u -f

Start-Sleep -Seconds 5

# Load the ELF file
picotool load build\carrier_receiver_baseband.elf

# Reboot normally
picotool reboot

Start-Sleep -Seconds 2

# Optional: connect to serial output
# Replace COM3 with your actual port
# picocom is usually not native on Windows, so use PuTTY instead:
putty -serial COM3 -sercfg 115200,8,n,1,N