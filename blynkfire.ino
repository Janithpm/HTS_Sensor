
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <FirebaseArduino.h>

#include <WiFiUdp.h>
#include <NTPClient.h>

#define DHTPIN D2
#define SMSPIN A0
#define RELAYPIN D7
#define DHTTYPE DHT11
#define LIMIT 10

#define FIREBASE_HOST "firebase host here"
#define FIREBASE_AUTH "firebase secret key here"
#define WIFI_SSID "wifi ssid here"
#define WIFI_PASSWORD "wifi password here"

char token[] = "blnyk token here";

float t, ft, h;
int sm;
String DEVICE_AUTH_KEY = "DIV001";
const long UTCOffset = 19800;

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org",UTCOffset);

void sendData()
{
   timeClient.update();
  String dateTime = timeClient.getFullFormattedTime();
 
  t = dht.readTemperature();
  ft = dht.readTemperature(true);
  h = dht.readHumidity();

  if (isnan(h) || isnan(t) || isnan(ft))
  {
    Serial.println(F("Failed to read from DHT sensor"));
    return;
  }
  send_data("dateTime", dateTime);
  send_data("tempC", t );
  send_data("tempF", ft);
  send_data("hum", h);
  Blynk.virtualWrite(V2, t);
  Blynk.virtualWrite(V3, ft);
  Blynk.virtualWrite(V4, h);

  sm = analogRead(SMSPIN);
  if (sm < 0)
  {
    Serial.println(F("Failed to read from Soil Moisture sensor"));
    return;
  }
  else
  {
    sm = map(sm, 0, 1023, 100, 0);
    send_data("sm", sm);
    Blynk.virtualWrite(V5, sm);
    if (sm < LIMIT)
    {
      digitalWrite(RELAYPIN, 0);
      Blynk.notify("WARNNIG : Soil Moisture level is too low.");
    }
    else
    {
      digitalWrite(RELAYPIN, 1);
    }
  }

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F(" %\t  Temperature: "));
  Serial.print(t);
  Serial.print(" C\t Soil Moisture : ");
  Serial.print(sm);
  Serial.println(" %");
}

void send_data(String key, float value){
  String temp = DEVICE_AUTH_KEY + "/" + key;
  Firebase.setFloat(temp, value);
  if(Firebase.failed()){
    Serial.println(Firebase.error());
    return;
  }
}

void send_data(String key, String value){
  String temp = DEVICE_AUTH_KEY + "/" + key;
  Firebase.setString(temp, value);
  if(Firebase.failed()){
    Serial.println(Firebase.error());
    return;
  }
}



void setup()
{
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  dht.begin();
  timeClient.begin();
  pinMode(RELAYPIN, OUTPUT);

  Blynk.begin(token, WIFI_SSID, WIFI_PASSWORD);
  timer.setInterval(5000L, sendData);
}

void loop()
{
  Blynk.run();
  timer.run();
}
