# flash.ps1 - Flash RP2040 and open PuTTY

# --- Step 1: Force BOOTSEL mode ---
Write-Host "Rebooting Pico into BOOTSEL mode..."
& picotool.exe reboot -uf
Start-Sleep -Seconds 3

# --- Step 2: Wait until device is in BOOTSEL mode ---
Write-Host "Waiting for Pico to appear in BOOTSEL mode..."
$bootselFound = $false
for ($i=0; $i -lt 20; $i++) {
    Start-Sleep -Seconds 1
    $info = & picotool.exe info 2>&1
    if ($info -match "Program Information|Device") {
        $bootselFound = $true
        break
    }
}
if (-not $bootselFound) {
    Write-Host "Error: Pico not detected in BOOTSEL mode. Check USB cable and drivers."
    exit 1
}

# --- Step 3: Flash firmware ---
Write-Host "Flashing carrier_receiver_baseband.elf..."
& picotool.exe load build/carrier_receiver_baseband.elf

# --- Step 4: Reboot into normal mode ---
Write-Host "Rebooting into normal mode..."
& picotool.exe reboot
Start-Sleep -Seconds 3

# --- Step 5: Set COM port manually ---
$comPort = "COM5"
Write-Host "Pico detected on $comPort"

# --- Step 6: Launch PuTTY ---
Write-Host "Opening PuTTY..."
Start-Process putty.exe -ArgumentList "-serial $comPort -sercfg 115200,8,n,1,N"