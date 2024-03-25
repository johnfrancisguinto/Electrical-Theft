 #include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <PZEM004Tv30.h>
#include <WiFi.h>

#include "ElectricalSystemConfig.h"
#include "MQTTHandler.h"
#include "UserConfig.h"

// Initialize MQTT
MQTTHandler mqttHandler;

TaskHandle_t MQTTMonitor;

#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17

#define PZEM_SERIAL Serial2

HardwareSerial sim(1); // RX, TX

PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 20, 4);

int _timeout;
int currentHour,
    currentMin;
    
String _buffer;

uint32_t updateTimeMillis = 0;

float voltage = 0,
      current = 0,
        power = 0,
       energy = 0;

void setup() {
    // Debugging Serial port
  Serial.begin(115200);

  _buffer.reserve(50);
  sim.begin(9600,SERIAL_8N1,18,19);
  
  // initialize the LCD
  lcd.begin();

  setup_wifi();

  // Initialize MQTT
  mqttHandler.init(mqttServer, mqttPort, mqttUser, mqttPassword, deviceESN);

  xTaskCreatePinnedToCore(MQTTMonitor_Routine, "MQTT Monitor", 5000, NULL, 1, &MQTTMonitor, 0);     // Default loop runs on Core 1
  delay(500);
    // Uncomment in order to reset the internal energy counter
    // pzem.resetEnergy()
}

void loop() {
  // monitor MQTT Connectivity
  mqttHandler.checkConnectivity();

  if(millis() - updateTimeMillis > 5000){
    mqttHandler.publish("GETtime", "updateTime");
    updateTimeMillis = millis();
  }
  
    // Print the custom address of the PZEM
    Serial.print("Custom Address:");
    Serial.println(pzem.readAddress(), HEX);

    // Read the data from the sensor
    voltage = pzem.voltage();
    current = pzem.current();
    power = pzem.power();
    energy = pzem.energy();

    // Check if the data is valid
    if(isnan(voltage)){
      Serial.println("Error reading voltage");
    } else if (isnan(current)) {
      Serial.println("Error reading current");
    } else if (isnan(power)) {
      Serial.println("Error reading power");
    } else if (isnan(energy)) {
      Serial.println("Error reading energy");
    }else {
      // Print the values to the Serial console
      Serial.print("Voltage: ");      Serial.print(voltage);      Serial.println("V");
      Serial.print("Current: ");      Serial.print(current);      Serial.println("A");
      Serial.print("Power: ");        Serial.print(power);        Serial.println("W");
      Serial.print("Energy: ");       Serial.print(energy,3);     Serial.println("kWh");
      lcd.clear();
      lcd.setCursor(0,0);//line 1
      lcd.print("Time: " + String(currentHour) + ":" + String(currentMin));
      lcd.setCursor(0,1);//line 2
      lcd.print("V:" + String(voltage) + " I:" + String(current));
      lcd.setCursor(0,2);//line 3
      lcd.print("E:" + String(energy) + " Kwh");
      lcd.setCursor(0,3);//line 4
      lcd.print("State: Normal");
      
    if(mqttHandler.checkConnectivity()){
      String payload = "{\"V\":\"" + String(voltage,1) + "\", \"A\":\"" + String(current,2) + "\", \"W\":\"" + String(power,2) + "\", \"kWh\":\"" + String(energy,2) + "\"}";
      Serial.println("PAYLOAD: " + payload);
      mqttHandler.publish(topicToPublish, payload.c_str());
    }
   }
    Serial.println();
    delay(10000);
}

void SendMessage()
{
  Serial.println ("Sending Message");
  sim.println("AT+CMGF=1\r");    //Sets the GSM Module in Text Mode
  delay(100);
  sim.println("AT+CMGS=\"" + number + "\"\r"); //Mobile phone number to send message
  delay(500);
  String SMS = "V:" + String(voltage) + " I:" + String(current) + " P:" + String(power) + " E:" + String(energy);
  sim.println(SMS);
  delay(500);
  sim.println((char)26);// ASCII code of CTRL+Z
  delay(500);
  sim.println();// ASCII code of CTRL+Z
  _buffer = _readSerial();
}

String _readSerial() {
  _timeout = 0;
  while  (!sim.available() && _timeout < 12000  )
  {
    delay(13);
    _timeout++;
  }
  if (sim.available()) {
    return sim.readString();
  }
}
