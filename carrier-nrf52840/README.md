# Pico-Backscatter: carrier-nrf52840
Guide to configure nrf52840 to generate a unmodulated carrier in the range of 2400MHz to 2480MHz.

You will control the board via serial port.

## Access Serial Port

```
picocom -b 115200 /your/path/to/pico


*** Booting nRF Connect SDK v2.9.2-4ab7b98fc76f ***
*** Using Zephyr OS v3.7.99-aa34a5632971 ***

--- nRF52840 Carrier TX ---
Enter carrier frequency in MHz [2400-2480]:


```

<br> If you do not see the prompt press the reset button.


## Generating a Carrier Frequency
<br> Just type in your desired carrier frequency. The pico backscatter tag should print the expected value.
<br> To stop the carrier press the reset button.
