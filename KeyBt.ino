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

  pinMode (PinLed, OUTPUT);                             
  pinMode (PinReset, INPUT_PULLUP);                           //Configuramos el pin de reset como entrada

  #ifdef Pir
    pinMode (PinPir, INPUT_PULLUP);
  #endif
  #ifdef Sirena
    pinMode (PinSirena, OUTPUT);
  #endif

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
                    cDispositivoMaestro = ((cDispositivo).substring(0, nPos_))+"m";
                    Serial.println("--------------------------");
                    Serial.println(cDispositivoMaestro);
                    Serial.println("--------------------------");
                  }
                #endif  

                cKey = GetCodeKey();

                nTiempoTestbtn = TiempoTestbtnOff;
                if ( lEstadisticas )                                                //Si están habilitadas las estadisticas, actualizamos el numero de inicios
                {
                    GrabaVariable ("inicios", 1 + LeeVariable("inicios") );
                }
                LeeConfiguracionDispositivo();              //Cargamos el estado de los dispositivos garbado en Servidor
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
  if (lKeyBt)                                                            //Si esta habilitado la detección Bluetooth
  {
    #ifdef KeyMaster    
    if ( millis() > nMiliSegundosTestRemoto + TiempoTestbtnOnRemoto )     //Si ha trancurrido el tiempo de Test de deteccion de los remomtos
      {
        lBotonRemoto = 0;                                                 //Ponemos el Flag de deteccion de boton remoto a 0
        nMiliSegundosTestRemoto = millis();                               //Reseteamos el contador de tiempo de deteccion de remotos
      }
    #endif
    #ifdef KeySlave
    if ( millis() > nMiliSegundosInfoRemoto + TiempoTestbtnOnRemoto - 5000 )     //Si ha trancurrido el tiempo de envio estado a Master
    {
				if ( lEstadoLocal )											                                 //Si se ha detectado boton el este remoto
				{
					cSalida = "mensaje-:-"+cDispositivoMaestro+"-:-KeyOn";	                 //Mandamos mensaje On a master 
				}else{														                                       //Si no se ha detectado borton en este remoto
					cSalida = "mensaje-:-"+cDispositivoMaestro+"-:-KeyOff";	               //Mandamos mensaje Off a master
				}
				MensajeServidor(cSalida);				
				cSalida = String(' ');
        nMiliSegundosInfoRemoto = millis();
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
  }
  if ( lPir )
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
    if ( lIntPir == 1 && ( millis() > nMilisegundosNoPir + TiempoNoPir ) )
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
  if (lSirena)
  {
 		/*----------------
 		Comprobacion tiempo SirenaOn temporizado
 		------------------*/
		if ( lOnTemporizado )
		{
			if ( millis() > nMilisegundosOn + (nTiempoOn * 1000 ))
			{
				SirenaOff();	
				cSalida = "SirenaOff";
				EnviaValor (cSalida);
				cSalida = String(' ');
				lOnTemporizado = 0;
			}
		} 
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
      if (lKeyBt)
      {
   		  #ifdef KeyMaster
           if (oMensaje.Mensaje == "KeyOn")							//Si se recibe 'KeyOn'  
	  	  	  {	
               nContador = 0;                            //Reseteamos el contador
               nMiliSegundosTestRemoto = millis();       //Inicializamos el contador de tiempo test del remoto     
               lBotonRemoto = 1;                         //ponemos el flag de deteccion de remoto a 1
           }
           if (oMensaje.Mensaje == "KeyOff")							//Si se recibe 'KeyOff'
	  	  	  {	
               //nContador = 0;                          //Reseteamos el contador
               //nMiliSegundosTestRemoto = millis();       //Inicializamos el contador de tiempo test del remoto        
               lBotonRemoto = 0;                         //ponemos el flag de deteccion de remoto a 0
           }       
        #endif 
        if (oMensaje.Mensaje == "GetKey")							  //Si se recibe GetKey
	  	  {	  
          if (KeyBtGetEstado())                            //Si el sistema tiene detectada presencia de boton
          {
            if ( KeyBtGetKeyLocal() )                           //Comprueba si la baliza tiene boton detectado en local, si lo tiene prepara el tecto para informar
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
      }  
			if ((oMensaje.Mensaje).indexOf("SetCodeKey-:-") == 0) // Si se recibe una orden de "SetCodeKey-:-", se graba en EEPROM el CodeKey
      {
	  		String cCodeKey = String(oMensaje.Mensaje).substring(3 + String(oMensaje.Mensaje).indexOf("-:-"), String(oMensaje.Mensaje).length()); // Extraemos segundo parametro pasado
        SetCodeKey(cCodeKey);
        cKey = cCodeKey;
        cSalida = "Grabado el codigo " + cCodeKey;
  			oMensaje.Mensaje = cSalida;								  //Confeccionamos el mensaje a enviar hacia el servidor	
	  		oMensaje.Destinatario = oMensaje.Remitente;
		  	EnviaMensaje(oMensaje);									    //Y lo enviamos
        cSalida = String(' ');                      //Limpiamos cSalida 
      }
      if (oMensaje.Mensaje == "GetCodeKey")				  //Si se recibe GetCodeKey
	  	{	  
        oMensaje.Mensaje = cKey;                    //Cogemos el Code JKey
	  		oMensaje.Destinatario = oMensaje.Remitente;
		  	EnviaMensaje(oMensaje);									    //Y lo enviamos
      }

      if (oMensaje.Mensaje == "EnableKeyBt")					//Si se recibe EnableKeyBt
	  	{
        EnableKeyBt();
      }  
      if (oMensaje.Mensaje == "DisableKeyBt")					//Si se recibe DisableKeyBt
	  	{
        DisableKeyBt();
      }  

      if (oMensaje.Mensaje == "GetKeyBt")							  //Si se recibe GetKeyBt
	  	{
        cSalida = GetKeyBt ();
  			oMensaje.Mensaje = cSalida;								      //Confeccionamos el mensaje a enviar hacia el servidor	
	  		oMensaje.Destinatario = oMensaje.Remitente;
		  	EnviaMensaje(oMensaje);									        //Y lo enviamos
        cSalida = String(' ');                          //Limpiamos cSalida 

      }  

      /*****************************************************
      * Ordenes Pir
      * EnablePir.- Habilita el Pir
      * DisablePir.- Deshabilita el Pir
      * EnablePirSirena.- Habilita Pir y SirenaLocal
      * DisablePirSirena.- Deshabilita Pir y SirenaLocal
      * GetPir.- Devuelve con 1 o 0 si el Pir está Habilitado o Deshabilitado
      */

      if (oMensaje.Mensaje == "EnablePir")					//Si se recibe EnablePir
	  	{
        EnablePir();
      }  
      if (oMensaje.Mensaje == "DisablePir")					//Si se recibe DisablePir
	  	{
        DisablePir();
      }  
      if (oMensaje.Mensaje == "EnablePirSirena")		//Si se recibe DisablePir
	  	{      
        EnablePir();
        delay(250);
        SirenaLocalOn();
       }  
      if (oMensaje.Mensaje == "DisablePirSirena")		//Si se recibe DisablePir
	  	{
        DisablePir();
        delay(250);
        SirenaLocalOff();
      }  

      if (oMensaje.Mensaje == "GetPir")							  //Si se recibe GetPir
	  	{
        cSalida = GetPir ();
  			oMensaje.Mensaje = cSalida;								      //Confeccionamos el mensaje a enviar hacia el servidor	
	  		oMensaje.Destinatario = oMensaje.Remitente;
		  	EnviaMensaje(oMensaje);									        //Y lo enviamos
        cSalida = String(' ');                          //Limpiamos cSalida 

      }  

      /*****************************************************
      * Ordenes Sirena
      * EnableSirena.- Habilita la sirena
      * DisableSirena.- Deshabilita la sirena
      * GetSirenaEnable.- Devuelve con 1 o 0 si la sirena está Habilitada o Deshabilitada
      * SierenaOn.- Suena sirena permanentemente o durante un tiempo  determinado pasado como parametro
      * SirenaOff.- La sirena deja de sonar
      * SirenaFlash.- Hace sonar la sirena a rafagas
      * GetSirenaSuena.- Devuelve con 1 o 0 si la sirena esta sonando o no 
      * SirenaLocalOn.- Habilita la sirena para ser utilizada localmente
      * SirenaLocalOff.- Deshabilita la sirena para ser utilizada localmente
      * GetSirenaLocal.- Devuelve con 1 o 0 si la sirena esta habilitada o no para usarse loclamente 
      */
      if (oMensaje.Mensaje == "EnableSirena")					//Si se recibe EnableSirena
	  	{
        EnableSirena();
      }  
      if (oMensaje.Mensaje == "DisableSirena")					//Si se recibe DisableSirena
	  	{
        DisableSirena();
      }  
      if (oMensaje.Mensaje == "GetSirenaEnable")			  //Si se recibe GetSirenaEnable
	  	{
        cSalida = GetSirenaEnable ();
  			oMensaje.Mensaje = cSalida;								      //Confeccionamos el mensaje a enviar hacia el servidor	
	  		oMensaje.Destinatario = oMensaje.Remitente;
		  	EnviaMensaje(oMensaje);									        //Y lo enviamos
        cSalida = String(' ');                          //Limpiamos cSalida 
      }  
      if (oMensaje.Mensaje == "SirenaLocalOn")					//Si se recibe SirenaLocalOn
	  	{
        SirenaLocalOn();
      }  
      if (oMensaje.Mensaje == "SirenaLocalOff")					//Si se recibe SirenaLocalOff
	  	{
        SirenaLocalOff();
      }  
      if (oMensaje.Mensaje == "GetSirenaLocal")							  //Si se recibe GetSirenaLocal
	  	{
        cSalida = GetSirenaLocal ();
  			oMensaje.Mensaje = cSalida;								      //Confeccionamos el mensaje a enviar hacia el servidor	
	  		oMensaje.Destinatario = oMensaje.Remitente;
		  	EnviaMensaje(oMensaje);									        //Y lo enviamos
        cSalida = String(' ');                          //Limpiamos cSalida 
      }        
      if (lSirena)
      {
		    if ( (oMensaje.Mensaje).indexOf("SirenaOn") == 0)								//Si se recibe "SirenaOn"
			  {	
			  	if ( (oMensaje.Mensaje).indexOf("SirenaOn-:-") == 0)        	//Si se le pasa un parametro es que se quiere temporizar la sirena    
          {
			  		nTiempoOn =  (String(oMensaje.Mensaje).substring(  3 + String(oMensaje.Mensaje).indexOf("-:-"),  String(oMensaje.Mensaje).length() )).toInt();
			  		lOnTemporizado = 1;		
			  		nMilisegundosOn = millis();		
			  	}	  
			  	SirenaOn();	
			  	cSalida = "SirenaOn";
			  }
        if (oMensaje.Mensaje == "SirenaOff")					    //Si se recibe SirenaOff
	  	  {
          SirenaOff();
          lOnTemporizado = 0;
          cSalida = "SirenaOff";
        }  
			  if ((oMensaje.Mensaje).indexOf("SirenaFlash") == 0)					//Si se recibe "SirenaFlash"
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
			  	cSalida = "SirenaOn";
			  	EnviaValor (cSalida);
			  	while ( nFlash > 0 )
			  	{
			  		SirenaOn();	
			  		delay(nTiempoFlashOn*100);
			  		SirenaOff();
			  		delay(nTiempoFlashOff*100);	
			  		nFlash--;
			  	}
			  	cSalida = "SirenaOff";
			  	EnviaValor (cSalida);
			  }	
        if (oMensaje.Mensaje == "GetSirenaSuena")					//Si se recibe GetSirenaSuena
	  	  {
          cSalida = GetSirenaSuena ();
  		  	oMensaje.Mensaje = cSalida;								      //Confeccionamos el mensaje a enviar hacia el servidor	
	  	  	oMensaje.Destinatario = oMensaje.Remitente;
		    	EnviaMensaje(oMensaje);									        //Y lo enviamos
          cSalida = String(' ');                          //Limpiamos cSalida 
        }  
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



