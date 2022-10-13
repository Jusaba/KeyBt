#define ESP32
//#define Pantalla


#include "ServerPic.h"
#include "IO.h"

const IPAddress remote_ip(192, 168, 1, 20);

void setup() {
  
  
  #ifdef Debug                                      //Usamos el puereto serie solo para debugar 
    Serial.begin(115200);                             //Si no debugamos quedan libres los pines Tx, Rx para set urilizados
    Serial.println("Iniciando........");
    Serial.setDebugOutput(true);
  #endif


  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value

  EEPROM.begin(128);                                //Reservamos zona de EEPROM
  //BorraDatosEprom ( 0, 128 );                     //Borramos n bytes empezando en la posicion 0   

  pinMode (PinLed, OUTPUT);
  pinMode(PinReset, INPUT_PULLUP);                        //Configuramos el pin de reset como entrada

    if ( LeeByteEprom ( FlagConfiguracion ) == 0 )            //Comprobamos si el Flag de configuracion esta a 0
    {                                 // y si esta
        ModoAP();                           //Lo ponemos en modo AP
    }else{                                //Si no esta
 
        if ( ClienteSTA() )                       //Lo poenmos en modo STA y nos conectamos a la SSID
        {                               //Si jha conseguido conectarse a ls SSID en modo STA
            if ( ClienteServerPic () )                  //Intentamos conectar a ServerPic
            {
//              CheckFirmware();                      //Comprobamos si el firmware esta actualizado a la ultima version
                #ifdef Debug
                    Serial.println(" ");
                    Serial.println("Conectado al servidor-------------");
                #endif 
                #ifdef  PulsadorLed                             //Si no esta definido Debug
                    ApagaLed();   
                #endif
                DataConfig aCfg = EpromToConfiguracion ();        //Leemos la configuracin de la EEprom
                char USUARIO[1+aCfg.Usuario.length()]; 
                (aCfg.Usuario).toCharArray(USUARIO, 1+1+aCfg.Usuario.length());          //Almacenamos en el array USUARIO el nombre de usuario 
                cDispositivo = USUARIO;
                lHomeKit = aCfg.lHomeKit;
                lWebSocket = aCfg.lWebSocket;
                lEstadisticas = aCfg.lEstadisticas;
                #ifdef KeySlave
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


                if ( lEstadisticas )                  //Si están habilitadas las estadisticas, actualizamos el numero de inicios
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
    if ( millis() > nMiliSegundosTestbtn + nTiempoTestbtn )
    {

      Scan();
      if (TestCambioEstadoLocal(lBoton))
      {
        Serial.println("*************** Cambio de estado ");
      }
      nMiliSegundosTestbtn = millis();
/*
      TestCambioEstado();                         //Comprobamos si hay cambio de estado

      if ( GetEstado () )                         //Si  Baliza detecta el KeyBt
      {
        nTiempoTestbtn = TiempoTestbtnOn;         //Establecemos un tiempo de escaneo largo, no importa si tarda en detectar que no hay KeyBt para conectar la alarma
        if (GetKey())
        {
          EnciendeLed();
        }else{
          FlashLed();
        }  
      }else{                                      //SI no detecta
        nTiempoTestbtn = TiempoTestbtnOff;        //Establecemos un tiempo corto de escaneo para que la baliza desconecta la alarma nada mas detectar el KeyBt siendo ideal que lo haga antes de abrir la puerta
        ApagaLed();
      }
      lBoton=0;
	    #ifdef KeyMaster
		    lBotonRemoto = 0;
	    #endif
      nMiliSegundosTestbtn = millis();
      Serial.print(nTiempoTestbtn);
      Serial.print(" ");
      Serial.print(nMiliSegundosTestbtn);
      Serial.println ("   Scan Bt");
*/
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
          lConexionPerdida = 1;                 //Ponemos el flag de perdida conexion a 1
/*
          if ( GetDispositivo() )                 //Si el dispositivo estaba a On
          {
            lEstado = 1;                    //Guardamos ese estado para tenerlo en cuenta en la reconexion
            DispositivoOff();                 //Ponemos a Off el dispositivo  
          } 
        }else{                            //Si existe conexion
            #ifdef  PulsadorLed                   //Si no esta definido Debug
                ApagaLed();                     //Apagamos el led para indicar que se ha finalizado el test                                                                                      
            #endif    
          if ( lConexionPerdida )                 //Comprobamos si es una reconexion ( por perdida anterior )
          {                           //Si lo es
            lConexionPerdida = 0;               //Reseteamos flag de reconexion
            if ( lEstado )                    //Ponemos el dispositivo en el estado anterior a la perdida de conexion
            {
              DispositivoOn();
            }else{
              DispositivoOff();
            }
            lEstado = 0;                    //Reseteamos el Flag lEstado
          } 
*/         
        } 

      } 
    }

    /*----------------
    Analisis comandos
    ------------------*/
    oMensaje = Mensaje ();                      //Iteractuamos con ServerPic, comprobamos si sigue conectado al servidor y si se ha recibido algun mensaje

    if ( oMensaje.lRxMensaje)                   //Si se ha recibido ( oMensaje.lRsMensaje = 1)
    {
      #ifdef Debug        
        Serial.println(oMensaje.Remitente);           //Ejecutamos acciones
        Serial.println(oMensaje.Mensaje);
      #endif  

   		#ifdef KeyMaster
        if (oMensaje.Mensaje == "KeyOn")							//Si se recibe 'Change', cambia el estado de PinSalida 
	  		{	
            nContador = 0;
            lBotonRemoto = 1;
        }
			#endif
      if (oMensaje.Mensaje == "GetKey")							//
	  	{	  
          if (GetEstado())
          {
            if ( GetKey() )
            {
              cSalida = "Key detectado en esta baliza"; 
            }else{
              cSalida = "Key detectado en el sistema pero no en esta baliza ";
            }
          }else{
            #ifdef KeyMaster
              cSalida = "Key no detectado en el sistema";
            #endif  
            #ifdef KeySlave
              cSalida = "Key no detectado en esta baliza";             
            #endif  
          }  
  				oMensaje.Mensaje = cSalida;								//Confeccionamos el mensaje a enviar hacia el servidor	
	  			oMensaje.Destinatario = oMensaje.Remitente;
		  		EnviaMensaje(oMensaje);									//Y lo enviamos
          cSalida = String(' ');                    //Limpiamos cSalida para no actualizar valor      }
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
          EnviaMensajeWebSocket(cSalida);           //Actualizamos las páginas html conectadas
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



