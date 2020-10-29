#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

//made-up MAC address
byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCD, 0xDE, 0x03
};

//if DHCP fails:
IPAddress ip(192, 168, 2, 100);
IPAddress myDns(192, 168, 2, 1);
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255,0);
IPAddress broadcast(192,168,2,255);

//ports around 50000 and beyond are kinda safe to use
unsigned int localPort = 50000; 

byte packetBuffer[4];
byte broadcastBeacon[] = {0xFF, 0x00, 0x00, 0x00};  //might change afterwards 

EthernetUDP Udp;

struct device {     //might change afterwards
  IPAddress ip;
  
  bool used = false;

  String alias = "";
};

device devices[10];
int devicesFound = 0;  

void printDevices() {
  Serial.print("Devices found: ");
  Serial.println(devicesFound);
  for (int i = 0; i < devicesFound; i++) {
    Serial.print("Device number: " + String(i+1) + "\nDevice IP: " + printIP(devices[i].ip) + "\n\n");
  }
}

String printIP(IPAddress ip) {
  return String(ip[0], DEC) + "." +String(ip[1], DEC) + "." + String(ip[2], DEC) + "." + String(ip[3], DEC);
}

void setup() {
  Serial.begin(9600);
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

  delay(500);

  //making visible
  Udp.beginPacket(broadcast, 50000);
  Udp.write(broadcastBeacon, 4);
  Udp.endPacket();

  Serial.print("Beacon sent ");
  for (int i = 0; i < sizeof(broadcastBeacon); i++) {
    Serial.print(broadcastBeacon[i], HEX);
    Serial.write(' ');
  }
  Serial.println();  

  
}

void loop() { 
  int packetSize = Udp.parsePacket();
  Udp.readByte(packetBuffer, 4);                          //my own function added in EthernetUDP.cpp - pretty much copy and paste, but with different type
  
  //printing table of devices//
  if (Serial.read() == 'p') {
    printDevices();
  }
  //------------------------//

  //broadcast beacon receieved--------------------//
  if ((packetSize == 4) && (packetBuffer[0] == 0xFF)) {    //packetBuffer would never be equal to broadcastBeacon, no idea why...
    Serial.print("Broadcast beacon from: ");
    Serial.println(Udp.remoteIP());
    if (devices[devicesFound].used != true) {             //kinda rubbish, i know, wont be used afterwards
      devices[devicesFound].ip = Udp.remoteIP();
      devices[devicesFound].alias = "Arduino1";
      devices[devicesFound].used = true;
      devicesFound++;
    }    
  }
  //----------------------------------------------//
  else if (packetSize)
  {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i =0; i < 4; i++)
    {
      Serial.print(remote[i], DEC);
      if (i < 3)
      {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());
    Serial.println("Contents:");
    for (int i = 0; i < sizeof(packetBuffer); i++) {
      Serial.print(packetBuffer[i], HEX);
      Serial.write(' ');
    } 
  }
  delay(10);
  Ethernet.maintain();
}

