#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCBundle.h>

#define LED LED_BUILTIN
#define BUTTON D5

// SWITCH LEG A -> 3V3
// SWITCH LEG B -> D5 + 10K -> GND

const int RSSI_MAX = -50; // define maximum strength of signal in dBm
const int RSSI_MIN = -100; // define minimum strength of signal in dBm

//const int displayEnc = 1; // set to 1 to display Encryption or 0 not to display

char* ssid[30] = {"artdesbruits2", "_BABEL", "g5scene-5G", "ssid4", "ssid5"};
char* pass[30] = {"0A1B2C3D4E", "modality.of.visible", "pulsopulso", "pass4", "pass5"};

bool connected = false;
char* cur_ssid = "none";
char* cur_pass = "none";

WiFiUDP Udp;                           // A UDP instance to let us send and receive packets over UDP
const unsigned int localPort = 8000;   // local port to listen for UDP packets at the NodeMCU (another device must send OSC messages to this port)
const unsigned int outPort = 14000;    // remote port of the target device where the NodeMCU sends OSC to
//const IPAddress outIp(192, 168, 0, 19);
IPAddress broadcast(0, 0, 0, 0);

int buttonState;
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 20;

void setup() {
  pinMode(BUTTON, INPUT);
  digitalWrite(BUTTON, HIGH); //turn pullup resistor on
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  Serial.begin(115200);
  //Serial.println("Robojax Wifi Signal Scan");
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(2000);

  //Serial.println("Setup done");
}

int StringSplit(String sInput, char cDelim, String sParams[], int iMaxParams)
{
  int iParamCount = 0;
  int iPosDelim, iPosStart = 0;

  do {
    // Searching the delimiter using indexOf()
    iPosDelim = sInput.indexOf(cDelim, iPosStart);
    if (iPosDelim > (iPosStart + 1)) {
      // Adding a new parameter using substring()
      sParams[iParamCount] = sInput.substring(iPosStart, iPosDelim - 1);
      iParamCount++;
      // Checking the number of parameters
      if (iParamCount >= iMaxParams) {
        return (iParamCount);
      }
      iPosStart = iPosDelim + 1;
    }
  } while (iPosDelim >= 0);
  if (iParamCount < iMaxParams) {
    // Adding the last parameter as the end of the line
    sParams[iParamCount] = sInput.substring(iPosStart);
    iParamCount++;
  }

  return (iParamCount);
}

void connect() {
  WiFi.begin(cur_ssid, cur_pass);

  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(50);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected to ");
  Serial.println(cur_ssid);

  Udp.begin(localPort);
  Serial.print("Connected, IP address: ");
  Serial.print(WiFi.localIP());
  Serial.print(" port: ");
  Serial.println(Udp.localPort());

  broadcast = WiFi.localIP();
  broadcast[3] = 255;
  Serial.print("Broadcast : ");
  Serial.print(broadcast);
  Serial.print(" port: ");
  Serial.println(outPort);

  connected = true;
  digitalWrite(LED, HIGH);
}

void loop() {


  buttonState = digitalRead(BUTTON);
  //Serial.print("BUTTON ");
  //Serial.println(buttonState);

  if (buttonState != lastButtonState) {
    if (buttonState == HIGH) {
      Serial.println("PHONE UP");
      sendPhone();
    } else {
      Serial.println("PHONE DOWN");
      sendPhone();
    }
    digitalWrite(LED, buttonState);
  }

  lastButtonState = buttonState;
  delay(20);

  if (!connected) {
    scan();
    //    if ( == 0) {
    //      delay(5000);
    //    }
  }
}

int scan() {
  Serial.println("Wifi scan started");
  digitalWrite(LED, HIGH);
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("Wifi scan ended");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    digitalWrite(LED, LOW);
    //Serial.print(n);
    //Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      //Serial.print(i + 1);
      //Serial.print(") ");
      //Serial.print(WiFi.SSID(i));// SSID

      /*
            Serial.print(WiFi.RSSI(i));//Signal strength in dBm
            Serial.print("dBm (");


            Serial.print(dBmtoPercentage(WiFi.RSSI(i)));//Signal strength in %
            Serial.print("% )");
            if (WiFi.encryptionType(i) == ENC_TYPE_NONE)
            {
              Serial.println(" <<***OPEN***>>");
            } else {
              Serial.println();
            }*/

      for (int j = 0 ; j < 5 ; j++) {
        if (WiFi.SSID(i) == ssid[j]) {
          Serial.print("Found Known SSID ");
          Serial.println(ssid[j]);
          cur_ssid = ssid[j];
          cur_pass = pass[j];
          connect();
          WiFi.scanDelete();
          return 1;
        }
      }
      delay(10);
    }
  }
  Serial.println("");

  // Wait a bit before scanning again
  WiFi.scanDelete();
  return 0;
}// loop

void sendPhone() {
  OSCMessage msg("/phone");
  msg.add(buttonState);
  Udp.beginPacket(broadcast, outPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
}

/*
   Written by Ahmad Shamshiri
    with lots of research, this sources was used:
   https://support.randomsolutions.nl/827069-Best-dBm-Values-for-Wifi
   This is approximate percentage calculation of RSSI
   WiFi Signal Strength Calculation
   Written Aug 08, 2019 at 21:45 in Ajax, Ontario, Canada
*/

int dBmtoPercentage(int dBm)
{
  int quality;
  if (dBm <= RSSI_MIN)
  {
    quality = 0;
  }
  else if (dBm >= RSSI_MAX)
  {
    quality = 100;
  }
  else
  {
    quality = 2 * (dBm + 100);
  }

  return quality;
}//dBmtoPercentage
