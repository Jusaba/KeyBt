/**
******************************************************
* @file Luz.h
* @brief funciones para manejo del Luz
* @author Julian Salas Bartolome
* @version 1.0
* @date Enero 2024
*
*
*******************************************************/
#ifndef Luz_H
	#define Luz_H

		#include "Arduino.h"

        #include "IO.h"
		#include "Alarma.h"

		//---------------------------------------
		//Definiciones y declaracion de variables 
		//---------------------------------------
       	boolean lLuz = 1;	    				    //Flag que habilita/Deshabilita el Luz
		boolean lLuzLocal = 1;						//Flag que indica si se encendera el Luz junto al sonar la sirena
		boolean lDestello;							//Flag que indica si hay destello de la luz

		unsigned long nMilisegundosDestello = 0;	//Variable donde se almacenan los msg para el destillo
		int nTiempoLuzOn;							//variable con segundos que estara la luz encendida
		long nMilisegundosLuzOn = 0;               	//Variable utilizada para temporizar el tiempo On temporizado de la luz
		boolean lOnLuzTemporizado = 0;				//Flag que se pone a 1 cuando la luz se pone a On con una temporizacion, se usa para temporizar
		#define TiempDestello 	 5000			    //Cadencia del destello
		#define TiempoDestelloFast 200
		#define PosEnableLuzCfg	4				    //Bit que indica la posicion de EnableLuz en el byte de condiguracion
		#define PosEnableLuzLocalcfg 5				//Bit que indica si el Luz está ligado a la sirena
		#define PosEnableDestello 7					//Bit que indica si esta habilitado el destello con la alarma en reposo

		unsigned long nTempoDestello = TiempDestello;

		//------------------------
		//Declaracion de funciones 
		//------------------------
        void EnableLuz (void);
        void DisableLuz (void);
        String GetLuz (void) { return ( lLuz ? "1" : "0" ); };
	    void EnciendeLuz();
	    void ApagaLuz();
	    void FlashLuz();
		void LuzLocalOn (void);
		void LuzLocalOff (void);
		String GetLuzLocal (void) { return ( lLuzLocal ? "1" : "0" ); };
		void LeeEstadoLuz (byte bConfiguracion);
		void EnableDestello (void);
        void DisableDestello (void);
        String GetDestello(void) { return ( lDestello ? "1" : "0" ); };
		void DestellaLuz (void);
		void Luz_Loop ();


		/**
	    ******************************************************
	    * @brief Habilita el Pir
	    *
	    * Pone a 1 el flag lPir y habilita su interrupción
	    */
	    void EnableLuz (void)
	    {
	    	lLuz = 1;
			GrabaConfiguracionDispositivo ( PosEnableLuzCfg, lLuz);			
		}
	    /**
	    ******************************************************
	    * @brief Deshabilita el Pir
	    *
	    * Pone a 0 el flag lPir y deshabilita su interrupción
	    */
	    void DisableLuz (void)
	    {
	    	lLuz = 0;
			GrabaConfiguracionDispositivo ( PosEnableLuzCfg, lLuz);			
	    }    
	    /**
	    ******************************************************
	    * @brief Enciende el Luz
	    *
	    */
	   
	    void EnciendeLuz (void)
	    {
	      	digitalWrite(PinLuz, HIGH);	
	    }
		
	    /**
	    ******************************************************
	    * @brief Apaga el Luz
	    *
	    */
	   
	    void ApagaLuz (void)
	    {
	      	digitalWrite(PinLuz, LOW);	
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
	    * @brief Hace un destello del Luz el Luz
	    *
	    */
	    void DestellaLuz (void)
	    {
			if (nTempoDestello == TiempDestello )	//Solo actualizamos ultimo valor si la destello es lento
			{
		    	cSalida = "LuzDestello";
    	    	EnviaValor (cSalida);             //Actualizamos ultimo valor
 	  	    	cSalida = String(' ');            //Limpiamos cSalida 
			}
			digitalWrite(PinLuz, HIGH);	
			delay(100);	
			digitalWrite(PinLuz, LOW);	
	    }
	    /**
	    ******************************************************
	    * @brief HAce flash el Luz
	    *
	    */
	    void FlashLuz (void)
	    {
		    cSalida = "LuzOn";
    	    EnviaValor (cSalida);             //Actualizamos ultimo valor
	      	digitalWrite(PinLuz, HIGH);
	      	delay(100);
	      	digitalWrite(PinLuz, LOW);
	      	delay(100);	
	      	digitalWrite(PinLuz, HIGH);
	      	delay(100);
	      	digitalWrite(PinLuz, LOW);
		  	delay(100);
	      	digitalWrite(PinLuz, HIGH);
	      	delay(100);
	      	digitalWrite(PinLuz, LOW);
	      	delay(100);	
	      	digitalWrite(PinLuz, HIGH);
	      	delay(100);
	      	digitalWrite(PinLuz, LOW);		
		    cSalida = "LuzOff";
    	    EnviaValor (cSalida);             //Actualizamos ultimo valor
	    }
   		/**
	    ******************************************************
	    * @brief Habilita el Luz para que funcione junto a la sirena
	    *
	    * Pone a 1 el flag lLuzLocal 
	    */
	    void LuzLocalOn (void)
	    {
	    	lLuzLocal = 1;
			GrabaConfiguracionDispositivo ( PosEnableLuzLocalcfg, lLuzLocal);

		}
    	/**
	    ******************************************************
	    * @brief Deshabilita el Luz para que funcione junto a la sirena
	    *
	    * Pone a 0 el flag lLuzLocal 
	    */
	    void LuzLocalOff (void)
	    {
	    	lLuzLocal = 0;
			GrabaConfiguracionDispositivo ( PosEnableLuzLocalcfg, lLuzLocal);
		}

		/**
		******************************************************
		* @brief lee el estado de la configuracion de la luz alamacenado en servidor
		*
		*/
		void LeeEstadoLuz (byte bConfiguracion)
		{
			if ( bitRead (bConfiguracion, PosEnableLuzCfg ))
			{
				lLuz = 1;
			}else{
				lLuz = 0;
			}
			if ( bitRead (bConfiguracion, PosEnableLuzLocalcfg ))
			{
				lLuzLocal = 1;
			}else{
				lLuzLocal = 0;
			}
			if ( bitRead (bConfiguracion, PosEnableDestello ))
			{
				lDestello = 1;
			}else{
				lDestello = 0;
			}			
		}

		void Luz_Loop (void)
		{

			if ( lOnLuzTemporizado )
			{
				if (TestTemporizacion( nMilisegundosLuzOn, nTiempoLuzOn * 1000))				//Comprobamos si ha transcurrido el tiempo de temporizacion fijado
				{																				//Y si ha transcurrido...
					ApagaLuz();
			    	cSalida = "LuzOff";						  									//Mandaremos el texto LuzOff como ultimo valor
					EnviaValor (cSalida);
					cSalida = String(' ');
					lOnLuzTemporizado = 0;														//Reseteamos el flag que indica que hay temporizacion					
				}
			}else{
				if (  lDestello )										   						//Si la alarma y el destello están habilitados	
				{
					if ( TestTemporizacion ( nMilisegundosDestello,  nTempoDestello ))     		//Si ha trancurrido el tiempo de Test de deteccion de los remotos
    		  		{
						DestellaLuz();	
						nMilisegundosDestello = millis();                                 		//Reseteamos el contador de tiempo de destello
					}	 
				}	
			}
		}

#endif