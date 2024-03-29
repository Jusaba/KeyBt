/**
******************************************************
* @file ServerPic.h
* @brief Documentacion de módulo CAM
* @author Julian Salas Bartolome
* @version 1.0
* @date Junio 2021
*
* Notas
*
* Compilar con 2.0.11 no con 1.0.6 de las camaras
* En  AsyncWebSocket.cpp:843 cambiar return IPAddress(0U); por IPAddress((uint32_t)0);

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
		#define Led
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
		#define Led
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
		#define LogicaPir 1
		#define Sirena
		#define LogicaSirena 1
		#define Luz
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

	//----------------------------
	//Declaracion de variables PARTICULARES
	//----------------------------

	byte Configuracion;
	int nConfiguracion;
	String cConfiguracion;

	//----------------------------
	//Declaracion de variables PARTICULARES
	//----------------------------
	void LeeConfiguracionDispositivo ( void );
	void GrabaConfiguracionDispositivo ( int nBitDispositivo, boolean lEstado );
	void ModificaConfiguracionDispositivo ( int nBitDispositivo, boolean lEstado );
	boolean TestTemporizacion ( double nMillisInicio, double nMillisconsigna );
	
	#include "IO.h"
	#include "Alarma.h"
	#include "KeyBt.h"
	#ifdef Pir
		#include "Pir.h"
	#endif
	#ifdef Sirena
		#include "Sirena.h"
	#endif
	#ifdef Luz
		#include "Luz.h"
	#endif
	#ifdef Led
		#include "Led.h"
	#endif

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


	void LeeConfiguracionDispositivo ( void )
	{
		Configuracion = LeeVariable ( "Configuracion");
		#ifdef KeyMaster
			LeeEstadoKeyBt (Configuracion);
		#endif	
		#ifdef KeyEslave
			LeeEstadoKeyBt (Configuracion);
		#endif	
		#ifdef Pir
			LeeEstadoPir (Configuracion);
		#endif
		#ifdef Sirena
			LeeEstadoSirena (Configuracion);
		#endif
		#ifdef Luz
			LeeEstadoLuz (Configuracion);
		#endif
		LeeEstadoAlarma (Configuracion);
		

	}
	/**
	******************************************************
	* @brief Actualiza el byte Configuración en función del dispositivo y el estado del mismo
	*  y Graba en servidor la variable Configuracion actualizada
	*
	*/
	void GrabaConfiguracionDispositivo ( int nBitDispositivo, boolean lEstado )
	{
		if ( lEstado )
		{
			bitSet (Configuracion, nBitDispositivo);
		}else{
			bitClear (Configuracion, nBitDispositivo);
		}
		GrabaVariable ("Configuracion", Configuracion );
	}
	/**
	******************************************************
	* @brief Actualiza el byte Configuración en función del dispositivo y el estado del mismo
	*  
	*/
	void ModificaConfiguracionDispositivo ( int nBitDispositivo, boolean lEstado )
	{
		if ( lEstado )
		{
			bitSet (Configuracion, nBitDispositivo);
		}else{
			bitClear (Configuracion, nBitDispositivo);
		}
	}	
#endif