//http://wiki.icbbuy.com/doku.php?id=developmentboard:esp32-c3mini
//https://github.com/espressif/arduino-esp32/tree/master

#define ESP32
//#define Pantalla


#include "ServerPic.h"
#include "IO.h"




void setup() {
  
  
  #ifdef Debug                                                //Usamos el puereto serie solo para debugar 
    Serial.begin(115200);                                     //Si no debugamos quedan libres los pines Tx, Rx para set urilizados
    Serial.println("Iniciando........");
    Serial.setDebugOutput(true);
  #endif


  BLEDevice::init("");                                        //Inicializamos BLE             
  pBLEScan = BLEDevice::getScan();                            //Inicia scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());  //Fijamos funcion para atencion de deteccion boton
  pBLEScan->setActiveScan(true);                              //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);                                    // less or equal setInterval value

  EEPROM.begin(128);                                          //Reservamos zona de EEPROM
  //BorraDatosEprom ( 0, 128 );                               //Borramos n bytes empezando en la posicion 0   

  pinMode (PinLed, OUTPUT);                             
  pinMode (PinReset, INPUT_PULLUP);                           //Configuramos el pin de reset como entrada

    if ( LeeByteEprom ( FlagConfiguracion ) == 0 )            //Comprobamos si el Flag de configuracion esta a 0
    {                                                         // y si esta
        ModoAP();                                             //Lo ponemos en modo AP
    }else{                                                    //Si no esta
 
        if ( ClienteSTA() )                                   //Lo poenmos en modo STA y nos conectamos a la SSID
        {                                                     //Si jha conseguido conectarse a ls SSID en modo STA
            if ( ClienteServerPic () )                        //Intentamos conectar a ServerPic
            {
//              CheckFirmware();                              //Comprobamos si el firmware esta actualizado a la ultima version
                #ifdef Debug
                    Serial.println(" ");
                    Serial.println("Conectado al servidor-------------");
                #endif 
                #ifdef  PulsadorLed                           //Si no esta definido Debug
                    ApagaLed();   
                #endif
                DataConfig aCfg = EpromToConfiguracion ();   //Leemos la configuracin de la EEprom
                char USUARIO[1+aCfg.Usuario.length()]; 
                (aCfg.Usuario).toCharArray(USUARIO, 1+1+aCfg.Usuario.length());          //Almacenamos en el array USUARIO el nombre de usuario 
                cDispositivo = USUARIO;
                lHomeKit = aCfg.lHomeKit;
                lWebSocket = aCfg.lWebSocket;
                lEstadisticas = aCfg.lEstadisticas;
                #ifdef KeySlave                              //Si es exclavo, determinamos el nombre del maestro que es igual que el exclavo pero terminado en 'm' 
                  if ( (cDispositivo).indexOf("_") > 0  )
                  { 
                    int nPos_ = (cDispositivo).indexOf("_");
                    cDispositivoRemoto = ((cDispositivo).substring(0, nPos_))+"m";
                    Serial.println("--------------------------");
                    Serial.println(cDispositivoRemoto);
                    Serial.println("--------------------------");
                  }
                #endif  

                nTiempoTestbtn = TiempoTestbtnOff;
                if ( lEstadisticas )                                                //Si están habilitadas las estadisticas, actualizamos el numero de inicios
                {
                    GrabaVariable ("inicios", 1 + LeeVariable("inicios") );
                }
            }
        } 
    }

}


  void loop ()
  {
    


    /*----------------
    Comprobacion Reset
    ------------------*/
    //TestBtnReset (PinReset);

    /*----------------
    Comprobacion 
    ------------------*/
    #ifdef KeyMaster    
      if ( millis() > nMiliSegundosTestRemoto + TiempoTestbtnOnRemoto )   //Si ha trancurrido el tiempo de Test de deteccion de los remomtos
      {
        lBotonRemoto = 0;                                                 //Ponemos el Flag de deteccion de boton remoto a 0
        nMiliSegundosTestRemoto = millis();                               //Reseteamos el contador de tiempo de deteccion de remotos
      }
    #endif
    if ( millis() > nMiliSegundosTestbtn + nTiempoTestbtn )               //Si ha trnascurrido el tiempo para testear localmente si existe boton
    {
      Scan();
      if (TestCambioEstadoLocal(lBoton))                                  //Comproamos si ha habido cambio de estado en la deteccion de boton
      {
        Serial.println("*************** Cambio de estado ");
      }
      nMiliSegundosTestbtn = millis();                                    //Reseteamos el contador de testeo de boton

    }

    /*----------------
    Comprobacion Conexion
    ------------------*/
    if ( TiempoTest > 0 )
    {
      if ( millis() > ( nMiliSegundosTest + TiempoTest ) )      //Comprobamos si existe conexion  
      {
          #ifdef  PulsadorLed                     //Si no esta definido Debug
              EnciendeLed();                          //Encendemos el led para indicar que se comprueba la conexion
          #endif    


  


        nMiliSegundosTest = millis();
        if ( !TestConexion(lEstadisticas) )             //Si se ha perdido la conexion
        {
          lConexionPerdida = 1;                         //Ponemos el flag de perdida conexion a 1

        }else{                            //Si existe conexion
                   
        } 

      } 
    }

    /*----------------
    Analisis comandos
    ------------------*/
    /*****************************************************
     * Ordenes
     * GetKey.- Devuelve si existe deteccion de Key en Sistema ( si es master) y/o en modulo si es Master o Slave
     * Si el modulos es maestro
     * KeyOn.- Pone el flag de detección en remoto a 1 e inicia contadores deteccion
     * KeyOff.- Pone el flag de deteccion en remoto a 0 e inicia contadores de deeccion
    */
    oMensaje = Mensaje ();                            //Iteractuamos con ServerPic, comprobamos si sigue conectado al servidor y si se ha recibido algun mensaje

    if ( oMensaje.lRxMensaje)                         //Si se ha recibido ( oMensaje.lRsMensaje = 1)
    {
      #ifdef Debug        
        Serial.println(oMensaje.Remitente);           //Ejecutamos acciones
        Serial.println(oMensaje.Mensaje);
      #endif  

   		#ifdef KeyMaster
        if (oMensaje.Mensaje == "KeyOn")							//Si se recibe 'KeyOn'  
	  		{	
            nContador = 0;                            //Reseteamos el contador
            nMiliSegundosTestRemoto = millis();       //Inicializamos el contador de tiempo test del remoto     
            lBotonRemoto = 1;                         //ponemos el flag de deteccion de remoto a 1
        }
        if (oMensaje.Mensaje == "KeyOff")							//Si se recibe 'KeyOff'
	  		{	
            //nContador = 0;                            //Reseteamos el contador
            nMiliSegundosTestRemoto = millis();       //Inicializamos el contador de tiempo test del remoto        
            lBotonRemoto = 0;                         //ponemos el flag de deteccion de remoto a 0
        }        
			#endif
      if (oMensaje.Mensaje == "GetKey")							  //Si se recibe GetKey
	  	{	  
          if (GetEstado())                            //Si el sistema tiene detectada presencia de boton
          {
            if ( GetKey() )                           //Comprueba si la baliza tiene boton detectado en local, si lo tiene prepara el tecto para informar
            {
              cSalida = "Key detectado en esta baliza"; 
            }else{                                    //Si no lo tiene detectado en local.....

              cSalida = "Key no detectado en esta baliza pero si en una remota";
            }
          }else{                                      //Si no tiene detectada presencia de boton en local
            #ifdef KeyMaster                          //Prepara e mensaje a enviar en funcion de si es master o slave
              cSalida = "Key no detectado en el sistema";
            #endif  
            #ifdef KeySlave
              cSalida = "Key no detectado en esta baliza";             
            #endif  
          }  
  				oMensaje.Mensaje = cSalida;								  //Confeccionamos el mensaje a enviar hacia el servidor	
	  			oMensaje.Destinatario = oMensaje.Remitente;
		  		EnviaMensaje(oMensaje);									    //Y lo enviamos
          cSalida = String(' ');                      //Limpiamos cSalida 
      }
    /*----------------
      Actualizacion ultimo valor
      ------------------*/
      if ( cSalida != String(' ') )       //Si hay cambio de estado
      { 
        EnviaValor (cSalida);         //Actualizamos ultimo valor
      }

      /*----------------
      Actualizacion WebSocket
      ------------------*/
      #ifdef WebSocket
        if ( cSalida != String(' ') && lWebSocket )       //Si está habililtado WebSocket y hay cambio de estado en el dispositivo, se notifica a WebSocket
        { 
          EnviaMensajeWebSocket(cSalida);                 //Actualizamos las páginas html conectadas
        }
      #endif


      
      cSalida = String(' ');                    //Limpiamos cSalida para iniciar un nuevo bucle

      if ( lEstadisticas )                    //Si están habilitadas las estadisticas, actualizamos el numero de comandos recibidos
      {
        GrabaVariable ("comandos", 1 + LeeVariable("comandos") );
      } 

      nMiliSegundosTest = millis();   
    }    
    

}



