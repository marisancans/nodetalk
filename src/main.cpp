// WiFi includes
 #include <ESP8266WiFi.h>

 // OTA Includes
 #include <ESP8266mDNS.h>
 #include <ArduinoOTA.h>
 #include <PubSubClient.h>
 #include <ESP8266HTTPClient.h>
 #include "SSD1306Wire.h"

 const char* ssid         = "kasteste";
 const char* password     = "punkaripa";
 const char* mqttServer   = "178.128.197.152";
 const char* djangoServer = "http://178.128.197.152/nodetalk/getdata/?node_id=";// TESTING
 const char* id           = "5";
 const char* subscribeTo  = "nodetalk/update/";
 const char  sep          = ',';

WiFiClient espClient;
PubSubClient client(espClient);

SSD1306Wire  display(0x3c, D2, D1);


void getImage()
{
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http; 
        String g = String(djangoServer) + String(id);
        Serial.print("GET reuest to: ");
        Serial.println(g);
        http.begin(g);
        int httpCode = http.GET();
                                                                
        if (httpCode > 0) {
            String payload = http.getString();

            auto getValue = [&payload](){ 
                int pos = 0;
                for(auto const& i : payload)
                {
                    if(i == sep){
                        auto x = payload.substring(0, pos);
                        payload.remove(0, pos + 1);
                        return x;
                    }
                    pos++;
                } 
                return String("");
            };

            
            
            int imgData[3];//  0 width, 1 height, 2 xbmSize
            for(int i = 0; i < 3; ++i)
                imgData[i] = getValue().toInt();

            uint8_t* buffer = new uint8_t [imgData[2]]();
            Serial.println(imgData[2]);
            

            for(int i = 0; i < imgData[2]; ++i)
                buffer[i] = getValue().toInt();


            for(int i = 0; i < imgData[2]; ++i)
                Serial.println(buffer[i]);

            display.clear();
            display.drawXbm(0, 0,imgData[0], imgData[1], buffer);
            display.display();
            
        }
    http.end();   
  }
}

void callback(char* topic, byte* payload, unsigned int length) {

    Serial.print("Message arrived in topic: ");
    Serial.println(topic);

    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    display.clear();
    display.drawString(1, display.getHeight()/2, "Got new image");
    display.display();

    getImage();

    Serial.println();
    Serial.println("-----------------------");
 
}


void setup() {
    Serial.begin(115200);
    WiFi.begin ( ssid, password );

    display.init();
    display.setContrast(255);

    // Wait for connection
    while ( WiFi.status() != WL_CONNECTED ) {
        delay ( 10 );
        display.clear();
        Serial.println("Connecting to WiFi..");
        display.drawString(0, display.getHeight()/2, "Connecting to WiFi..");
        display.display();
    }
    display.clear();
    Serial.println("Connected to the WiFi network");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    client.setServer(mqttServer, 1883);
    client.setCallback(callback);


    ArduinoOTA.begin();
    ArduinoOTA.onStart([]() {
        display.clear();
        display.setFont(ArialMT_Plain_10);
        display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
        display.drawString(display.getWidth()/2, display.getHeight()/2 - 10, "OTA Update");
        display.display();
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        display.drawProgressBar(4, 32, 120, 8, progress / (total / 100) );
        display.display();
    });

    ArduinoOTA.onEnd([]() {
        display.clear();
        display.setFont(ArialMT_Plain_10);
        display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
        display.drawString(display.getWidth()/2, display.getHeight()/2, "Restart");
        display.display();
    });

    // Align text vertical/horizontal center
    display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
    display.setFont(ArialMT_Plain_10);
    display.drawString(display.getWidth()/2, display.getHeight()/2, "Ready for OTA:\n" + WiFi.localIP().toString());
    display.display();

    while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    
    String client_name = String("ESP32Client_") + String(id);
    if (client.connect(client_name.c_str())) {
            Serial.println("connected");  
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }

    std::string buf(subscribeTo);
    buf.append(id);
    client.subscribe(buf.c_str());
    getImage();

}

void loop() {
    ArduinoOTA.handle();
    client.loop();
}