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

							

	
	//------------------
	//Hardware Utilizado 
	//------------------	
	#define MiniD1_1M

	#ifdef MiniD1_0M
		#define KeyMaster
		#define MiniD1_0
		#define Placa "MiniD1_0"
		#define Modelo "MiniD1"
		#define Ino "KeyBtM"					
		#define ESP32
		#define Tipo "Baliza"
		//-----------------
		//TiEmpo de rebotes
		//-----------------
 		#define TempoRebotes 150
	#endif

	#ifdef MiniD1_0E
		#define KeySlave
		#define MiniD1_0
		#define Placa "MiniD1_0"
		#define Modelo "MiniD1"
		#define Ino "KeyBtE"					
		#define ESP32
		#define Tipo "Baliza"
		//-----------------
		//TiEmpo de rebotes
		//-----------------
 		#define TempoRebotes 150
	#endif

	#ifdef MiniD1_1M
		#define KeyMaster
		#define MiniD1_1
		#define Placa "MiniD1_1"
		#define Modelo "MiniD1"
		#define Ino "KeyBtM"					
		#define ESP32
		#define Tipo "Baliza"
		#define Pir
		//-----------------
		//TiEmpo de rebotes
		//-----------------
 		#define TempoRebotes 150
	#endif

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
	//Definiciones y declaracion de variables PARTICULARES
	//----------------------------
  	//

	//-----------------
	//Keybt
	//-----------------
	#define TiempoTestbtnOn  10000						//Tiempo de scaneo cuando se ha detectado boton
	#define TiempoTestbtnOff  1000						//Tiempo de scaneo cuando no hay boton de tectado
	#define TiempoTestbtnOnRemoto  30000 				//En Master Tiempo de espera de deteccion de remotos
 														//En Eslave, tiempo referencia para envio de estado al Master
	#ifdef KeyMaster
		unsigned long nMiliSegundosTestRemoto = 0;		//variable para manejar tiempos de comprobacion de deteccion de boton por parte de remotos
		#define CiclosNoDetect	3						//Ciclos de no deteccion del master para considerar que no hay presencia de boton
	#endif

	#ifdef KeySlave
		unsigned long nMiliSegundosInfoRemoto = 0;		//variable para manejar tiempos de envio mensajes de estado al maestro
		#define CiclosNoDetect	1						//Ciclos de no deteccion del slave para considerar que no hay presencia de boton
		String cDispositivoMaestro = String(' '); 		//Variable donde se almacena el nombre del controlador Master
	#endif	

	boolean lKeyBt = 1;	    				//Flag que habilita/Deshabilita la deteccion de llave bluetooth
	int scanTime = 2; 						//Periodo de scaneo
	BLEScan *pBLEScan;						//Objeto para el scaneo BLE
	boolean lBoton = 0;						//Flag que Indica si la rutina de Scan detecta boton valido en local
	boolean lBotonLocal = 0;				//Flag que indica que la baliza ha detectado presencia de boton local ( tiene el contenido de lBoton )
	boolean lBotonRemoto = 0;				//Flag que se pone a 1 cuando el master recibe mensaje 'KeyOn' de un remoto. Se resetea cada TiempoTestbtnOnRemoto sg 
	boolean lEstadoLocal = 0;				//Flag que indica el estado actual del sistema ( 1 deteccion de presencia )
	boolean lEstadoLocalAnterior = 0;		//Flag que indica el estado anterior del sistema  ( para detectar cambio )
	unsigned long nMiliSegundosTestbtn = 0;	//Variable utilizada para contabilizar tiempo de comprobacion de presencia
	unsigned long nTiempoTestbtn = 0;		//Variable que contiene el tiemp de scaneo ( varia entre TiempoTestbtnOn y TiempoTestbtnOff correspondientes a scaneo lento y rapido)

	int nContador = 0;						//Contador de veces que no se detecta KeyBt para determinar cuando se considera que no hay KeyBt.
											//Esta variable se usa en el master para dar tiempo a lso exclavos a que refresquen el contador si un exclavo detecta KeyBt

	String cKey;

	//-----------------
	//Pir
	//-----------------
	#define TiempoNoPir	10000				//Tiempo entre alarmas de pir. Una vez disparada la alrama pir no se dispara otra hasta que no transucrre ese tiempo			

	boolean lPir = 1;	    				//Flag que habilita/Deshabilita el pir
	boolean lIntPir = 0;					//Flag que con 1 indica si ha habido interrupción de Pir 
	boolean lIntPirAnterior = 0;			//Flag que sirve para validar la interrupoción una vez en el bucle
	unsigned long nMilisegundosNoPir = 0;	//Variable que contiene el tiempo entre dispartos del pir
	
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
	#define VIno "1.0"
	#define VBuild  "1"	
	//----------------------------------------------

	//---------------------------------
	//CARACTERISTICAS DE LA COMPILACION
	//---------------------------------
	#define Compiler "Arduino-cli";			//Compilador
	#define VCore "1.0.4";					//Versión del Core Arduino


	//----------------------------
	//Declaracion de funciones PARTICULARES
	//----------------------------

	//-----------------
	//Keybt
	//-----------------
	#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))	
	void Scan (void);
	boolean TestIbeacomRegistrado (BLEAdvertisedDevice advertisedDevice);
	void ActualizaEstadoBalizaLocal (boolean lEstadoBotonLocal);
	String InvierteUUID (String cUUID);

	boolean KeyBtGetEstado (void);
	boolean KeyBtGetKeyLocal();
	boolean KeyBtGetKeyRemoto();

	String GetCodeKey ();
	void SetCodeKey (String cCodeKey);	

	void EnableKeyBt (void) { lKeyBt = 1; };
	void DisableKeyBt (void) { lKeyBt = 0; };
	String GetKeyBt (void) { return ( lKeyBt ? "1" : "0" ); };

	//*Funciones para Debug
	void printcUUID (String cUUID, BLEBeacon oBeacon, BLEAdvertisedDevice advertisedDevice);
	void printScan (void);

	//-----------------
	//Pir
	//-----------------
	void EnablePir (void);
	void DisablePir (void);
	String GetPir (void) { return ( lPir ? "1" : "0" ); };
	void ArmaPir (void);
	void DesarmaPir (void);
	void IRAM_ATTR IntPir(void);

	

	void EnciendeLed();
	void ApagaLed();
	void FlashLed();



	/**
	******************************************************
	* @brief clase heredada de BLEAdvertisedDeviceCallbacks para personalizar la respuesta de un cliente bt
	*
	* Se lee la informacion que proviene del cliente bluetooth y se comprueba si es un boton registrado llamando a TestIbeacomRegistrado()
	* En caso de que TestIbeacomRegistrado() identifique un boton valido, TestIbeacomRegistrado() pone flag lBoton a 1 para indicar que se detecta boton 
	* En el programa principal, despues de llamar a TestCambioEstado() se debe resetear este flag
	*/
	class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
	{
    	void onResult(BLEAdvertisedDevice advertisedDevice)
		{
			if (advertisedDevice.haveManufacturerData() == true)			//Si hay datos de fabricante
			{
				TestIbeacomRegistrado ( advertisedDevice );					//Analizamos si es un boton registrado
			}			
        	return;
    	}

	};


	/**
	******************************************************
	* @brief Funcion que valida los botones registrados
	*
	* Si se detecta un boton valido poen a 1 el flag lBoton ( Ojo, leer descripcion de BLEAdvertisedDeviceCallbacks para ver como reponer flag)
	* Para la comprobacion de si es boton valido lo primero que ira es que la trama sea de un dispositivo iBeacom
	* y leugo mira si la trama lleva el codigo cKey
	*
	* @param advertisedDevice.- Objeto BLEAdvertisedDevice recibido en BLEAdvertisedDeviceCallbacks
	* 
	* La UUID Adoptada para reconocimiento es
	*	05501230380491191231010060190824 que se traduce a  24081960-0001-3112-1991-043830125005
	*   05501230380491191231010091191231
	*   05501230380491191231010091191231
	*
	* @return Devuelve 1 si se es boton registrado, 0 en caso contrario
	*/	
	boolean TestIbeacomRegistrado (BLEAdvertisedDevice advertisedDevice)
	{
		boolean lSalida = 0;
		std::string strManufacturerData = advertisedDevice.getManufacturerData();
		uint8_t cManufacturerData[100];
		strManufacturerData.copy((char *)cManufacturerData, strManufacturerData.length(), 0);
		//Si es ibeacom ( Apple )
		if (strManufacturerData.length() == 25 && cManufacturerData[0] == 0x4C && cManufacturerData[1] == 0x00)	
		{		
			BLEBeacon oBeacon = BLEBeacon();
			//Leemos la trama iBeacom
			oBeacon.setData(strManufacturerData);
			//Extraemos de la trama el UUID
			String cUUID = oBeacon.getProximityUUID().toString().c_str();
			cUUID = InvierteUUID(cUUID);	//Invertimos cUUID	

			//Si el UUID contiene el texto cKey se trata de un boton para detectar presencia
			if (cUUID.indexOf(cKey) > 0)
			{
				lBoton=1;					//Ponemos a 1 el flag de detección boton
				lSalida = 1;				//Ponemos el flag de salida a 1 para indicar que se ha detectado boton
				#ifdef Debug
					printcUUID	(cUUID, oBeacon, advertisedDevice)	;		
				#endif				
			}

			//------------------------
			//Boton HelytIot averiados
			//------------------------			
			if(ENDIAN_CHANGE_U16(oBeacon.getMajor()) == 256 && ENDIAN_CHANGE_U16(oBeacon.getMinor()) == 25 )
			{
				lBoton=1; 			//Ponemos a 1 el flag de detección boton
				lSalida = 1;		//Ponemos el flag de salida a 1 para indicar que se ha detectado boton
				#ifdef Debug
					printcUUID	(cUUID, oBeacon, advertisedDevice)	;		
				#endif
			}
			
			
			if(ENDIAN_CHANGE_U16(oBeacon.getMajor()) == 1011 && ENDIAN_CHANGE_U16(oBeacon.getMinor()) == 19641)  
			{
				lBoton=1; 			//Ponemos a 1 el flag de detección boton
				lSalida = 1;		//Ponemos el flag de salida a 1 para indicar que se ha detectado boton
				#ifdef Debug
					printcUUID	(cUUID, oBeacon, advertisedDevice)	;		
				#endif
			}
			
		}	
		return (lSalida);
	}
	/**
	******************************************************
	* @brief Invierte una cadena UUID
	*
	* @param cUUID.- Cadena UUID a invertir 
	*
	* @return Devuelve la cadena UUID invertida
	*
	*/
	String InvierteUUID (String cUUID)
	{
		int nLenUUID = cUUID.length();
		String cUUIDInvert = String(' ');
		int nPos = nLenUUID - 2;
		while ( nPos > -1 )
		{
			if ( cUUID.substring(nPos + 1, nPos + 2) == String('-'))
			{ 
				cUUIDInvert = cUUIDInvert + "-";
				nPos = nPos -1 ;
			}else{
				cUUIDInvert = cUUIDInvert + cUUID.substring(nPos, nPos + 2);
				nPos = nPos - 2;	
			}	
		}
	    return (cUUIDInvert.substring(1));				//Quitamos el espacio inicial
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
	  #ifdef Debug
		printScan();
	  #endif

	}	
	/**
	******************************************************
	* @brief Actualiza el estado local de la baliza (el Flag lBotonRemoto y lEstadoLocal )
	*
	* @param lEstadoBotonLocal.- Estado del boton resultado del escaneo ( en programa es lBoton )
	*
	* Con la ultima lectura de la deteccion del boton se actualiza el estado local de la baliza con los siguientes criterios
	*	- Si se ha detectado boton, se pone el flag lEstadoLocal a 1 y se fija un tiempo de escaneo largo
	*	- Si no se ha detectado boton, se hacen un total de CiclosNoDetect comprobaciones antes de considerar que 
	*     el estado local es 0. Cuando se completan las comprobaciones, se actualiza el flag lEstadoLocal a 0
	*     y se fija un tiempo de escano corto para estar preparada para un enganche rapido
	*     Cada vez que se detecta boton, se resetea el contador de comprobaciones para asegurar que
	*     solo se considerara que es estado es 0 cuando se en todos los ciclos de comprobacion de forma continuada
	*     no se ha detectado boton
	*
	* 
	*
	*/	
	void ActualizaEstadoBalizaLocal (boolean lEstadoBotonLocal)
	{	
		
		if ( (lEstadoBotonLocal || lBotonRemoto) == 0 )			//Si no se detecta boton en local ni en los remotos
		{	
																//en caso de esclavos, lBotonRemoto siempre sera 0
			#ifdef Debug	
				Serial.print ("Contador: ");
				Serial.println (nContador);
			#endif		
			if ( nContador < CiclosNoDetect )					//Si no se ha llef¡gado al final de las cuentas de no deteccion
			{
				nContador ++;									//Incrementamos el contador de cuentas de no deteccion
			}else{												//Si llego al final, 
				lBotonRemoto = 0;								//Ponemos a 0 el flag lBotonRemoto para indicar que no hay boton remoto detectado
				lEstadoLocal = 0;								//Ponemos a 0 el flag lEstadoLocal para indicar que no hay boton local ni remoto detectado
				nTiempoTestbtn = TiempoTestbtnOff;				//Ponemos tiempo de escaneo rapido		
				ApagaLed();										//Apagamos el led para indicar que no se detecta boton en el sistema
			}			
		}else{													//Si se detecta boton en local o en remoto
			lEstadoLocal = 1;									//Ponemos a 1 el flag lEstadoLocal para indicar que hay boton local o remoto detectado, es decir, ha boton detectado en el sistema
			nContador = 0;										//Reseteamos el contador de no deteccion
			nTiempoTestbtn = TiempoTestbtnOn;					//Fijamos un tiempo de escaneo lento
			if (lEstadoBotonLocal)								//Si hay boton local detectado
			{
				EnciendeLed();									//Encendemos el led
			}else{												//Si solo es el remoto
				FlashLed();										//Hacemos un flash
			}	
		}	
	}
	/**
	******************************************************
	* @brief Testea si se ha producido un cambio de estado local
	*
	* Llama a la funcion que actualiza el estado actual, compara el resultado con el estado anterior
	* si son distintos es que se ha producido un cambio de estado
	*
	* @return Devuelve 1 si se ha producido un cambio de estado, 0 en caso contrario 
	*
	*/
	boolean TestCambioEstadoLocal (boolean lEstadoBotonLocal)
	{
		

		boolean lCambioEstado = 0;
		ActualizaEstadoBalizaLocal(lEstadoBotonLocal);					//Determinamos el flag lEstadoLocal que indica en Master si hay deteccion de boton en master o remoto y en el remoto si hy deteccion de boton por parte de ese remoto
		if ( lEstadoLocal != lEstadoLocalAnterior )						//Si ha habido cambio de estado
		{
			lCambioEstado = 1;												//Lo señalizamos con su flag
			lEstadoLocalAnterior = lEstadoLocal;							//Actualizamos el estado anterior
		}
		#ifdef KeySlave														//Los escalvos deben informar al maestro en cada scaneo por que si se hiciera en el cambio de estado, un no deteccion mandaria off e innhibiria al resto
			if (lCambioEstado)												//En el esclavo, si ha habido cambio de estado
			{
				if ( lEstadoLocal )											//Si se ha detectado boton el este remoto
				{
					cSalida = "mensaje-:-"+cDispositivoMaestro+"-:-KeyOn";	//Mandamos mensaje On a master 
				}else{														//Si no se ha detectado borton en este remoto
					cSalida = "mensaje-:-"+cDispositivoMaestro+"-:-KeyOff";	//Mandamos mensaje Off a master
				}
				MensajeServidor(cSalida);				
				cSalida = String(' ');
			}	
		#endif
		#ifdef KeyMaster
			if (lCambioEstado)											//En el maestro, si ha habido cambio de estado
			{
				if(lEstadoLocal)										//Si el nuevo estado es deteccion 
				{
					//cSalida = "comando-:-exec-:-TorreEntrar";			//Desconectamos alarma
cSalida = "comando-:-exec-:-"+cDispositivo+"I";
					//cSalida = "mensaje-:-sirena-:-Flash-:-1-:-1-:-1";	//Avisamos con sirena
				}else{
cSalida = "comando-:-exec-:-"+cDispositivo+"O";
					//cSalida = "comando-:-exec-:-TorreSalir";			//Conectamos alarma
					//cSalida = "mensaje-:-sirena-:-Flash-:-2-:-1-:-1"; //Avisamos con sirena
				}
				MensajeServidor(cSalida);		
				cSalida = String(' ');                     

			}
		#endif

		lBoton = 0;										//Reseteamos el flag lBoton	
		return ( lCambioEstado );	
	}	

	/**
	******************************************************
	* @brief Funcion que Graba en Eprom el codigo de llave autorizado
	*
	* @param cCodeKey.- Codigo de la llave a grabar
	*/
	void SetCodeKey (String cCodeKey)
	{
		#ifdef Debug
			Serial.print ("SetCodeKey()->Data Key: ");
			Serial.println ( cCodeKey ); 	
		#endif	
    	EEPROM.put(nPosicionDataUsuario, cCodeKey+'\0'); //Guardo el codigo de la llave
    	EEPROM.commit();
	}
	/**
	******************************************************
	* @brief Funcion que recupera el codigo autorizado de llave desde Eprom
	*
	* @return Codigo de la llave autorizada
	*/
	String GetCodeKey (void)
	{
		String cCodeKey; 
		EEPROM.get(nPosicionDataUsuario, cCodeKey); //Guardo la config
		#ifdef Debug
			Serial.print ("GetCodeKey()->Posicion EEPROM: ");
			Serial.print (nPosicionDataUsuario);
			Serial.print (" Data Key: ");
			Serial.println ( cCodeKey ); 	
		#endif	
		return ( cCodeKey );

	}
	/**
	******************************************************
	* @brief Funcion que imprime los datos de un bloque UUID
	*
	* @param.- cUUID, UUID del que se quieren imprimir los datos
	*
	*/
	void printcUUID (String cUUID, BLEBeacon oBeacon, BLEAdvertisedDevice advertisedDevice)
	{
		Serial.println(cUUID);
		Serial.printf("ID: %04X Major: %d Minor: %d UUID: %s Power: %d\n", oBeacon.getManufacturerId(), ENDIAN_CHANGE_U16(oBeacon.getMajor()), ENDIAN_CHANGE_U16(oBeacon.getMinor()), oBeacon.getProximityUUID().toString().c_str(), oBeacon.getSignalPower());
		Serial.print ("Fecha de naciemiento ");
		Serial.println(cUUID.substring(0,8));
		Serial.print ("Boton: ");
		Serial.println (cUUID.substring( 8,12 ));
		Serial.print ("Codigo: ");
		Serial.println (cUUID.substring( 13,22 ));
		if (advertisedDevice.haveRSSI())
		{
			Serial.print("RSSI: ");
			Serial.println(advertisedDevice.getRSSI());
		}
		if (advertisedDevice.haveTXPower())
		{
			Serial.print("TXPower: ");
			Serial.println(advertisedDevice.getTXPower());
		}				
		if (advertisedDevice.haveName())
		{
			String cNombre = "";	
			cNombre = advertisedDevice.getName().c_str();
			Serial.println("########################");
			Serial.print ("Nombre dispositivo: ");
			Serial.println (cNombre);
			Serial.println(" ");
			Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
			Serial.println("########################");
		}	

	}
	/**
	******************************************************
	* @brief Funcion que imprime datos de la función Scan()
	*/
	void printScan (void)
	{
		#ifdef KeyMaster
	  		Serial.print ("Scan()->Boton Local: ");
	  		Serial.print (lBotonLocal);
	  		Serial.print (" Boton Remoto: ");
	  		Serial.print (lBotonRemoto);
	  		Serial.print (" Sistema: ");
	  		Serial.println (lEstadoLocal);
		#endif
		#ifdef KeySlave
	  		Serial.print ("Scan()->Boton Local: ");
	  		Serial.println (lBotonLocal);
		#endif
	}
	/**
	******************************************************
	* @brief Comprueba si el sistema tiene keybt detectado 
	*
	* Devuelve 1 el sistema detectado keybt, 0 en caso contrario
	* Es posible que la baliza no detecte key pero este en estado 1 por encontrarse en cilcos de escaneo antes de desconexion
	* o en caso de maestro, que puedas ser detectado por otra baliza
	*/
	boolean  KeyBtGetEstado (void)
	{
		return ( lEstadoLocal );
	}
	/**
	******************************************************
	* @brief Comprueba si esta baliaza tiene keybt detectado
	*
	* Devuelve 1 si la baliza detecta keybt, 0 en caso contrario
	*/
	boolean  KeyBtGetKeyLocal (void)
	{
		return ( lBotonLocal );
	}
	/**
	******************************************************
	* @brief Comprueba si hay baliza remota con deteccion de key
	*
	* Devuelve 1 si la baliza detecta keybt, 0 en caso contrario
	*/
	boolean  KeyBtGetKeyRemoto (void)
	{
		return ( lBotonRemoto );
	}		

	/**
	******************************************************
	* @brief Habilita el Pir
	*
	* Pone a 1 el flag lPir y habilita su interrupción
	*/
	void EnablePir (void)
	{
		lPir = 1;
		attachInterrupt(digitalPinToInterrupt(PinPir), IntPir,  FALLING);
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
	}
	/**
	******************************************************
	* @brief Arma el Pir
	*
	* aabilita la interrupción del pir
	*/
	void ArmaPir (void)
	{
		attachInterrupt(digitalPinToInterrupt(PinPir), IntPir,  FALLING);
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