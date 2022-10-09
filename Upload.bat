echo %0
echo %~s0
echo %~d0%~p0
arduino-cli upload -p %1 -b esp32:esp32:esp32wrover -i %~d0%~p0build\esp32.esp32.esp32wrover\BLE.ino.bin