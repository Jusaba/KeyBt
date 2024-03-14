/**
******************************************************
* @file Alarma.h
* @brief funciones para de ordenes generalñes de la alarma
* @author Julian Salas Bartolome
* @version 1.0
* @date Marzo 2024
*
*
*******************************************************/

#ifndef ALARMA_H
	#define ALARMA_H

		#include "Arduino.h"

		//---------------------------------------
		//Definiciones y declaracion de variables 
		//---------------------------------------
       	boolean lAlarma ;	    				    //Flag que habilita/Deshabilita la alarma
        #define PosEnableAlarma 6

		//------------------------
		//Declaracion de funciones 
		//------------------------
        
		void EnableAlarma (void);
        void DisableAlarma (void);
        String GetAlarma (void) { return ( lAlarma ? "1" : "0" ); };


		/**
	    ******************************************************
	    * @brief Habilita la Alarma
	    *
	    * Pone a 1 el flag lAlarma 
	    */
	    void EnableAlarma (void)
	    {
	    	lAlarma = 1;
			GrabaConfiguracionDispositivo ( PosEnableAlarma, lAlarma);			
		}
	    /**
	    ******************************************************
	    * @brief Deshabilita la Alarma
	    *
	    * Pone a 0 el flag lPir 
	    */
	    void DisableAlarma (void)
	    {
	    	lAlarma = 0;
			GrabaConfiguracionDispositivo ( PosEnableAlarma, lAlarma);			
	    }    
#endif