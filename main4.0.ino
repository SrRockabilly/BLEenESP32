#include <BLEDevice.h>
//#include <BLEUtils.h> //#include <BLEServer.h> //#include<iostream> 
//#include <string> // for string class //using namespace std; 
#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" 
#define CHARACTERISTIC_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

//Variables globales
// Asigacion de pines para el PWM:
int PWM1Pin = 19;
int PWM2Pin = 21;

// Propiedades de la señal PWM:
const int frecuencia = 30000; // 30 kHz
const int canal_PWM1 = 0;      // Canal PWM1 -> 0
const int canal_PWM2 = 1;      // Canal PWM1 -> 1
const int resolucion = 10;     // 10 bits de resolucion, 1024 posibles valores del "duty"
int duty = 0;

bool Start=0;
bool Giro=0;
int value_int=0;

String value_string="";  //variable intermedia
std::string rxvalue;  //Objeto rxvalue de la clase std::string
// std::string cero="0"; //Para comparar dos objetos std::string


class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic)
    {
      //std::string value = pCharacteristic->getValue();  //Local declaration
      rxvalue = pCharacteristic->getValue(); //Obtenemos el valor recibido
      
      if (rxvalue.length() > 0) //Si hay algo en rxvalue entonces
      {  
        value_string="";   //Reiniciamos variable intermedia
          for (int i = 0; i < rxvalue.length(); i++)
          {
            //Serial.print(rxvalue[i]); //para imprimir en un for desde rxvalue
            value_string=value_string+rxvalue[i];
          }
        if(value_string == "start"){  //arrancamos PWM
          Start=1;
        }
        else if (value_string == "stop"){ //paramos PWM
          Start=0;
        }
        else if (value_string == "change") //hacemos giro
        {
          if (Giro == 0){
            Giro=1; 
          }
          else{ 
            Giro=0;
          }
        }
        else
        {// Si lo que recibimos es un numero lo pasamos a string y de string a int
          for (int i = 0; i < rxvalue.length(); i++)
          {
            value_string=value_string+rxvalue[i];

          }
          value_int=value_string.toInt();
          if(value_int>100){
            value_int=100;
          }
          if(value_int<0){
            value_int=0;
          }
          duty=6*value_int;  //*10 pa led *6 pa motor

        }
      }
      Serial.print("Duty:\t");
      Serial.println(duty);
      Serial.print("Giro:\t");
      Serial.println(Giro);
      Serial.print("Start:\t");
      Serial.println(Start);
      Serial.print("\n");
   }
};

void setup() {
  Serial.begin(9600);   //Aqui declarar los pines de entrada y de salida
  
  // Configuro las entradas y salidas del microcontrolador:
  pinMode(PWM1Pin, OUTPUT);
  pinMode(PWM2Pin, OUTPUT);

  // Configuro el moduo PWM;
  ledcSetup(canal_PWM1, frecuencia, resolucion);
  ledcSetup(canal_PWM2, frecuencia, resolucion);
  
  // Asigno el PWM1Pin para ser controlado
  ledcAttachPin(PWM1Pin, canal_PWM1);
  ledcAttachPin(PWM2Pin, canal_PWM2);  

  // Configuracion de BLE: 
  BLEDevice::init("ESP32");
  BLEServer *pServer = BLEDevice::createServer(); //creamos servidor

  BLEService *pService = pServer->createService(SERVICE_UUID); //creamos servicio

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(  //creamos caracteristica
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->setValue("Hello World");
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();  //nos anunciamos para que nos vean
  pAdvertising->start();
}

void loop()   //programa principal
{ 
 
  // Asignacion de estados:
  if (Start == 1){
    if (Giro == 0){
      ledcWrite(canal_PWM1, duty);
      ledcWrite(canal_PWM2, 0);
    }
    if (Giro == 1){
      ledcWrite(canal_PWM1, 0);
      ledcWrite(canal_PWM2, duty);
    }
  }
  else{
      ledcWrite(canal_PWM1, 0);
      ledcWrite(canal_PWM2, 0);     
  }

  delay(100);
}
