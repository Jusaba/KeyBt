/**
******************************************************
* @file Led.h
* @brief funciones para manejo del Luz
* @author Julian Salas Bartolome
* @version 1.0
* @date MArzo 2024
*
*
*******************************************************/
#ifndef Led_H
	#define Led_H

		#include "Arduino.h"

        #include "IO.h"

        void ApagaLed (void) {  digitalWrite(PinLed, LOW);	};
        void EnciendeLed (void)  {  digitalWrite(PinLed, HIGH);	};
        void FlashLed (void);

        void FlashLed (void)
        {
            EnciandeLed();
            delay (100);
            ApagaLed();
            delay(100);   
            EnciandeLed();
            delay (100);
            ApagaLed();
            delay(100);   
            EnciandeLed();
            delay (100);
            ApagaLed();
        }
#endif
