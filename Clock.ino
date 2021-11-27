#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

const char *ssid = "****"; //Enter your WIFI ssid
const char *password = "****"; //Enter your WIFI password
ESP8266WebServer server(80);

const long utcOffsetInSeconds = -25200;

boolean on = true;
boolean change = false;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void handleRoot() {
 server.send(200, "text/html", "<form action=\"/LED_BUILTIN_on\" method=\"get\" id=\"form1\"></form><button type=\"submit\" form=\"form1\" value=\"On\">On</button><form action=\"/LED_BUILTIN_off\" method=\"get\" id=\"form2\"></form><button type=\"submit\" form=\"form2\" value=\"Off\">Off</button>");
}

void handleSave() {
 if (server.arg("pass") != "") {
   Serial.println(server.arg("pass"));
 }
}

Adafruit_7segment matrix = Adafruit_7segment();
Adafruit_7segment matrix2 = Adafruit_7segment();

int tim [6] = {0, 0, 2, 1, 2, 1};
boolean AM = false;
int days [7] = {28, 92, 120, 94, 116, 113, 109};
int counter = 4;
int rate = 1000;
int period = 1000 / rate;
String alarm = "0530";

void setup() {
   //play();
   pinMode(LED_BUILTIN, OUTPUT);
   delay(3000);
   Serial.begin(115200);
   Serial.println();
   Serial.print("Configuring access point...");
   WiFi.begin(ssid, password);
   while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
   }
   Serial.println("");
   Serial.println("WiFi connected");
   Serial.println("IP address: ");
   Serial.println(WiFi.localIP());
   server.on ( "/", handleRoot );
   server.on ("/save", handleSave);
   server.begin();
   Serial.println ( "HTTP server started" );
   
   server.on("/LED_BUILTIN_on", []() {
     digitalWrite(LED_BUILTIN, 1);
     on = true;
     change = true;
     Serial.println(on);
     handleRoot();
   });
   server.on("/LED_BUILTIN_off", []() {
     digitalWrite(LED_BUILTIN, 0);
     on = false;
     Serial.println(on);
     handleRoot();
   });

  matrix.begin(0x70);
  matrix2.begin(0x71);

  matrix.setBrightness(7);
  matrix2.setBrightness(7);
  matrix.drawColon(true);

  getTime();
  
  matrix.writeDisplay();
  matrix2.writeDisplay();
}

void getTime() {
  timeClient.update();
  counter = timeClient.getDay();
  String formatTime = timeClient.getFormattedTime();
  Serial.println(formatTime);
  String formatH = formatTime.substring(0, 2);
  String formatM = formatTime.substring(3, 5);
  String formatS = formatTime.substring(6);

  tim[5] = formatH.substring(0, 1).toInt();
  tim[4] = formatH.substring(1).toInt();
  tim[3] = formatM.substring(0, 1).toInt();
  tim[2] = formatM.substring(1).toInt();
  tim[1] = formatS.substring(0, 1).toInt();
  tim[0] = formatS.substring(1).toInt();

  /*for (int i = sizeof(tim)/sizeof(tim[0]) - 1; i >= 0; i--) {
    Serial.print(tim[i]);
  }*/
  Serial.println();
  Serial.println(WiFi.localIP());

  
  if (formatH >= "12") {
    AM = false;
    matrix2.writeDigitAscii(3, 80);
    tim[5] -= 1;
    tim[4] -= 2;
  } else {
    AM = true;
    matrix2.writeDigitAscii(3, 65);
  }

  matrix.writeDigitNum(0, tim[5]);
  matrix.writeDigitNum(1, tim[4]);
  matrix.writeDigitNum(3, tim[3]);
  matrix.writeDigitNum(4, tim[2]);
  matrix2.writeDigitNum(0, tim[1]);
  matrix2.writeDigitNum(1, tim[0]);

  matrix.drawColon(true);
  matrix2.writeDigitRaw(4, days[counter]);
  
  matrix.writeDisplay();
  matrix2.writeDisplay();
}

void loop() {
  server.handleClient();
  if (on) {
    if (change) {
      getTime();
      change = false;
    }
    incrementTime();
    delay(period);
  } else {
    matrix.print("");
    matrix2.print("");
    matrix.writeDisplay();
    matrix2.writeDisplay();
  }
}

void incrementTime() {
  
  tim[0] += 1;

  if (tim[0] > 9) {
    tim[0] = 0;
    tim[1] += 1;

    if (tim[1] > 5) {
      tim[1] = 0;
      tim[2] += 1;

      if (tim[2] > 9) {
        tim[2] = 0;
        tim[3] += 1;

        if (tim[3] > 5) {
          tim[3] = 0;
          tim[4] += 1;

          if (tim[4] > 9) {
            tim[4] = 0;
            tim[5] += 1;
            matrix.writeDigitNum(0, tim[5]);
          } else if (tim[5] == 1 && tim[4] == 3) {
            tim[4] = 1;
            tim[5] = 0;
            matrix.writeDigitNum(0, tim[5]);
          } else if (tim[5] == 1 && tim[4] == 2) {
            day();
          }

          matrix.writeDigitNum(1, tim[4]);
        }

        matrix.writeDigitNum(3, tim[3]);
      }

      matrix.writeDigitNum(4, tim[2]);
    }

    matrix2.writeDigitNum(0, tim[1]);
  }

  matrix2.writeDigitNum(1, tim[0]);

  char buffer[4];
  sprintf(buffer, "%d%d%d%d", tim[5], tim[4], tim[3], tim[2]);
  if (alarm.equals(buffer) && tim[1] + tim[0] == 0) {
    incrementTime();
    play();
  }

  matrix.writeDisplay();
  matrix2.writeDisplay();
}

void day() {
  if (AM) {
    AM = false;
    matrix2.writeDigitAscii(3, 80);
  } else {
    AM = true;
    matrix2.writeDigitAscii(3, 65);
    counter++;
    if (counter > 6) {
      counter = 0;
    }
    matrix2.writeDigitRaw(4, days[counter]);
  }
  matrix2.writeDisplay();
}
