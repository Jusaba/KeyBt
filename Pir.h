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

        #define TiempoNoPir	10000				//Tiempo entre alarmas de pir. Una vez disparada la alrama pir no se dispara otra hasta que no transucrre ese tiempo			

        boolean lPir = 1;	    				//Flag que habilita/Deshabilita el pir
        boolean lIntPir = 0;					//Flag que con 1 indica si ha habido interrupción de Pir 
        boolean lIntPirAnterior = 0;			//Flag que sirve para validar la interrupoción una vez en el bucle
        unsigned long nMilisegundosNoPir = 0;	//Variable que contiene el tiempo entre dispartos del pir
		#define PosEnablePirCfg	1				//Bit que indica la posicion de EnablePir en el byte de condiguracion


		//------------------------
		//Declaracion de funciones 
		//------------------------
        void EnablePir (void);
        void DisablePir (void);
        String GetPir (void) { return ( lPir ? "1" : "0" ); };
		void LeeEstadoPir (byte bConfiguracion );
        void ArmaPir (void);
        void DesarmaPir (void);

		void Pir_Loop (String cDispositivo);
		
        void IRAM_ATTR IntPir(void);

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
	    	detachInterrupt(digitalPinToInterrupt(PinPir));
			GrabaConfiguracionDispositivo ( PosEnablePirCfg, lPir);			
	    }    
		/**
		******************************************************
		* @brief configura estado de EnablePir en funcion del Byte configuracion
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