
void MQTTMonitor_Routine( void * pvParameters ){

  Serial.print("MQTT monitoring running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){

    // Check if there is an incoming message
    if (mqttHandler.messageAvailable()) {
        // Get the topic and payload of the incoming message
        String incomingTopic = mqttHandler.getMessageTopic();
        String incomingPayload = mqttHandler.getMessagePayload();
        
        // Process the incoming message
        Serial.print("Received message on topic: ");
        Serial.println(incomingTopic);
        Serial.print("Message payload: ");
        Serial.println(incomingPayload);

        if(incomingTopic == "GIVEtime"){

          // Sample payload {21:10:03}
          int index[4] = {0};
          index[0] = incomingPayload.indexOf('{');
          index[3] = incomingPayload.indexOf('}');

      
          if(index[0] < 0 || index[3] < 0){
            Serial.println("Invalid Command");
          } 
      
          for(byte x = 1; x < 3; x++){
            index[x] = incomingPayload.indexOf(':', index[x - 1] + 1);
          }

          int _hour   = incomingPayload.substring(index[0] + 1, index[1]).toInt();
          int _minute = incomingPayload.substring(index[1] + 1, index[2]).toInt();
          int _second = incomingPayload.substring(index[2] + 1, index[3]).toInt();

          Serial.println("HOUR: "   + String(_hour));
          Serial.println("MINUTE: " + String(_minute));
          Serial.println("SECOND: " + String(_second));
          
          currentHour = _hour;
          currentMin = _minute;
          
        }
        else if(incomingTopic == "P003_Detection"){
          SendMessage();   
          }
  
        // Clear the flag after processing the message
        mqttHandler.clearMessageFlag();
    }
  
    delay(1);
  } 
  
}


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
