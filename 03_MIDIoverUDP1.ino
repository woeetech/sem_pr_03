#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SoftwareSerial.h>

//made-up MAC address
byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCD, 0xDE, 0x01
};

//if DHCP fails:
IPAddress ip(192, 168, 2, 100);
IPAddress myDns(192, 168, 2, 1);
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255,0);
IPAddress broadcast(192,168,2,255);
IPAddress host(192,168,2,108);

//ports around 50000 and beyond are kinda safe to use
unsigned int localPort = 50000; 

byte packetBuffer[4];
byte broadcastBeacon[] = {0xFF, 0x00, 0x00, 0x00};  //might change afterwards 

EthernetUDP Udp;


String printIP(IPAddress ip) {
  return String(ip[0], DEC) + "." +String(ip[1], DEC) + "." + String(ip[2], DEC) + "." + String(ip[3], DEC);
}

SoftwareSerial midiSerial(3,5);
byte broadcastMSG[4];

void setup() {
  Serial.begin(38400);
  midiSerial.begin(31250);
  while(!Serial){
    ;
  }
  //<setup>
  Serial.println("Trying to obtain IP from DHCP...");
  if (Ethernet.begin(mac) == 0) {
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield not found!");
      while(true) {
        delay(1); //basically do nothing, we're done, end of story...
      }  
    }  
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable not connected!"); //never got this message, even with cable unplugged
    }
    Serial.println("Failed to obtain IP from DHCP, setting default values.");
    Ethernet.begin(mac, ip);
  }

  Serial.print("Arduino adress: ");
  Serial.println(Ethernet.localIP());
  ///<setup>

  Udp.begin(localPort);
  
}

void loop() { 
  if (midiSerial.available()) {
    broadcastMSG[0] = 0xAA;
    broadcastMSG[1] = midiSerial.read();
    broadcastMSG[2] = midiSerial.read();
    broadcastMSG[3] = midiSerial.read();

    Udp.beginPacket(host, 50000);
    Udp.write(broadcastMSG, 4);
    Udp.endPacket();
  }


  int packetSize = Udp.parsePacket();
  Udp.read(packetBuffer, packetSize);

  if (packetSize) {
    for (int i = 1; i < packetSize; i++)
    {
      Serial.write(packetBuffer[i]);
      digitalWrite(13, HIGH);
      digitalWrite(13, LOW);
    }
    
  }
  Ethernet.maintain();
}

