/**
******************************************************
* @file ServerPic.h
* @brief Documentacion de módulo CAM
* @author Julian Salas Bartolome
* @version 1.0
* @date Junio 2021
*
*
*******************************************************/

#ifndef SERVERPIC_H
	#define SERVERPIC_H

	//----------------------------------------------
	//Includes
	//----------------------------------------------
#define ESP32

		//Aruino & ESP32
		#include <WebServer.h>
		#include "esp_camera.h"
		#include <WiFi.h>
		#include "img_converters.h"
		#include "Arduino.h"
		#include "soc/soc.h"           
		#include "soc/rtc_cntl_reg.h"  
		#include "driver/rtc_io.h"
		#include <WebServer.h>
		//#include <StringArray.h>
		#include <SPIFFS.h>
		#include <FS.h>
		#include <WiFiUdp.h>
		#include <ArduinoOTA.h>
		#include <Wire.h>
		#include <base64.h>
		#include <libb64/cencode.h>		

		//Serverpic
		#include "Global.h"
		#include "Configuracion.h"
		#include "Eprom.h"
		#include "ModoAP.h"
		#include "ModoSTA.h"
		#include "Servidor.h"

		//BLE
		#include <BLEDevice.h>
		#include <BLEUtils.h>
		#include <BLEScan.h>
		#include <BLEAdvertisedDevice.h>
		#include <BLEEddystoneURL.h>
		#include <BLEEddystoneTLM.h>
		#include <BLEBeacon.h>

		#define Placa "ESP32 CAM"
		#define Modelo "ESP32"
		#define Ino "KeyBt"
		#define Tipo "Baliza"					

	

	//----------------------------
	//Declaracion de variables UNIVERSALES
	//----------------------------
	WebServer server(80);													//Definimos el objeto Servidor para utilizarlo en Modo AP

	WiFiClient Cliente;														//Definimos el objeto Cliente para utilizarlo en Servidor
	Telegrama oMensaje;									 					//Estructura mensaje donde se almacenaran los mensajes recibidos del servidor
 
	String cDispositivo = String(' ');										//Variable donde se deja el nombre del dsipositivo. Se utiliza para comunicar con HomeKit, llamar a listas ...
	String cSalida;															//Variable donde se deja el estado ( String ) para mandar en mensaje a ServerPic
	boolean lEstado = 0;													//Variable donde se deja el estado del dispositivo para reponer el estado en caso de reconexion
	boolean lConexionPerdida = 0;											//Flag de conexion perdida, se pone a 1 cuando se pierde la conexion. Se utiliza para saber si se ha perdido la conexion para restablecer estado anterior de la perdida

	boolean lHomeKit;													    //Flag para habililtar HomeKit ( 1 Habilitado, 0 deshabilitado )
	boolean lWebSocket ;  													//Flag para habililtar WebSocket ( 1 Habilitado, 0 deshabilitado )
	boolean lEstadisticas;													//Flag para habilitar Estadisticas ( 1 Habilitado, 0 Deshabilitado )
	boolean lPush ;														    //Flag donde para que indica si se usa Push o no ( 1/0 )
	String cPush = String(' ');												//Nombre del cliente push			

 
	unsigned long nMiliSegundosTest = 0;									//Variable utilizada para temporizar el Test de conexion del modulo a ServerPic


	#include "IO.h";

	//----------------------------
	//Declaracion de variables PARTICULARES
	//----------------------------
  	#define KeyMaster

	#define TiempoTestbtnOn  10000
	#define TiempoTestbtnOff  1000
	
	#ifdef KeyMaster
		#define TiempoTestbtnOnRemoto  15000
	#endif

	#ifdef KeyMaster
		#define CiclosNoDetect	3
	#endif
	
	#ifdef KeySlave
		#define CiclosNoDetect	1
	#endif	

	#ifdef KeySlave
		String cDispositivoRemoto = String(' '); 
	#endif
	
	#ifdef KeyMaster
		#define TiempoTestbtnOn  10000
		
		unsigned long nMiliSegundosTestRemoto = 0;
		unsigned long nTiempoTestRemoto = 0;
	#endif
	
	int scanTime = 2; //In seconds
	BLEScan *pBLEScan;
	boolean lBoton = 0;				//Flag que Indica si la rutina de Scan detecta boton
	boolean lBotonLocal = 0;
	boolean lBotonRemoto = 0;
	boolean lBotonAnterior=0;		//Flag que Indica el resultado del scaneo anterior
	boolean lBotonIn = 0;			//Flag que indica si el sistema registra boton. Se utiliza para controlar el cambio de estado de deteccion/no deteccion
	boolean lUltimoScan = 0;		//Flag que indica si en el ultimo escaneo se ha detectado key o no (1/0)
	boolean lEstadoLocal = 0;
	boolean lEstadoLocalAnterior = 0;
	unsigned long nMiliSegundosTestbtn = 0;
	unsigned long nTiempoTestbtn = 0;

	int nContador = 0;		//Contador de veces que no se detecta KeyBt para determinar cuando se considera que no hay KeyBt.
									//Esta variable se usa en el master para dar tiempo a lso exclavos a que refresquen el contador si un exclavo detecta KeyBt
	
	//----------------------------
	//Declaracion de funciones PARTICULARES
	//----------------------------

	
		

	//----------------------------------------------
	//DEBUG
	//----------------------------------------------
	#define Debug

	//----------------------------------------------
	//HOME KIT
	//----------------------------------------------
	#define HomeKit

	//----------------------------------------------
	//Teimpo de Test de conexion
	//----------------------------------------------
	#define TiempoTest	60000												//Tiempo en milisegundos para Test de conexion a servidor


	//----------------------------------------------
	//cARACTERISTICAS DISPOSITIVO
	//----------------------------------------------
	#define ESP32
	#define VIno "1.0"
	#define VBuild  "1"	
	//----------------------------------------------

	//---------------------------------
	//CARACTERISTICAS DE LA COMPILACION
	//---------------------------------
	#define Compiler "Arduino-cli";			//Compilador
	#define VCore "1.0.4";					//Versión del Core Arduino


	//---------------------------------
	//FUNCIONES PARTICULARES
	//---------------------------------

	#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))	
	
	/**
	******************************************************
	* @brief clase heredada de BLEAdvertisedDeviceCallbacks para personalizar la respuesta de un cliente bt
	*
	* Se lee la informacion que proviene del cliente bluetooth y se comprueba si es una trama com formato ibeacom
	* si es ese formato, comprobamos si el UUID contiene la cadena '3112-1991' para identificar uno de nuestros keys 
	* en caso de identificar la cedena pone el flag lBoton a 1 para indicar que se detecta boton 
	*
	* En el programa principal, despues de llamar a TestCambioEstado() se debe resetear este flag
	*/
	class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
	{
    	void onResult(BLEAdvertisedDevice advertisedDevice)
		{
		
			if (advertisedDevice.haveManufacturerData() == true)
			{

				std::string strManufacturerData = advertisedDevice.getManufacturerData();

				uint8_t cManufacturerData[100];
				strManufacturerData.copy((char *)cManufacturerData, strManufacturerData.length(), 0);

				//Comprobamos si es formato iBeacom
				if (strManufacturerData.length() == 25 && cManufacturerData[0] == 0x4C && cManufacturerData[1] == 0x00)
				{
					BLEBeacon oBeacon = BLEBeacon();
					//Leemos la trama iBeacom
					oBeacon.setData(strManufacturerData);
					//Serial.printf("ID: %04X Major: %d Minor: %d UUID: %s Power: %d\n", oBeacon.getManufacturerId(), ENDIAN_CHANGE_U16(oBeacon.getMajor()), ENDIAN_CHANGE_U16(oBeacon.getMinor()), oBeacon.getProximityUUID().toString().c_str(), oBeacon.getSignalPower());
					//Extraemos de la trama el UUID
					String cUUID = oBeacon.getProximityUUID().toString().c_str();
					//Si el UUID contiene el texto '3112-1991' se trata de un boton para detectar presencia
					if (cUUID.indexOf("3112-1991") > 0)
					{
						lBoton=1; //Ponemos a 1 el fla de detección de bluetooth de presencia
						Serial.println(cUUID);
					}else{
						Serial.println("******* No Boton ************");
						Serial.println(cUUID);
					}
				}
			}
        	return;
    	}

	};
	/**
	******************************************************
	* @brief Comprueba si esta baliaza tiene keybt detectado 
	*
	* Devuelve 1 si la baliza tiene detectado keybt, 0 en caso contrario
	* Es posible que la baliza no detecte key pero este en estado 1 por encontrarse en cilcos de escaneo antes de desconexion
	* o en caso de maestro, que puedas ser detectado por otra baliza
	*/
	boolean  GetEstado (void)
	{
		return ( lBotonLocal );
	}
	/**
	******************************************************
	* @brief Comprueba si esta baliaza tiene keybt detectado
	*
	* Devuelve 1 si la baliza detecta keybt, 0 en caso contrario
	*/
	boolean  GetKey (void)
	{
		return ( lUltimoScan );
	}	
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
	/**
	******************************************************
	* @brief Hace un Scan Bluetooth
	*
	* Actualia el flag global lBoton 
	*
	*/
	void Scan (void)
	{
	  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
      pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
	  lBotonLocal = lBoton;
	  Serial.println (lBotonLocal);
	}	
	/**
	******************************************************
	* @brief Actualiza el estado local de la baliza
	*
	* @param lEstadoBotonLocal.- Estado del boton resultado del escaneo
	*
	* Con la ultima lectura de la deteccion del boton se actualiza el estado local de la baliza con los siguientes criterios
	*	- Si se ha detectado boton, se pone el flag lEstadoActual a 1 y se fija un tiempo de escaneo largo
	*	- Si no se ha detectado boton, se hacen un total de CiclosNoDetect comprobaciones antes de considerar que 
	*     el estado local es 0. Cuando se completan las comprobaciones, se actualiza el flag lEstadoActual a 0
	*     y se fija un tiempo de escano corto para estar preparada para un enganche rapido
	*     Cada vez que se detecta boton, se resetea el contador de comprobaciones para asegurar que
	*     solo se considerara que es estado es o cuando se en todos los ciclos de comprobacion de forma continuada
	*     no se ha detectado boton
	*
	* Actualia el flag global lBoton 
	*
	*/	
	void ActualizaEstadoBalizaLocal (boolean lEstadoBotonLocal)
	{	
		
		if ( (lEstadoBotonLocal || lBotonRemoto) == 0 )			//Si no se detecta boton en local ni en los remotos
		{
			if ( nContador < CiclosNoDetect )					//Si no se ha llef¡gado al final de las cuentas de no deteccion
			{
				nContador ++;									//Incrementamos el contador de cuentas de no deteccion
			}else{												//Si llego al final, 
				lBotonRemoto = 0;								//Ponemos a o el flag lBotonRemoto para indicar que no hay boton remoto detectado
				lEstadoLocal = 0;								//Ponemos a o el flag lEstadoLocal para indicar que no hay boton local detectado
				nTiempoTestbtn = TiempoTestbtnOff;				//Ponemos tiempo de escaneo rapido		
				ApagaLed();										//Apagamos el led para indicar que no se detecta boton en el sistema
			}			
		}else{													//Si se detecta boton en local o en remoto
			lEstadoLocal = 1;
			nContador = 0;										//Reseteamos el contador de no deteccion
			nTiempoTestbtn = TiempoTestbtnOn;					//Fijamos un tiempo de escaneo lento
			if (lEstadoBotonLocal)								//Si hay boton local detectado
			{
				EnciendeLed();									//Encendemos el led
			}else{												//Si solo es el remoto
				FlashLed();										//Hacemos un flash
			}	
			#ifdef KeySlave
				cSalida = "mensaje-:-"+cDispositivoRemoto+"-:-KeyOn";
				MensajeServidor(cSalida);
				cSalida = String(' ');
			#endif
		}	
	}
	/**
	******************************************************
	* @brief Testea si se ha producido un cambio de estado local
	*
	* Llama a la funcion que actualiza el estado actual, compara el resuoltado con el estado anterior
	* si son distintos es que se ha producido un cambio de estado
	*
	* @return Devuelve 1 si se ha producido un cambio de estado, 0 en caso contrario 
	*
	*/
	boolean TestCambioEstadoLocal (boolean lEstadoBotonLocal)
	{
		

		boolean lCambioEstado = 0;
		ActualizaEstadoBalizaLocal(lEstadoBotonLocal);
		if ( lEstadoLocal != lEstadoLocalAnterior )
		{
			lCambioEstado = 1;
			lEstadoLocalAnterior = lEstadoLocal;
		}
		#ifdef KeySlave
			if ( lCambioEstado )
			{
				if ( lEstadoLocal )
				{
					cSalida = "mensaje-:-"+cDispositivoRemoto+"-:-KeyOn";
				}else{
					//cSalida = "mensaje-:-"+cDispositivoRemoto+"-:-KeyOff";
				}
				MensajeServidor(cSalida);
				cSalida = String(' ');
			}
		#endif
		#ifdef KeyMaster
			if (lCambioEstado)
			{
				if(lEstadoLocal)
				{
					//cSalida = "comando-:-exec-:-TorreEntrar";
					cSalida = "telegram-:-Julian-:-Desonexion alarma";
					//cSalida = "mensaje-:-sirena-:-Flash-:-1-:-1-:-1";
				}else{
					//cSalida = "comando-:-exec-:-TorreSalir";
					cSalida = "telegram-:-Julian-:-Conexion alarma";
					//cSalida = "mensaje-:-sirena-:-Flash-:-2-:-1-:-1";
				}
				MensajeServidor(cSalida);		
				cSalida = String(' ');                     

			}
		#endif

		lBoton = 0;										//Reseteamos el flag lBoton	
		return ( lCambioEstado );	
	}	
	
		/*

		if ( lBotonAnterior == 1)
		{
				cSalida = "telegram-:-Julian-:-Desconexion alarma";
		}else{
				cSalida = "telegram-:-Julian-:-Conexion alarma";
		}
		MensajeServidor(cSalida);		
		cSalida = String(' ');                     
		*/
	
	/**
	******************************************************
	* @brief Comprueba si ha habido cambio de estado en la deteccion y manda Telegram en caso de detectar cambio y ejecuta acciones en funcion del nuevo estado
	*
	* Devuelve 1 si ha habido cambio de estado, 0 en caso contrario
	* Si hay cambio de estado manda telegram para notificarlo
	* Si hay cambio de estado se ejecutan las instrucciones pertinentes al cuenvo cambio
	* 
	* En el programa principal, despues de llamar a esta funcion se debe resetear el flag lBoton 
	*
	*/
	/*
	boolean TestCambioEstado ( void )
	{
		boolean lSalida = 0;
		if (lBoton || lBotonRemoto)						//Si detecta Key en loacal o de algun remoto
		{
			if (lBoton)
			{			
				lUltimoScan = 1;											//Ponemos el flag lUltimoScan a 1		
				Serial.println("Detectado boton en baliza");
			}else{
				lUltimoScan = 0;											//Ponemos el flag lUltimoScan a 0		
			}
			if (lBotonRemoto)
			{
				Serial.println("Detectado boton en remoto");
			}
			if (!lBotonIn)
    		{
				cSalida = "telegram-:-Julian-:-Desconexion alarma";
	   			MensajeServidor(cSalida);		
	       		//cSalida = "mensaje-:-sirena-:-Flash-:-1-:-1-:-1";
				cSalida = String(' ');                    //Limpiamos cSalida para no actualizar valor   
				lSalida = 1;												//Ha habido cambio de estado, ponemos el flag a 1	
    		}			
			nContadorNoDetect = 0;											//Reseteamos contador de ciclos sin deteccion de KeyBt	
    		lBotonIn=1;														//Ponemos a 1 el Flag de deteccion del sistema
		}else{
			Serial.print ("Ciclos: ");
			Serial.print (nContadorNoDetect);
    		Serial.println(" Boton no detectado");
			if ( nContadorNoDetect < CiclosNoDetect )						//Si el contador de no detecciones no ha llegado al limita
			{
				nContadorNoDetect ++;										//Lo incrementamos
			}else{															//Si ha llegado al limite
    			if (lBotonIn)												//Si viene de un estado en el que estaba detectado key en el sistema
    			{
					cSalida = "telegram-:-Julian-:-Conexion alarma";		//Conectamos la alarma
       				//cSalida = "mensaje-:-sirena-:-Flash-:-2-:-1-:-1";
      				MensajeServidor(cSalida);	
					cSalida = String(' ');                    //Limpiamos cSalida para no actualizar valor   					
					lSalida = 1;											//Ha ha habido cambio de estado y lo señalizamos en flag
    			}    
				lBotonIn = 0;												//Marcamos  el estado del sistema como key no detectado
			}
			lUltimoScan = 0;    											//Señalizamos el ultimo scan con ausencia de deteccion
		}
		return (lSalida);
	}	
	*/
	/*
	boolean TestCambioEstado ( void )
	{
		boolean lSalida = 0;
		#ifdef KeyMaster
			if (lBoton || lBotonRemoto)
  		#endif
		#ifdef KeySlave
			if (lBoton)
  		#endif
		{
			if (lBoton)														//Si el resultado del  ultimo escaneo es 1
			{			
				lUltimoScan = 1;											//Ponemos el flag lUltimoScan a 1		
				Serial.println("Detectado boton en baliza");
			}
			if (lBotonRemoto)
			{
				Serial.println("Detectado boton en remoto");
			}
			if (!lBotonIn)
    		{
        		#ifdef KeyMaster
					cSalida = "telegram-:-Julian-:-Desconexion alarma";
	      			MensajeServidor(cSalida);		
				#endif
        		//cSalida = "mensaje-:-sirena-:-Flash-:-1-:-1-:-1";
				lSalida = 1;	
    		}
			#ifdef KeySlave
				cSalida = "mensaje-:-"+cDispositivoRemoto+"-:-KeyOn";
				MensajeServidor(cSalida);
			#endif
			nContadorNoDetect = 0;												//Reseteamos contador de ciclos sin deteccion de KeyBt	
    		lBotonIn=1;
  		}else{
			Serial.print ("Ciclos: ");
			Serial.print (nContadorNoDetect);
    		Serial.println(" Boton no detectado");
			if ( nContadorNoDetect < CiclosNoDetect )
			{
				nContadorNoDetect ++;
			}else{
    			if (lBotonIn)
    			{
					#ifdef KeyMaster
						cSalida = "telegram-:-Julian-:-Conexion alarma";
					#endif
       				//cSalida = "mensaje-:-sirena-:-Flash-:-2-:-1-:-1";
      				MensajeServidor(cSalida);	
					lSalida = 1;		
    			}    
				lBotonIn = 0;
			}
			lUltimoScan = 0;    		
  		}
		lSalida = lBotonIn;
		return lSalida;
	}
	*/

	//Variables donde se almacenan los datos definidos anteriormente para pasarlos a Serverpic.h
	//para mandar la información del Hardware y Software utilizados
	//En la libreria ServerPic.h estan definidos como datos externos y se utilizan en la funcion
	//Mensaje () para responder al comando generico #Info.
	//Con ese comando, el dispositivo mandara toda esta información al cliente que se lo pida
	// ESTOS DATOS NO SON PARA MODIFICAR POR USUARIO
	//----------------------------
	//Datos del programa principal
	//----------------------------
	String VMain = VIno;
	String Main = Ino; 
	String VModulo = VBuild;	
	//----------------------------
	//Datos de compilación
	//----------------------------	
	String Compilador = Compiler;
	String VersionCore = VCore;

	//----------------------------
	//Datos de Hardware
	//----------------------------	
	String ModeloESP = Modelo;
	String Board = Placa;
	String Tipe = Tipo;


#endif