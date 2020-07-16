#include <boarddefs.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <ir_Lego_PF_BitStreamEncoder.h>
#include "EEPROM.h"
#include "BluetoothSerial.h" //Header File for Serial Bluetooth, will be added by default into Arduino
#include "esp_bt_main.h"
#include "esp_bt_device.h"
//#include "driver/include/driver/uart.h"

#define EEPROM_SIZE 128

BluetoothSerial ESP_BT; //Object for Bluetooth
const int LEFT=1,RIGHT=2,RIGHT2=2323,DISPENSE=3, SAVE=25,LOAD =26,STOP=7;
bool save = false;
const String DELIMITER = "*";
String incoming;
int LED_BUILTIN = 32;
int stillWaiting =2;//Paired or not
int photoPower =22;
int photoEmit =23;
int pP1 = 12,pE1 =13,iRP1 = 14;
int pP2 = 26, pE2 = 27,iRP2 = 25;
int irPower = 5;
bool waitingLED = false;
bool paired = false;

//setting up the photo emitters to be recieved
IRrecv irrecv(photoEmit);
IRrecv irrecv1(pE1);
IRrecv irrecv2(pE2);
// place to store results
decode_results results;
decode_results res1;
decode_results res2;
//Uart setup
//const int uart_num = UART_NUM_2;



void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){//callback i guess is a trace
  if(event == ESP_SPP_SRV_OPEN_EVT){//connected event
    Serial.println("Paired");
    paired = true;
  }
  else if(event == ESP_SPP_CLOSE_EVT)//disconnected event
  {
    Serial.println("Unpaired");
    paired = false;
  }
}

void setup() {
  pinMode(16, INPUT_PULLUP);
  Serial.begin(9600); //Start Serial monitor in 9600
  
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  
  ESP_BT.begin("ESP32"); //Name of your Bluetooth Signal
  Serial.println("Bluetooth Device is Ready to Pair");
  pinMode (LED_BUILTIN, OUTPUT);//Specify that LED pin is output
//  pairing light setup
  pinMode(stillWaiting,OUTPUT);
//  setting up the photo resistor power lines
  pinMode(photoPower,OUTPUT);
//  pinMode(pP1,OUTPUT);
//  pinMode(pP2,OUTPUT);
//  setting up the photo resistor
  pinMode(photoEmit,INPUT);
//  pinMode(pE1,INPUT);
//  pinMode(pE2,INPUT);
//  setting up the IR's
  pinMode(irPower,OUTPUT);
//  pinMode(iRP1,OUTPUT);
//  pinMode(iRP2,OUTPUT);
//  enabling the decoders
  irrecv.enableIRIn();
//  irrecv1.enableIRIn();
//  irrecv2.enableIRIn();
  

  
}
void loop() {
  
  //ensure paired or not
  ESP_BT.register_callback(callback);
  
  messageChecking();
  
  pairedBlinking();
//  readUart();
}



void messageChecking()
{
  if(irrecv.decode(&results)){
    Serial.println("recieved a signal");
    Serial.println(results.value,HEX);
    irrecv.resume();
  }
  if(!save){
//  Serial.println("reading "+digitalRead(photoEmit));
    if (ESP_BT.available()) //Check if we receive anything from Bluetooth
    {
      
      incoming = ESP_BT.readString(); //Read what we recievive 
      Serial.print("Received:");
      Serial.println(incoming);
      
      if(incoming.toInt()==LEFT){
        ESP_BT.print("Moving Left");
      }
      else if(incoming.toInt()==RIGHT){
        ESP_BT.print("Moving Right");
      }
      else if(incoming.toInt()==RIGHT2){
        ESP_BT.print("Moving Right*2");
      }
      else if(incoming.toInt()==DISPENSE){
        ESP_BT.print("Finished Dispensing");
      }
      else if(incoming.toInt()==STOP){
        ESP_BT.print("Stopped");
      }
  }
    


  }
  delay(100);
}
void spiceLevel(int num)
{
  int percent = 100;
  switch(num){
    case 0:
    break;
    
    case 1:
    // checking if first ir sensor is crossed
    digitalWrite(photoPower,HIGH);
    digitalWrite(irPower,HIGH);
    for(int i=0;i<20;i++){
      if(irrecv.decode(&results)){
        Serial.println("first photo resister read");
        percent-=25;
        irrecv.resume();
        break;
      }
    }
    digitalWrite(photoPower,LOW);
    digitalWrite(irPower,LOW);
    digitalWrite(photoEmit,LOW);
    break;
    
    case 2:
    // checking if first ir sensor is crossed
    digitalWrite(photoPower,HIGH);
    digitalWrite(irPower,HIGH);
    if(irrecv.decode(&results)){
      Serial.println("first photo resister read");
      percent-=25;
      irrecv.resume();
    }
    digitalWrite(photoPower,LOW);
    digitalWrite(irPower,LOW);
    
    // checking if second ir sensor is crossed
    digitalWrite(pP1,HIGH);
    digitalWrite(iRP1,HIGH);
    if(irrecv1.decode(&results)){
      Serial.println("Second photo resister read");
      percent-=25;
      irrecv.resume();
    }
    digitalWrite(pP1,LOW);
    digitalWrite(iRP1,LOW);
    break;

    case 3:
    // checking if first ir sensor is crossed
    digitalWrite(photoPower,HIGH);
    digitalWrite(irPower,HIGH);
    if(irrecv.decode(&results)){
      Serial.println("first photo resister read");
      percent-=25;
      irrecv.resume();
    }
    digitalWrite(photoPower,LOW);
    digitalWrite(irPower,LOW);
    
    // checking if second ir sensor is crossed
    digitalWrite(pP1,HIGH);
    digitalWrite(iRP1,HIGH);
    if(irrecv1.decode(&results)){
      Serial.println("Second photo resister read");
      percent-=25;
      irrecv.resume();
    }
    digitalWrite(pP1,LOW);
    digitalWrite(iRP1,LOW);

    // checking if third ir sensor is crossed
    digitalWrite(pP2,HIGH);
    digitalWrite(iRP2,HIGH);
    if(irrecv2.decode(&results)){
      Serial.println("Second photo resister read");
      percent-=25;
      irrecv.resume();
    }
    digitalWrite(pP2,LOW);
    digitalWrite(iRP2,LOW);
    break;
    
    default: 
    break;
    
  }
  if(percent==25){
    ESP_BT.print(percent);
    ESP_BT.println("% remaining, consider replacing soon");
  }
  else{
    ESP_BT.print(percent);
    ESP_BT.print("% remaining");
  }
  Serial.println(percent);
}

void pairedBlinking()
{
  
  if(paired)//if not connected
  {
    //turn on the LED that says it is paired
    waitingLED = true;
    digitalWrite(stillWaiting,HIGH);
  }
  else//blink the lights
  {
//  digitalWrite(stillWaiting,LOW);//the blinking light was getting annoying
    if(waitingLED){
        digitalWrite(stillWaiting,LOW);
      }
      else if(!waitingLED){
        digitalWrite(stillWaiting,HIGH);
      }
      waitingLED = !waitingLED;
      delay(500);
  }
}
