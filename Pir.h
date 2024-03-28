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
#ifndef PIR_H
	#define PIR_H

		#include "Arduino.h"
		#include "Global.h"
		#include "Sirena.h"

        #include "IO.h"

		//---------------------------------------
		//Definiciones y declaracion de variables 
		//---------------------------------------

        #define TiempoNoPir	10000								//Tiempo entre alarmas de pir. Una vez disparada la alrama pir no se dispara otra hasta que no transucrre ese tiempo			
		#define TiempoSirenaxPir								//Tiempo que sonara la sirana si hay alarma de Pir ( La sirena tiene que estar habilitada y lSirenaLocal tien que estar a 1 )
        boolean lPir = 1;	    								//Flag que habilita/Deshabilita el pir
        boolean lIntPir = 0;									//Flag que con 1 indica si ha habido interrupción de Pir 
        boolean lIntPirAnterior = 0;							//Flag que sirve para validar la interrupoción una vez en el bucle
		boolean lSirenaLocal = 1;								//Flag que habilita la sirena para que pueda ser activada localmente ( por el pir )
        unsigned long nMilisegundosNoPir = 0;					//Variable que contiene el tiempo entre dispartos del pir
		#define PosEnablePirCfg	1								//Bit que indica la posicion de EnablePir en el byte de condiguracion
		#define PosEnableSirenaLocalCfg	3						//Bit que indica la posicion de EnableSirenaLocal en el byte de condiguracion


		//------------------------
		//Declaracion de funciones 
		//------------------------
        void EnablePir (void);									//Habilita el Pir
        void DisablePir (void);									//Deshabilita el Pir
        String GetPir (void) { return ( lPir ? "1" : "0" ); };	//Devuelve es estado Habilitado/deshabilitado del Pir (1/0)
		void LeeEstadoPir (byte bConfiguracion );				//Configura lPir y lSirenaLocal en funcion del bit PosEnablePirCfg y del bit PosEnableSirenaLocalCfg del Byte configuracion
        void ArmaPir (void);									//Habilita la interrupción del Pir
        void DesarmaPir (void);									//Deshabilita la interrupción del Pir
		void SirenaLocalOn (void) ;								//Habilita la sirena para dispararla por detección de Pir
		void SirenaLocalOff (void) ;							//Deshabilita la sirena para dispararla por detección de Pir
		String GetSirenaLocal (void) { return ( lSirenaLocal ? "1" : "0" ); };		

		void Pir_Loop (String cDispositivo);					//Función de atención del Pir utilizada en Lopp del programa principal
		
        void IRAM_ATTR IntPir(void);							//Rutina de atención de interrupción del Pir

		//------------------------
		//Codigo de las funciones 
		//------------------------

	    /**
	    ******************************************************
	    * @brief Habilita el Pir
	    *
	    * Pone a 1 el flag lPir y habilita su interrupción
	    */
	    void EnablePir (void)
	    {
	    	lPir = 1;
			if (LogicaPir)
			{
	    		attachInterrupt(digitalPinToInterrupt(PinPir), IntPir,  RISING);
			}else{
	    		attachInterrupt(digitalPinToInterrupt(PinPir), IntPir,  FALLING);
			}
			GrabaConfiguracionDispositivo ( PosEnablePirCfg, lPir);			
		}
	    /**
	    ******************************************************
	    * @brief Deshabilita el Pir
	    *
	    * Pone a 0 el flag lPir y deshabilita su interrupción
	    */
	    void DisablePir (void)
	    {
	    	lPir = 0;
			lSirenaLocal = 0;
	    	detachInterrupt(digitalPinToInterrupt(PinPir));
			ModificaConfiguracionDispositivo( PosEnableSirenaLocalCfg, lSirenaLocal);
			GrabaConfiguracionDispositivo ( PosEnablePirCfg, lPir);			
	    }    
		/**
		******************************************************
		* @brief configura estado de EnablePir y de lSirenaLocal en funcion del Byte configuracion
		*
		* @param.- bConfiguracion.- Byte con el contenido de Configuracion
		*/
		void LeeEstadoPir (byte bConfiguracion )
		{
			if ( bitRead (bConfiguracion, PosEnablePirCfg ))
			{
				lPir = 1;
			}else{
				lPir = 0;
			}
			if ( bitRead (bConfiguracion, PosEnableSirenaLocalCfg ))
			{
				lSirenaLocal = 1;
			}else{
				lSirenaLocal = 0;
			}

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
			ModificaConfiguracionDispositivo(PosEnableSirenaLocalCfg, lSirenaLocal);
			EnablePir();
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
			ModificaConfiguracionDispositivo( PosEnableSirenaLocalCfg, lSirenaLocal);
			EnablePir();
		}
		/**********************************
	    * @brief Arma el Pir
	    *
	    * aabilita la interrupción del pir
	    */
	    void ArmaPir (void)
	    {
			if (LogicaPir)
			{
	    		attachInterrupt(digitalPinToInterrupt(PinPir), IntPir,  RISING);
			}else{
	    		attachInterrupt(digitalPinToInterrupt(PinPir), IntPir,  FALLING);

			}	
	    }
	    /**
	    ******************************************************
	    * @brief Desarma el Pir
	    *
	    * Deshabilita la interrupción del pir
	    */
	    void DesarmaPir (void)
	    {
	    	detachInterrupt(digitalPinToInterrupt(PinPir));
	    }	
	    /**
	    ******************************************************
	    * @brief Rutina de interrupcion del PIR
	    *
	    * 
	    */
	    void IRAM_ATTR IntPir(void)
	    {
	    	DesarmaPir();
	    	lIntPir = 1;
	    	nMilisegundosNoPir = millis();
	    }	
    
	    /**
	    ******************************************************
	    * @brief Funcion de Atencion al Pir
		* 
		* Comprueba si el estado actual es 1 y es diferente al estado anterior, si es asi, Envia al servidor
		* el texto PirOn, modifica el estado anterior al actual ( para que no se vuelva a repetir la accion )
		* y manda ejecutar la lista de comandos <cDispositivo> + 'P' donde pueden grabarse las acciones a ejecutar
		* cuando hay alarma de Pir.
		* Si esta habilitada la sirena y ademas esta habilitada la sirena local, rersetea el contador nMilisegundosOn
		* envia al servidor el texto SirenaOn y pone en marcha la sirena empezando a soonar 
	    *
		* Por otro lado, comprueba si desde que se activo el PIr ha transcurrido un tiempo TeimpoNOPir
		* para resetear los flag del pir, enviar al servidor el texto PirOff y 'Armar' el Pir 
	    * Deshabilita la interrupción del pir
		* 
		* @param cDispositivo.- Nombre del dispositivo que utilizará para ejecutar la lista de comandos que le toca a este dispositivo
	    */
		void Pir_Loop (String cDispositivo)
		{
			
			if ( lIntPir == 1 &&  !lIntPirAnterior )
    		{
    	  		#ifdef Debug
    	    		Serial.println ("Alarma pir");
    	  		#endif  
    	  		lIntPirAnterior = 1;
    	  		cSalida = "PirOn";
    	  		EnviaValor (cSalida);             //Actualizamos ultimo valor
    	  		cSalida = String(' ');            //Limpiamos cSalida 
    	  		cSalida = "comando-:-exec-:-"+cDispositivo + String("P");
   				MensajeServidor(cSalida);
    	  		if (lSirena && lSirenaLocal)
    	  		{
				  nTiempoOn =  5;
				  lOnTemporizado = 1;		
				  nMilisegundosOn = millis();		
    	    	  cSalida = "SirenaOn";
    	    	  EnviaValor (cSalida);             //Actualizamos ultimo valor
    	    	  cSalida = String(' ');            //Limpiamos cSalida 
    	    	  SirenaOn();
    	  		}
	
    		}
    		if ( lIntPir == 1 && (TestTemporizacion( nMilisegundosNoPir, TiempoNoPir ) ) )
    		{
    	    	#ifdef Debug                                                
    	    	    Serial.println("Fin emporización del pir");
    	    	#endif    
    	  		lIntPirAnterior = 0;
    	  		lIntPir = 0;
    	  		cSalida = "PirOff";
    	  		EnviaValor (cSalida);             //Actualizamos ultimo valor
    	  		cSalida = String(' ');            //Limpiamos cSalida 
    	  		ArmaPir();
    		}
		}
#endif