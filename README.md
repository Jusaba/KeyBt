# KeyBt
El uso principal de un dispositivo KeyBt es detectar la presencia o ausencia de un usuario en un domicilio para el control de acceso.

El sistema utiliza el bluetooth y se compone de dos elementos fundamentales, la **Baliza** y la **llave**.

La **Baliza** es un dispositivo compatible con Serverpic basado en ESP32 ya que los modulos ESP32 disponen de la facilidad Bluetooth incorporada.

Las **Llaves** son botones/llaveros con modulos nRF52810 o similar que emniten señal Bluetooth BLE detectable por las **Balizas**

El principio de funcionamiento es muy sencillo, el usuario porta una **Llave** de las mencionadas. En circunstancias normales el usuario se encuentra en el domicilio, La **Baliza** tiene registrado el botón como localizado. Si el usuario abandona el domicilio la **Llave** abondona el radio de detección de la **Baliza** y esta lo detecta como cambio de estado  y desencadena las acciones que se fijen ante el cambio de estado de presencia a ausencia, por ejemplo la conexión de un PIR.

Cuando ll usuario regresa al domicilio, la **Baliza** detecta la presencia de la **LLave**, se produce ahora un cambio de estado de ausencia a presencia y desencadena las acciones asignadas a ese cambio de estado, por ejemplo la desconexión de un PIR.

Como que un domicilio normalmente tiene varios residentes y cada uno tendrá su  **Llave**, aunque el sistema detecte que un elemento abandona el domicilio no debe desencadenar las acciones asignadas a la ausencia de **Llave** mientras al menos una se encuentre en el domicilio. Por otro lado cada residente, en el domicilio depositará su **Llave** en una estancia distinta dentro de la casa por lo que el sistema se complica.

Para garantizar que el sistema detecta la existencia de al menos una **Llave** en la vivienda será necesario varias **Balizas**, uno de ellas será la **Maestra** que se intentará ubicar en la entrada de la casa y el resto serán **Excalvas** y se distribuiran en la vivienda dando cobertura de detección a todas las estancias.

Los **Excalvos** deberán comunicar al **Maestro** si en su radio de detección se encuentra alguna **Llave**, con esa información y con la propia del **Maestro**, este decidirá si hay un cambio de estado de Ausencia a Presencia o viceversa en la vivienda.

Más detalles del sistema en la [Wiki](https://github.com/Jusaba/KeyBt/wiki) 


