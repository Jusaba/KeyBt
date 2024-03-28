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
		#include "Alarma.h"
		#ifdef Luz
			#include "Luz.h"
		#endif	


		//---------------------------------------
		//Definiciones y declaracion de variables 
		//---------------------------------------
    	#define  nFlashOn 5		//Numereo de veces que la utilizacion se pone en On en el comando Flash
		#define  tFlashOn 2		//Tiempo On en 100 msg de la rafaga flash
		#define  tFlashOff 5  	//Tiempo Off en 100 msg de la rafaga flash 	 	

	
	    boolean lSirena = 1;	    				//Flag que habilita/Deshabilita la sirena
		boolean lSirenaOn = 0;						//Flag que indica si la sirena está sonando
		boolean lOnTemporizado = 0;					//Flag que se pone a 1 cuando la sirena se pone a On con una temporizacion, se usa para temporizar
		boolean lDestelloSirena = 0;				//Variable donde se almacenara temporalmente el flag lDestello de la Alarma

		int nTiempoOn;								//variable con segundos que estara la sirena en On
		int nTiempoFlashOn;							//variable con centenas de segundo del tiempo On en modo Flash
		int nTiempoFlashOff;						//variable con centenas de segundo del tiempo Off en modo Flash
		long nMilisegundosOn = 0;                	//Variable utilizada para temporizar el tiempo On temporizado
		int nFlash;									//Variable deonde se almacena el numero de On/Off en un flash 

		#define PosEnableSirenaCfg	2				//Bit que indica la posicion de EnableSirena en el byte de condiguracion


 		//------------------------
		//Declaracion de funciones 
		//------------------------
		void SirenaOn (void) ;
		void SirenaOff (void) ;
		void SirenaFlash ( int nCiclos, int nTempoOn, int nTempoOff );
		String GetSirenaSuena (void) { return ( lSirenaOn ? "1" : "0" ); };		
		void EnableSirena (void);
		void DisableSirena (void);
		String GetSirenaEnable (void) { return ( lSirena ? "1" : "0" ); };
		void LeeEstadoSirena (byte bConfiguracion);

		void Sirena_Loop (void);

		//------------------------
		//Codigo de las funciones 
		//------------------------

	    /**
	    ******************************************************
	    * @brief Hace sonar la sirena
	    *
	    * Pone a 1 el flag lSirenaOn y pone en marcha la sirena
	    */
	    void SirenaOn (void)
	    {
	    	lSirenaOn = 1;																	//Flag lSirena a 1
			#ifdef Luz
				if ( lLuzLocal )																//Si esta habilitada la luz con sirena
				{
					nTempoDestello = TiempoDestelloFast;										//Destello rápido	
					lDestelloSirena = lDestello;
					lDestello = 1;															    //Habilitar destello
				}	
			#endif					
			LogicaSirena ? digitalWrite(PinSirena, HIGH) : digitalWrite(PinSirena, LOW);	    
		}
    	/**
	    ******************************************************
	    * @brief Hace sonar la sirena
	    *
	    * Pone a 0 el flag lSirenaOn apaga la sirena
	    */
	    void SirenaOff (void)
	    {
	    	lSirenaOn = 0;									//Flag lSirena a 0
			#ifdef Luz
				if ( lLuzLocal )																//Si esta habilitada la luz con sirena
				{
					nTempoDestello = TiempDestello;
					lDestello = lDestelloSirena;												//Deshabilitar el destello
				}
			#endif	
			LogicaSirena ? digitalWrite(PinSirena, LOW) : digitalWrite(PinSirena,HIGH);	    
		}
   		/**
	    ******************************************************
	    * @brief Hace sonar la sirena en modo flash
	    *
	    * Pone a 0 el flag lSirenaOn apaga la sirena
		* 
		* @param nCiclos.- Numero de periodos de flasheo
		* @param nTempoOn.- Tiempo On de cada ciclo
		* @param nTempoOff.- Tienmpo Off de cada ciclo
		*
	    */
			
		void SirenaFlash ( int nCiclos, int nTempoOn, int nTempoOff )
		{
			while ( nCiclos > 0 )
			{
				LogicaSirena ? digitalWrite(PinSirena, HIGH) : digitalWrite(PinSirena, LOW);	    
				#ifdef Luz
					if ( lLuzLocal )																//Si esta habilitada la luz con sirena
					{
						EnciendeLuz();
					}	
				#endif
				delay(nTempoOn*100);
				LogicaSirena ? digitalWrite(PinSirena, LOW) : digitalWrite(PinSirena,HIGH);	    
				#ifdef Luz
					if ( lLuzLocal )																//Si esta habilitada la luz con sirena
					{
						ApagaLuz();
					}
				#endif
				delay(nTempoOff*100);	
				nCiclos--;
			}
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
			#ifdef Luz
				lLuzLocal = 0;
				ModificaConfiguracionDispositivo( PosEnableLuzLocalcfg, lLuzLocal );
			#endif	
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
		}
		/**
		******************************************************
		* @brief Comprueba si ha finalizado la temporizacion de la Sirena On
		*
		*/
		void Sirena_Loop (void)
		{
			if ( lOnTemporizado )											//Si la sirena esta sonando y tiene que temporizar
			{
      			if (TestTemporizacion( nMilisegundosOn, nTiempoOn * 1000))	//Comprobamos si ha transcurrido el tiempo de temporizacion fijado
				{															//Y si ha transcurrido...
					SirenaOff();											//Apagamos la sirena
					#ifdef Luz
			    		if ( lLuzLocal )																//Si esta habilitada la luz con sirena
			    		{
							nTempoDestello = TiempDestello;
							ApagaLuz();																//Deshabilitar la luz
				    		cSalida = "OffSirenaLuz";						  	//Mandaremos el texto OnSirenaLuz como ultimo valor

			    		}else{
				    		cSalida = "SirenaOff";								//Mandamos el texto SirenaOn como ultimo valor
			    		}	
					#else
						cSalida = "SirenaOff";								//Mandamos el texto SirenaOn como ultimo valor						
					#endif				
					EnviaValor (cSalida);
					cSalida = String(' ');
					lOnTemporizado = 0;										//Reseteamos el flag que indica que hay temporizacion
				}
			} 
		}
#endif


