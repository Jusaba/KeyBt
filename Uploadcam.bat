echo %0
echo %~s0
echo %~d0%~p0
arduino-cli upload -p %1 -b esp32:esp32:esp32cam -i KeyBt.ino.binCOM