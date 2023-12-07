
//Para ver detalles de la placa arduino-cli board details -b esp32:esp32:esp32c3
arduino-cli compile -b esp32:esp32:esp32c3:PartitionScheme=min_spiffs,FlashMode=qio,DebugLevel=none,UploadSpeed=115200,CDCOnBoot=cdc,JTAGAdapter=builtin  -e -v