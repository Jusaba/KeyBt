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

  //********************
  // Inicializacion de BLE
  //********************
  BLEDevice::init("");                                        //Inicializamos BLE             
  pBLEScan = BLEDevice::getScan();                            //Inicia scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());  //Fijamos funcion para atencion de deteccion boton
  pBLEScan->setActiveScan(true);                              //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);                                    // less or equal setInterval value

  EEPROM.begin(256);                                          //Reservamos zona de EEPROM
  //BorraDatosEprom ( 0, 256 );                                 //Borramos n bytes empezando en la posicion 0   

  pinMode (PinReset, INPUT_PULLUP);                           //Configuramos el pin de reset como entrada

  #ifdef Led
    pinMode (PinLed, OUTPUT);                             
  #endif
  #ifdef Luz
    pinMode (PinLuz, OUTPUT);                             
  #endif
  #ifdef Pir
    pinMode (PinPir, INPUT_PULLUP);
  #endif
  #ifdef Sirena
    pinMode (PinSirena, OUTPUT);
  #endif

  if ( LeeByteEprom ( FlagConfiguracion ) == 0 )                            //Comprobamos si el Flag de configuracion esta a 0
  {                                                                         // y si esta
      ModoAP();                                                             //Lo ponemos en modo AP
  }else{                                                                    //Si no esta
    if ( ClienteSTA() )                                                     //Lo poenmos en modo STA y nos conectamos a la SSID
    {                                                                       //Si ha conseguido conectarse a ls SSID en modo STA
      if ( ClienteServerPic () )                                            //Intentamos conectar a ServerPic
      {
//          CheckFirmware();                                                //Comprobamos si el firmware esta actualizado a la ultima version
          #ifdef Debug
              Serial.println(" ");
              Serial.println("Conectado al servidor-------------");
          #endif 
          #ifdef  PulsadorLed                             
              ApagaLed();   
          #endif
          DataConfig aCfg = EpromToConfiguracion ();                        //Leemos la configuracin de la EEprom
          char USUARIO[1+aCfg.Usuario.length()]; 
          (aCfg.Usuario).toCharArray(USUARIO, 1+1+aCfg.Usuario.length());   //Almacenamos en el array USUARIO el nombre de usuario 
          cDispositivo = USUARIO;
          lHomeKit = aCfg.lHomeKit;
          lWebSocket = aCfg.lWebSocket;
          lEstadisticas = aCfg.lEstadisticas;
          #ifdef KeySlave                                                  //Si es exclavo, determinamos el nombre del maestro que es igual que el exclavo pero terminado en 'm' 
            if ( (cDispositivo).indexOf("_") > 0  )
            { 
              int nPos_ = (cDispositivo).indexOf("_");
              cDispositivoMaestro = ((cDispositivo).substring(0, nPos_))+"m";
            }
          #endif  
          cKey = GetCodeKey();
          nTiempoTestbtn = TiempoTestbtnOff;                               //Tiempo de escaneo Bluetooth rapido
          if ( lEstadisticas )                                             //Si están habilitadas las estadisticas, actualizamos el numero de inicios
          {
              GrabaVariable ("inicios", 1 + LeeVariable("inicios") );
          }
          LeeConfiguracionDispositivo();                                  //Cargamos el estado de los dispositivos garbado en Servidor
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

  /*---------------------------------------
  Comprobacion de lso distintos componentes
  ----------------------------------------*/
  if (lKeyBt)                           //Si esta habilitado la detección Bluetooth
  {
    KeyBt_Loop(cDispositivoMaestro);  //Comprobamso el estado de los remotos y el local y escaneamos el bluetooth si la temporización así lo determina
  }
  if ( lPir )                           //Si esta habilitado el PIR
  { 
    Pir_Loop(cDispositivo);           //Comprobamos si hay alarma de Pir y desencadenamos acciones
  }
  if (lSirena)                          //Si está habilitada la sirena
  {
    Sirena_Loop();                    //Comprobamos la temporizacion de la sirena sonando   
  }
  if (lLuz || lDestello || lLuzLocal )  //Si está habilitada la Luz-Led
  {
    Luz_Loop();                         //Si esta habilitado el destello, se hace
  }
  if (lAlarma)
  {
    Alarma_Loop();
  }
  /*----------------
  Comprobacion Conexion
  ------------------*/
  if ( TiempoTest > 0 )
  {
    if ( millis() > ( nMiliSegundosTest + TiempoTest ) )      //Comprobamos si existe conexion  
    {
      #ifdef  PulsadorLed                                     //Si no esta definido Debug
          EnciendeLed();                                      //Encendemos el led para indicar que se comprueba la conexion
      #endif    
      nMiliSegundosTest = millis();
      if ( !TestConexion(lEstadisticas) )                     //Si se ha perdido la conexion
      {
        lConexionPerdida = 1;                                 //Ponemos el flag de perdida conexion a 1
      }else{                                                  //Si existe conexion
        nConfiguracion  = LeeVariable("Configuracion");
        cConfiguracion = ((String) nConfiguracion);
        MensajeServidor((String)"Estado-:-Configuracion-:-"+(String) nConfiguracion);
      } 
    } 
  }

    /*----------------
    Analisis comandos
    ------------------*/
 
    /*****************************************************
     * Ordenes Pir
     *  EnablePir.- Habilita el Pir
     *  DisablePir.- Deshabilita el Pir
     *  GFetPir.- Devuelve con 1/0 si el Pir está Habilitado/Deshabilitado
     */
    oMensaje = Mensaje ();                            //Iteractuamos con ServerPic, comprobamos si sigue conectado al servidor y si se ha recibido algun mensaje

    if ( oMensaje.lRxMensaje)                         //Si se ha recibido ( oMensaje.lRsMensaje = 1)
    {
      #ifdef Debug        
        Serial.println(oMensaje.Remitente);           //Ejecutamos acciones
        Serial.println(oMensaje.Mensaje);
      #endif  
      if (lAlarma)
      {  
        /*****************************************************
        * Ordenes Keybt
        * Para todos los modulos 
        *    GetKey.- Devuelve si existe deteccion de Key en Sistema ( si es master) y/o en modulo si es Master o Slave
        *    SetCodeKey.- Fija codigo de la llave para que sea reconocida por el modulo 
        *    GetCodeKey.- Devuelve el codigo de recopnocimiento de llave 
        *    EnableKeyBt.- Habilita la detección de Llaves Bluetooth
        *    DisableKeyBt.- Deshabilita la detección de Llaves Bluetooth 
        *    GetKyyBt.- Devuelve con 1/0 si el módulo está Habilitado/Deshabilitado para deteccion Bluetooth 
        * Si el modulos es maestro
        *    KeyOn.- Pone el flag de detección en remoto a 1 e inicia contadores deteccion
        *    KeyOff.- Pone el flag de deteccion en remoto a 0 e inicia contadores de deteccion
        */        
        if (lKeyBt)
        {
   		    #ifdef KeyMaster
            if (oMensaje.Mensaje == "KeyOn")                  //Si se recibe 'KeyOn'  
	  	  	  {	
               nContador = 0;                             //Reseteamos el contador
               nMiliSegundosTestRemoto = millis();        //Inicializamos el contador de tiempo test del remoto     
               lBotonRemoto = 1;                          //ponemos el flag de deteccion de remoto a 1
            }
            if (oMensaje.Mensaje == "KeyOff")							    //Si se recibe 'KeyOff'
	  	  	  {	
               //nContador = 0;                          //Reseteamos el contador
               //nMiliSegundosTestRemoto = millis();     //Inicializamos el contador de tiempo test del remoto        
               lBotonRemoto = 0;                         //ponemos el flag de deteccion de remoto a 0
            }       
          #endif 
          if (oMensaje.Mensaje == "GetKey")							      //Si se recibe GetKey
	  	    {	  
            if (KeyBtGetEstado())                             //Si el sistema tiene detectada presencia de boton
            {
              if ( KeyBtGetKeyLocal() )                       //Comprueba si la baliza tiene boton detectado en local, si lo tiene prepara el texto para informar
              {
                cSalida = "Key detectado en esta baliza"; 
              }else{                                          //Si no lo tiene detectado en local.....
                cSalida = "Key no detectado en esta baliza pero si en una remota";
              }
            }else{                                            //Si no tiene detectada presencia de boton en local
              #ifdef KeyMaster                                //Prepara e mensaje a enviar en funcion de si es master o slave
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
        }  
		    if ((oMensaje.Mensaje).indexOf("SetCodeKey-:-") == 0) // Si se recibe una orden de "SetCodeKey-:-", se graba en EEPROM el CodeKey
        {
	  		  String cCodeKey = String(oMensaje.Mensaje).substring(3 + String(oMensaje.Mensaje).indexOf("-:-"), String(oMensaje.Mensaje).length()); // Extraemos segundo parametro pasado
          SetCodeKey(cCodeKey);
          cKey = cCodeKey;
          cSalida = "Grabado el codigo " + cCodeKey;
  			  oMensaje.Mensaje = cSalida;								          //Confeccionamos el mensaje a enviar hacia el remitente	
	  		  oMensaje.Destinatario = oMensaje.Remitente;
		  	  EnviaMensaje(oMensaje);									            //Y lo enviamos
          cSalida = String(' ');                              //Limpiamos cSalida 
        }
        if (oMensaje.Mensaje == "GetCodeKey")				          //Si se recibe GetCodeKey
	  	  {	  
          oMensaje.Mensaje = cKey;                            //Cogemos el Code JKey
	  		  oMensaje.Destinatario = oMensaje.Remitente;
		  	  EnviaMensaje(oMensaje);									            //Y lo enviamos
        }
        if (oMensaje.Mensaje == "EnableKeyBt")					      //Si se recibe EnableKeyBt
	  	  {
          EnableKeyBt();
        }  
        if (oMensaje.Mensaje == "DisableKeyBt")					      //Si se recibe DisableKeyBt
	  	  {
          DisableKeyBt();
        }  
        if (oMensaje.Mensaje == "GetKeyBt")							      //Si se recibe GetKeyBt
	  	  {
          cSalida = GetKeyBt ();
  			  oMensaje.Mensaje = cSalida;								          //Confeccionamos el mensaje a enviar hacia el servidor	
	  		  oMensaje.Destinatario = oMensaje.Remitente;
		  	  EnviaMensaje(oMensaje);									            //Y lo enviamos
          cSalida = String(' ');                              //Limpiamos cSalida 
        }  

        /*****************************************************
        * Ordenes Pir
        * EnablePir.- Habilita el Pir
        * DisablePir.- Deshabilita el Pir
        * EnablePirSirena.- Habilita Pir y SirenaLocal
        * DisablePirSirena.- Deshabilita Pir y SirenaLocal
        * GetPir.- Devuelve con 1 o 0 si el Pir está Habilitado o Deshabilitado
        * SirenaLocalOn.- Habilita la sirena para ser utilizada localmente por el Pir
        * SirenaLocalOff.- Deshabilita la sirena para ser utilizada localmente por el Pir
        * GetSirenaLocal.- Devuelve con 1 o 0 si la sirena esta habilitada o no para usarse loclamente con el Pir  
        */
        if (oMensaje.Mensaje == "EnablePir")					        //Si se recibe EnablePir
	  	  {
          EnablePir();
        }  
        if (oMensaje.Mensaje == "DisablePir")					        //Si se recibe DisablePir
	  	  {
          DisablePir();
        }  
        if (oMensaje.Mensaje == "EnablePirSirena")		        //Si se recibe DisablePir
	  	  {      
          SirenaLocalOn();
        }  
        if (oMensaje.Mensaje == "DisablePirSirena")		        //Si se recibe DisablePir
	  	  {
          SirenaLocalOff();
        }  
        if (oMensaje.Mensaje == "GetPir")							        //Si se recibe GetPir
	  	  {
          cSalida = GetPir ();
  		  	oMensaje.Mensaje = cSalida;								          //Confeccionamos el mensaje a enviar hacia el servidor	
	  	  	oMensaje.Destinatario = oMensaje.Remitente;
		    	EnviaMensaje(oMensaje);									            //Y lo enviamos
          cSalida = String(' ');                              //Limpiamos cSalida 
        }  
        if (oMensaje.Mensaje == "GetPirSirena")	   						//Si se recibe GetSirenaLocal
	  	  {
          cSalida = GetSirenaLocal ();
  		  	oMensaje.Mensaje = cSalida;								          //Confeccionamos el mensaje a enviar hacia el servidor	
	  	  	oMensaje.Destinatario = oMensaje.Remitente;
		    	EnviaMensaje(oMensaje);									            //Y lo enviamos
          cSalida = String(' ');                              //Limpiamos cSalida 
        }   
         /*****************************************************
         * Ordenes Sirena
         * EnableSirena.- Habilita la sirena
         * DisableSirena.- Deshabilita la sirena
         * GetSirenaEnable.- Devuelve con 1 o 0 si la sirena está Habilitada o Deshabilitada
         * SirenaOn.- Suena sirena permanentemente o durante un tiempo  determinado pasado como parametro
         * SirenaOff.- La sirena deja de sonar
         * SirenaFlash.- Hace sonar la sirena a rafagas
         * GetSirenaSuena.- Devuelve con 1 o 0 si la sirena esta sonando o no 
         */
        if (oMensaje.Mensaje == "EnableSirena")	  				    //Si se recibe EnableSirena
	  	  {
          EnableSirena();
        }  
        if (oMensaje.Mensaje == "DisableSirena")					    //Si se recibe DisableSirena
	  	  {
          DisableSirena();
        }  
        if (oMensaje.Mensaje == "GetSirenaEnable")			      //Si se recibe GetSirenaEnable
	  	  {
          cSalida = GetSirenaEnable ();
  		  	oMensaje.Mensaje = cSalida;								          //Confeccionamos el mensaje a enviar hacia el servidor	
	  		  oMensaje.Destinatario = oMensaje.Remitente;
		  	  EnviaMensaje(oMensaje);									            //Y lo enviamos
          cSalida = String(' ');                              //Limpiamos cSalida 
        }        
        if (lSirena)
        {
		      if ( (oMensaje.Mensaje).indexOf("SirenaOn") == 0)		//Si se recibe "SirenaOn"
			    { 	
			  	  if ( (oMensaje.Mensaje).indexOf("SirenaOn-:-") == 0)        	//Si se le pasa un parametro es que se quiere temporizar la sirena    
            {
			  		  nTiempoOn =  (String(oMensaje.Mensaje).substring(  3 + String(oMensaje.Mensaje).indexOf("-:-"),  String(oMensaje.Mensaje).length() )).toInt();
			  		  lOnTemporizado = 1;		
			  		  nMilisegundosOn = millis();		
			  	  }	  
			  	  SirenaOn();	
			      if ( lLuzLocal )																//Si esta habilitada la luz con sirena
			      {
 				      cSalida = "OnSirenaLuz";						  				//Mandaremos el texto OnSirenaLuz como ultimo valor
			      }else{
				      cSalida = "SirenaOn";													//Mandamos el texto SirenaOn como ultimo valor
			      }
 			    }
          if (oMensaje.Mensaje == "SirenaOff")					      //Si se recibe SirenaOff
	  	    {
            SirenaOff();
            lOnTemporizado = 0;
			      if ( lLuzLocal )																//Si esta habilitada la luz con sirena
			      {
				      cSalida = "OffSirenaLuz";						  				//Mandaremos el texto OnSirenaLuz como ultimo valor
			      }else{
				      cSalida = "SirenaOff";												//Mandamos el texto SirenaOn como ultimo valor
			      }
          }  
			    if ((oMensaje.Mensaje).indexOf("SirenaFlash") == 0)	//Si se recibe "SirenaFlash"
			    {
			    	if ((oMensaje.Mensaje).indexOf("SirenaFlash-:-") == 0)		//Si se reciben parametros
			    	{
			    		String cValor = String(oMensaje.Mensaje).substring(  3 + String(oMensaje.Mensaje).indexOf("-:-"),  String(oMensaje.Mensaje).length() ); //Extraemos los parametros
			    		nFlash =   (String(cValor).substring(0, String(cValor).indexOf("-:-"))).toInt();
			    		cValor = String(cValor).substring(  3 + String(cValor).indexOf("-:-"),  String(cValor).length() );
			    		nTiempoFlashOn =  (String(cValor).substring(0, String(cValor).indexOf("-:-"))).toInt();
			    		nTiempoFlashOff = (String(cValor).substring(  3 + String(cValor).indexOf("-:-"),  String(cValor).length() )).toInt();
 			    	}else{
			    		nFlash = nFlashOn;
			    		nTiempoFlashOn = tFlashOn;
			    		nTiempoFlashOff = tFlashOff;
			    	}	
			    	if ( lLuzLocal )	            									//Si esta habilitada la luz con sirena
			      {
 				      cSalida = "OnSirenaLuz";						  				//Mandaremos el texto OnSirenaLuz como ultimo valor
			      }else{
				      cSalida = "SirenaOn";													//Mandamos el texto SirenaOn como ultimo valor
			      }
			    	EnviaValor (cSalida);
            SirenaFlash(nFlash, nTiempoFlashOn, nTiempoFlashOff );
			      if ( lLuzLocal )																//Si esta habilitada la luz con sirena
			      {
				      cSalida = "OffSirenaLuz";						  				//Mandaremos el texto OnSirenaLuz como ultimo valor
			      }else{
				      cSalida = "SirenaOff";												//Mandamos el texto SirenaOn como ultimo valor
			      }			    	
            EnviaValor (cSalida);
			    }	
          if (oMensaje.Mensaje == "GetSirenaSuena")					  //Si se recibe GetSirenaSuena
	  	    {
            cSalida = GetSirenaSuena ();
  		    	oMensaje.Mensaje = cSalida;								      //Confeccionamos el mensaje a enviar hacia el servidor	
	  	    	oMensaje.Destinatario = oMensaje.Remitente;
		      	EnviaMensaje(oMensaje);									        //Y lo enviamos
            cSalida = String(' ');                          //Limpiamos cSalida 
          }  
        }

         /*****************************************************
         * Ordenes Luz
         * EnableLuz.- Habilita el Luz
         * DisableLuz.- Deshabilita el Luz
         * GetLuz.- Devuelve con 1 o 0 si el Luz está Habilitado o Deshabilitado
         * LuzOn.- Enciende el Luz
         * LuzOff.- Apaga el Luz
         * LuzFlash.- Flash de Luz
         * LuzSirenaOn.- Pone el flag lLuzLocal a 1 para enlazar sirena con Luz
         * LuzSirenaOff.- Pone el flag lLuzLocal a 0 para desenlazar sirena con Luz
         * GetLuzSirena.- Devuelve el estado del flag lLuzLocal
         */
        if ((oMensaje.Mensaje).indexOf("LuzOn") == 0)					              //Si se recibe LuzOn
	  	  {
			  	  if ( (oMensaje.Mensaje).indexOf("LuzOn-:-") == 0)   //Si se le pasa un parametro es que se quiere temporizar la luz    
            {
			  		  nTiempoLuzOn =  (String(oMensaje.Mensaje).substring(  3 + String(oMensaje.Mensaje).indexOf("-:-"),  String(oMensaje.Mensaje).length() )).toInt();
			  		  lOnLuzTemporizado = 1;		
			  		  nMilisegundosLuzOn = millis();		
			  	  }	            
            EnciendeLuz();
		        cSalida = "LuzOn";
        }  
        if (oMensaje.Mensaje == "LuzOff")					            //Si se recibe LuzOff
	  	  {
          ApagaLuz();
 		      cSalida = "LuzOff";
        } 
        if (oMensaje.Mensaje == "LuzFlash")	  				        //Si se recibe LuzFlash
	  	  {
          FlashLuz();
        }         
        if (oMensaje.Mensaje == "EnableLuz")					        //Si se recibe EnableLuz
	  	  {
          EnableLuz();
        }  
        if (oMensaje.Mensaje == "DisableLuz")					        //Si se recibe DisableLuz
	  	  {
          DisableLuz();
        }  
        if (oMensaje.Mensaje == "GetLuz")       							//Si se recibe GetLuz
	      	{
           cSalida = GetLuz ();
  	    		oMensaje.Mensaje = cSalida;								      //Confeccionamos el mensaje a enviar hacia el servidor	
	      		oMensaje.Destinatario = oMensaje.Remitente;
		     	EnviaMensaje(oMensaje);									        //Y lo enviamos
           cSalida = String(' ');                          //Limpiamos cSalida 
         }  
        if (oMensaje.Mensaje == "LuzSirenaOn")					      //Si se recibe LuzSirenaOn
	  	  {
          LuzLocalOn();
        }  
        if (oMensaje.Mensaje == "LuzSirenaOff")	      				//Si se recibe LuzSirenaOff
	  	  {
          LuzLocalOff();
        }  
        if (oMensaje.Mensaje == "GetLuzSirena")					      //Si se recibe GetLuzSirena
	  	  {
          cSalida = GetLuzLocal ();
  			  oMensaje.Mensaje = cSalida;								      	
	  		  oMensaje.Destinatario = oMensaje.Remitente;
		  	  EnviaMensaje(oMensaje);									        
          cSalida = String(' ');                           
        }  
    }
      /*****************************************************
      * Ordenes General
      * 
      * AlarmaEnable.- Habilita la alarma
      * AlarmaDisable.- Deshabilita la alarma
      * GetAlarma.- Devuelve el estado de la alarma
      * AlarmaDestelloOn.- Habilita el destello
      * AlarmaDestelloOff.- Deshabilita el destello
      * AlarmaDestelloGet.- Devuelve el estado del destello
      * 
      */
      if (oMensaje.Mensaje == "AlarmaEnable")					        //Si se recibe AlarmaEnable
	  	{
        EnableAlarma();
		    cSalida = "AlarmaEnable";
      }  
      if (oMensaje.Mensaje == "AlarmaDisable")				        //Si se recibe AlarmaEnable
	  	{
        DisableAlarma();
		    cSalida = "AlarmaDisable";
      }  
      if (oMensaje.Mensaje == "AlarmaGet")							      //Si se recibe AlarmaGet
	  	{
        cSalida = GetAlarma ();
  			oMensaje.Mensaje = cSalida;								      //Confeccionamos el mensaje a enviar hacia el servidor	
	  		oMensaje.Destinatario = oMensaje.Remitente;
		  	EnviaMensaje(oMensaje);									        //Y lo enviamos
        cSalida = String(' ');                          //Limpiamos cSalida 
      }      
      if (oMensaje.Mensaje == "AlarmaDestelloOn")				      //Si se recibe AlarmaDestelloOn
	  	{
        EnableDestello();
		    cSalida = "AlarmaDestelloOn";
      }  
      if (oMensaje.Mensaje == "AlarmaDestelloOff")				    //Si se recibe AlarmaDestelloOff
	  	{
        DisableDestello();
		    cSalida = "AlarmaDestelloOff";
      }    
      if (oMensaje.Mensaje == "AlarmaDestelloGet")							//Si se recibe AlarmaDestelloGet
	  	{
        cSalida = GetDestello ();
  			oMensaje.Mensaje = cSalida;								      //Confeccionamos el mensaje a enviar hacia el servidor	
	  		oMensaje.Destinatario = oMensaje.Remitente;
		  	EnviaMensaje(oMensaje);									        //Y lo enviamos
        cSalida = String(' ');                          //Limpiamos cSalida 
      }             
      /*----------------
      Actualizacion ultimo valor
      ------------------*/
      if ( cSalida != String(' ') )       //Si hay cambio de estado
      { 
        EnviaValor (cSalida);             //Actualizamos ultimo valor
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



