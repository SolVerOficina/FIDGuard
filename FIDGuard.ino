#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <SPI.h>
#include <MFRC522.h>
#include "SPIFFS.h"

#define LED 2
#define RELAYS 26 // opto derecha sw 1 cuadrado - | +
#define RELAYB 27 // opto izquierda sw 2 + | -
#define SS_PIN 21
#define RST_PIN 22
#define buzzer 5
int contador = 0;
int reseteo = 10;
float txValue = 0;
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
int nElementos = 0;
String ssid;
String pass;
String json;
StaticJsonDocument<500> doc;
JsonObject root;
JsonObject usuarios;

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

  //7C:9E:BD:45:52:8C
  //ascensor 1: 0x84, 0xCC, 0xA8, 0x7A, 0xB6, 0x60
  //ascensor 1 con 19 pisos: 84:CC:A8:7A:17:B0 - 0x84, 0xCC, 0xA8, 0x7A, 0x17, 0xB0
  //ascensor 2: 84:CC:A8:7A:AD:C8 - 0x84, 0xCC, 0xA8, 0x7A, 0xAD, 0xC8
  //ascensor 3: 84:CC:A8:7A:16:C8 - 0x84, 0xCC, 0xA8, 0x7A, 0x16, 0xC8 
  //prueba: 7C:9E:BD:45:70:74 - 0x7C, 0x9E, 0xBD, 0x45, 0x70, 0x74
  //rEPETIDOR lORA aNTENA rOJA 30:C6:F7:0D:D2:28 
  //repetidor LORA ANTENA BLANCA 58:BF:25:99:BF:F8
  //Darsalud 7C:9E:BD:ED:15:78 - 7C:9E:BD:F9:F4:5C
  // AIRCALL bucaros 84:CC:A8:7A:01:EC
  //Tarjeta prueba 7C:9E:BD:45:DA:58 7C:9E:BD:FA:17:B0

  #define SERVICE_UUID           "ffe0" // UART service UUID a5f81d42-f76e-11ea-adc1-0242ac120002
  #define CHARACTERISTIC_UUID_RX "ffe1"
  #define CHARACTERISTIC_UUID_TX "ffe2"

 uint8_t broadcastAddress[] = {0x84, 0xCC, 0xA8, 0x7A, 0x01, 0xEC};

 esp_now_peer_info_t peerInfo; 

String code[] = {
  "Alejandro",
  "JC",
  "Andres",
  "b1 6d 3f 20", 
  "sdfwdfwdedw",
  "dqdqwqdwdjd",
  "asdasdadsdd",
  "fdjhfjdfjsh",
  "dfidsljisdi",
  "fhidsfddhsu",
  "nfosiajdijd",
  "nsvhiosdfoi",
  "fodhioshoid",
  "hioeqhiodis",
  "fdnifhdshjd",
  "nsidoahfiug",
  "ndahosdoisd",
  "dodanksdjdi",
  "jdpeqjpeiia",
  "dnjshjjhfdj",
  "dsiojhfiods",
  "idoeocdcjka",
  "09 41 0f b3",
  "dnijnfjdfdj",
  "nfuhdshjdkk",
  "ndklanfjdnj",
  "dfniodsoiha",
  "mewjiojdwqo",
  "ipojqweiwka",
  "nciodjasjpa",
  "ndsijlaoajs",
  "ncdisonldsn",
  "ncdsncldsnn",
  "fudhhdshjaj",
  "fdshfsdhjka",
  "ncsakldklsj",
  "ndijdshjhja",
  "cdsjdsjhjaj",
  "nfsdjndjsjs",
  "ndlfldsjajj",
  "jdpequiqjak",
  "idsoajoisja",
  "disisoialak",
  "iewuoqpanns",
  "hfushhahjas",
  "buchanans18",
  "hfuiwsdklal",
  "bb 40 a9 09",
  "deserteagle",
  "hufwhqwoijd",
  "huohdshfsdi",
  "fbudihfdshu",
  "hfuisdiuuia",
  "fuidushfodd",
  "nudhfdsjkaj",
  "hdifdshhsis",
  "56 0a 16 9b", 
  "chduohdosua",
  "idajsiodjid",
  "cndsndnsjsl",
  "dasjnasndks",
  "cdsnncdsnjd",
  "hduhdsuhuds",
  "dsuohqhlnaa",
  "sioshdodhud",
  "01 8e b3 4d",
  "dnnsdnoalpq",
  "dsuhsabbshs",
  "ndsjnkjdsja",
  "nvdsnvkjsjn",
  "fnjdsnjkaja",
  "mdedjbajkkk",
  "hdiauhuauha",
  "sdbuifjksks",
  "hufiewhuoka",
  "b9 a4 57 c1", 
  "huowjdjakal",
  "fdhsajajaka",
  "ewuihewodjs",
  "haosnklsakl",
  "cdusuhuauau",
  "fusjknjkaoq",
  "bb fd 1b 00",
  "vofsodsisis",
  "fjdsjksjsks",
  "dsjdjhdshjs",
  "dsbjkaakaka",
  "fdsuihdjsjs",
  "ndajkfdsnjk",
  "bb 40 a9 01"
};

typedef struct struct_message{
  String a;
 String  uid;
  String nombre;
} struct_message;

 //Create a struct_message called myData
 struct_message myData;
 
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
 };

 class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
        Serial.println("***");
        Serial.print("Received Value: ");
        Serial.print(rxValue.length());
        Serial.println();
          for (int i = 0; i < rxValue.length(); i++) {
          Serial.println(rxValue[1]);
  //           credentials[i] = rxValue[i];  
          // Set values to send
         myData.a = rxValue[i];
         Serial.println(myData.a);

  
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));     
    if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  ESP.restart();
  //  delay(2000);                   
        }

      //h   save_data(ssid,pass);
        Serial.println();
        // Do stuff based on the command received from the app
    
        if (rxValue.find("Send") != -1) {
   
        }

        Serial.println();
        Serial.println("***");
      }
    }
};

void succes(){
  Serial.println("succes");
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
}

void failure(){
  Serial.println("failure");
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
}

void ble_setup(){

   // Create the BLE Device
  BLEDevice::init("FIDGuard SV"); // Give it a name
   // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                      
  pCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify.");
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  status == ESP_NOW_SEND_SUCCESS ? succes() : failure();
}

void setup() {
  Serial.begin(115200);   // Initiate a serial communication
  // Serial.println("Usuarios inscritos:");
  // create_user_json();
  Serial.print("Size of string: ");
  Serial.println(sizeof(String));
  nElementos = sizeof(code) / sizeof(String);
  Serial.print("Numero de elementos: ");
  Serial.println(nElementos);
 // Serial.println(code);
  pinMode(buzzer, OUTPUT);
  pinMode(RELAYS,OUTPUT);
  pinMode(RELAYB,OUTPUT);
  SPI.begin();      // Initiate  SPI bus
  pinMode(LED, OUTPUT);
  WiFi.mode(WIFI_STA);
  mfrc522.PCD_Init();   // Initiate MFRC522
  //Serial.println("Approximate your card to the reader...");
  //Serial.println();
  ble_setup();
 // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
   
  // register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  // register first peer  
 
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void check_if_exist(String lectura){
  Serial.print("uid: ");
  Serial.println(lectura);
    for(int a = 0; a < nElementos; a++){
      String usuario = code[a];    
      if (usuario == lectura){
         Serial.print("Usuario ");  
         Serial.print(a);    
         Serial.print(": ");    
         Serial.println(usuario);
       Serial.println("Usuario SI existe");    
       // abrir puerta con el nuevo tarjetero es activar un rel rele subida y rele bajada       
        digitalWrite(buzzer, HIGH);
        delay(120);
        digitalWrite(buzzer, LOW);
        delay(200);
        digitalWrite(buzzer, HIGH);
        delay(120);
        digitalWrite(buzzer, LOW);
        digitalWrite(LED,HIGH);
        digitalWrite(RELAYS,HIGH);
        digitalWrite(RELAYB,HIGH);
        delay(3000);
        digitalWrite(LED,LOW); 
        digitalWrite(RELAYB,LOW);
        digitalWrite(RELAYS,LOW);
        ESP.restart();       
       }else{
        //Serial.println("Usuario no existe");         
       }
    }
}

void sent_data(String uid, String nombre){
  Serial.println(uid);
  String uuid = uid;
  myData.a = "K";
  myData.uid = uid;
  myData.nombre = nombre;
  Serial.println(myData.uid);
  
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
 // delay(2000);
}

void loop() {
  //--------------LEER LOS COMENTARIOS SIEMPRE PARA ENTENDER QUE HACE LA FUNCION! -------------
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
  // Serial.print("Contador: ");
  // Serial.println(contador);  
    
  //--------------ESTA ES LA FUNCION QUE BUSCA EL NOMBRE DEPENDIENDO DEL UID QUE LLEGA-----------   
  //check_if_exist("5B CA 96 E4");
  
  contador++;
  //Serial.println("new card prtesent");
    if(contador > reseteo){ 
    Serial.println(" reseteando");
    ESP.restart();
    contador = 0;
    
    }
  //  //Serial.print("contador");
  //  Serial.println(contador);
    delay(1000);
      return;
    }
  //  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    //Serial.println(" Access denied");
    return;
  }
  //Show UID on serial monitor
 // Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
   //  Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
   //  Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  Serial.println(content.substring(1));
  String lectura = content.substring(1);
 // content.toUpperCase(content.substring(1));
    check_if_exist(lectura);
    
    
   //LLAVE MAESTRA ----------------------------
      if (lectura == "B1 6D 3F 20" ||
          content.substring(1) == "01 BE B3 40" ||
          content.substring(1) == "BB CE 59 01" ||
          content.substring(1) == "5B CA 96 E4" ||
          content.substring(1) == "B1 EB EB 20" ||
          content.substring(1) == "BB 9C 3B 01" ) //change here the UID of the card/cards that you want to give access
   {
  //FUNCION QUE RECIBE EL STRING DEL BLE Y LO ENVIA PARA BUSCAR EL NOMBRE ASIGNADO --------
     Serial.println();
     delay(500);
   }
} 
