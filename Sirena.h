/**
******************************************************
* @file Pir.h
* @brief funciones para manejo del PIR
* @author Julian Salas Bartolome
* @version 1.0
* @date Enero 2024
*
*
*******************************************************/
#ifndef SIRENA_H
	#define SIRENA_H

		#include "Arduino.h"
		#include "Global.h"

        #include "IO.h"

		//---------------------------------------
		//Definiciones y declaracion de variables 
		//---------------------------------------
    	#define  nFlashOn 5		//Numereo de veces que la utilizacion se pone en On en el comando Flash
		#define  tFlashOn 2		//Tiempo On en 100 msg de la rafaga flash
		#define  tFlashOff 5  	//Tiempo Off en 100 msg de la rafaga flash 	 	

	
	    boolean lSirena = 1;	    				//Flag que habilita/Deshabilita la sirena
		boolean lSirenaLocal = 1;					//Flag que habilita la sirena para que pueda ser activada localmente ( por el pir )
		boolean lSirenaOn = 0;						//Flag que indica si la sirena está sonando
		boolean lOnTemporizado = 0;					//Flag que se pone a 1 cuando la sirena se pone a On con una temporizacion, se usa para temporizar

		int nTiempoOn;								//variable con segundos que estara la sirena en On
		int nTiempoFlashOn;							//variable con centenas de segundo del tiempo On en modo Flash
		int nTiempoFlashOff;						//variable con centenas de segundo del tiempo Off en modo Flash
		long nMilisegundosOn = 0;                	//Variable utilizada para temporizar el tiempo On temporizado
		int nFlash;									//Variable deonde se almacena el numero de On/Off en un flash 

		#define PosEnableSirenaCfg	2				//Bit que indica la posicion de EnableSirena en el byte de condiguracion
		#define PosEnableSirenaLocalCfg	3			//Bit que indica la posicion de EnableSirenaLocal en el byte de condiguracion


 		//------------------------
		//Declaracion de funciones 
		//------------------------
		void SirenaOn (void) ;
		void SirenaOff (void) ;
		String GetSirenaSuena (void) { return ( lSirenaOn ? "1" : "0" ); };		
		void SirenaLocalOn (void) ;
		void SirenaLocalOff (void) ;
		void LeeEstadoLocalSirena (void);
		String GetSirenaLocal (void) { return ( lSirenaLocal ? "1" : "0" ); };		
		void EnableSirena (void);
		void DisableSirena (void);
		String GetSirenaEnable (void) { return ( lSirena ? "1" : "0" ); };
		void LeeEstadoSirena (byte bConfiguracion);

		//------------------------
		//Codigo de las funciones 
		//------------------------

	    /**
	    ******************************************************
	    * @brief Hace sonar la sirena
	    *
	    * Pone a 1 el flag lSirenaOn y habilita su interrupción
	    */
	    void SirenaOn (void)
	    {
	    	lSirenaOn = 1;
			LogicaSirena ? digitalWrite(PinSirena, HIGH) : digitalWrite(PinSirena, LOW);	    
		}
    	/**
	    ******************************************************
	    * @brief Hace sonar la sirena
	    *
	    * Pone a 0 el flag lSirenaOn y deshabilita su interrupción
	    */
	    void SirenaOff (void)
	    {
	    	lSirenaOn = 0;
			LogicaSirena ? digitalWrite(PinSirena, LOW) : digitalWrite(PinSirena,HIGH);	    
		}
   		/**
	    ******************************************************
	    * @brief Habilita la sirena para que pueda ser usada localmente ( pir )
	    *
	    * Pone a 1 el flag lSirenaLocal 
	    */
	    void SirenaLocalOn (void)
	    {
	    	lSirenaLocal = 1;
			GrabaConfiguracionDispositivo ( PosEnableSirenaLocalCfg, lSirenaLocal);

		}
    	/**
	    ******************************************************
	    * @brief Deshabilita la sirena para ser usada localmente ( Pir )
	    *
	    * Pone a 0 el flag lSirenaLocal 
	    */
	    void SirenaLocalOff (void)
	    {
	    	lSirenaLocal = 0;
			GrabaConfiguracionDispositivo ( PosEnableSirenaLocalCfg, lSirenaLocal);
		}

		/**
		******************************************************
		* @brief Habilita la sirena
		*
		*/
		void EnableSirena (void)
		{
			lSirena = 1;
			GrabaConfiguracionDispositivo ( PosEnableSirenaCfg, lSirena);
		}	
		/**
		******************************************************
		* @brief Deshabilita la sirena
		*
		* Devuelve 1 si la baliza detecta keybt, 0 en caso contrario
		*/
		void DisableSirena (void)
		{
			lSirena = 0;
			GrabaConfiguracionDispositivo ( PosEnableSirenaCfg, lSirena);
		}					
		/**
		******************************************************
		* @brief lee el estado de EnableSirena alamacenado en servidor
		*
		*/
		void LeeEstadoSirena (byte bConfiguracion)
		{
			if ( bitRead (bConfiguracion, PosEnableSirenaCfg ))
			{
				lSirena = 1;
			}else{
				lSirena = 0;
			}
			if ( bitRead (bConfiguracion, PosEnableSirenaLocalCfg ))
			{
				lSirenaLocal = 1;
			}else{
				lSirenaLocal = 0;
			}

		}
#endif


