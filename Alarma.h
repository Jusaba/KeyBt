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
		boolean lDestello;
        #define PosEnableAlarma 6
		#define PosEnableDestello 7
		//------------------------
		//Declaracion de funciones 
		//------------------------
        
		void EnableAlarma (void);
        void DisableAlarma (void);
        String GetAlarma (void) { return ( lAlarma ? "1" : "0" ); };
		void EnableDestello (void);
        void DisableDestello (void);
        String GetDestello(void) { return ( lDestello ? "1" : "0" ); };


		/**
	    ******************************************************
	    * @brief Habilita la AlarmaS
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
	    * Pone a 0 el flag lAlarma 
	    */
	    void DisableAlarma (void)
	    {
	    	lAlarma = 0;
			GrabaConfiguracionDispositivo ( PosEnableAlarma, lAlarma);			
	    }    
		/**
	    ******************************************************
	    * @brief Habilita el destello
	    *
	    * Pone a 1 el flag lAlarma 
	    */
	    void EnableDestello (void)
	    {
	    	lDestello = 1;
			GrabaConfiguracionDispositivo ( PosEnableDestello, lDestello);			
		}
	    /**
	    ******************************************************
	    * @brief Deshabilita el destello
	    *
	    * Pone a 0 el flag lPir 
	    */
	    void DisableDestello (void)
	    {
	    	lDestello = 0;
			GrabaConfiguracionDispositivo ( PosEnableDestello, lDestello);			
	    }    

		/**
		******************************************************
		* @brief lee el estado de la configuracion de la alarma en servidor
		*
		*/
		void LeeEstadoAlarma (byte bConfiguracion)
		{
			if ( bitRead (bConfiguracion, PosEnableAlarma ))
			{
				lAlarma = 1;
			}else{
				lAlarma = 0;
			}
			if ( bitRead (bConfiguracion, PosEnableDestello ))
			{
				lDestello = 1;
			}else{
				lDestello = 0;
			}
		}
#endif