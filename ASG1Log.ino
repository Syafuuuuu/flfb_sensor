#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <WiFiUdp.h>
#include "DHT.h"

/*

Firebase connection code was pulled from 
https://github.com/ahmadhanis/SKIH3113-Sensor/blob/main/dht_firebase_datalogger/dht_firebase_datalogger.ino
Thank you very much Dr!

DHT22 code was adapted from the DHT sesnor library by Adafruit from version 1.4.6
Good job Adafruit?

Ultrasonic Sensor code was adapted from 
https://randomnerdtutorials.com/esp8266-nodemcu-hc-sr04-ultrasonic-arduino/
Thank god for random nerds!

*/

//Network Related
bool signupOK = false;
const char* ssid = "UUMWiFi_Guest";
const char* pass = "";
float hum = 0, temp = 0;
String relay = "Off", prevRelay = "On";

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;

//DHT Library Defines
#define DHTPIN 4       // Digital pin connected to the DHT sensor at D2
#define DHTTYPE DHT22  // DHT 22  (AM2302), AM2321

//define sound velocity in cm/uS
const int trigPin = 12;  // Trigger pin connected to the Ultrasonic sensor at D6
const int echoPin = 14;  // Echo pin connected to the Ultrasonic sensor at D5
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701

//define Ultrasonic related variables
long duration;
float distanceCm;

DHT dht(DHTPIN, DHTTYPE); //set dht pin and type of dht

//Setup Section
void setup() {
  Serial.begin(115200); //Begin serial monitor 

  //Network Setup
  delay(100);
  WiFi.begin(ssid, pass);
  pinMode(12, OUTPUT);
  delay(100);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  /* Assign the api key (required) */
  config.api_key = "REDACTED";

  /* Assign the RTDB URL (required) */
  config.database_url = "REDACTED";

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  config.signer.preRefreshSeconds = 3420; 

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  //DHT Setup
  dht.begin();

  //US Setup
  pinMode(trigPin, OUTPUT);  // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);   // Sets the echoPin as an Input
}

//Looping Section
void loop() {
  // Wait a few seconds between measurements.
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)) {
    getDHT(); //Get DHT data - humidity and temperature
    getDist(); //Get Ultrasonic data - distance (cm)
    sendDataPrevMillis = millis();
    // Write an Int number on the database path test/int

    //Send humidity to RTDB
    if (Firebase.RTDB.setInt(&fbdo, "101/hum", hum)) {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
      Serial.println(hum);
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    delay(500);

    //Send temperature to RTDB
    if (Firebase.RTDB.setInt(&fbdo, "101/temp", temp)) {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
      Serial.println(temp);
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    delay(500);

    //Send distance to RTDB
    if (Firebase.RTDB.setInt(&fbdo, "101/dist", distanceCm)) {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
      Serial.println(distanceCm);
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
  else{
    delay(100);
    Serial.print("~");
  }
}

void getDist() {
  //Initialise trigger pin to low at the start
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH); //Shoot out signal
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW); //Stops signal

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance
  distanceCm = duration * SOUND_VELOCITY / 2;

  // Prints the distance on the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
}

void getDHT() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  hum = dht.readHumidity();
  // Read temperature as Celsius (the default)
  temp = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(hum) || isnan(temp)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print(F("Humidity: "));
  Serial.print(hum);
  Serial.print(F("%  Temperature: "));
  Serial.print(temp);
}
