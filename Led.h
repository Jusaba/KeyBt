/**
******************************************************
* @file Led.h
* @brief funciones para manejo del led
* @author Julian Salas Bartolome
* @version 1.0
* @date Enero 2024
*
*
*******************************************************/
#ifndef LED_H
	#define LED_H

		#include "Arduino.h"

        #include "IO.h"

		//------------------------
		//Declaracion de funciones 
		//------------------------
	    void EnciendeLed();
	    void ApagaLed();
	    void FlashLed();

	    /**
	    ******************************************************
	    * @brief Enciende el led
	    *
	    */
	    void EnciendeLed (void)
	    {
	      digitalWrite(PinLed, HIGH);	
	    }
	    /**
	    ******************************************************
	    * @brief Apaga el led
	    *
	    */
	    void ApagaLed (void)
	    {
	      digitalWrite(PinLed, LOW);	
	    }
	    /**
	    ******************************************************
	    * @brief Apaga el led
	    *
	    */
	    void FlashLed (void)
	    {
	      EnciendeLed();
	      delay(100);
	      ApagaLed();
	      delay(100);	
	      EnciendeLed();
	      delay(100);
	      ApagaLed();
	    }



#endif