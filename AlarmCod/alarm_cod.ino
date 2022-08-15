#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#define WLAN_SSID       "*****"
#define WLAN_PASS       "*****"
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "*****"
#define AIO_KEY         "*****"

int rele_01 = 16;
int alarmeNivel_01 = 5;
int alarmeNivel_02 = 4;
int rele_02 = 0;

int LED = 2;

int alarmeNivel_03 = 14;
int alarmeNivel_04 = 12;
int rele_03 = 13;
int rele_04 = 15;

unsigned long contador;
unsigned long contadorErro;

bool flagPublicado;
bool flagErroPublicado;

int alarme_01;
int alarme_02;
int alarme_03;
int alarme_04;

int tempoPublicacaoAlarme_01 = 10;
int tempoPublicacaoAlarme_02 = 8;
int tempoPublicacaoAlarme_03 = 6;
int tempoPublicacaoAlarme_04 = 4;
int tempoPublicacaoNivelNormal = 12;
int tempoPublicacaoErroSensor = 20;

int alarme;
int oldAlarme;

int alarmeSensor;
int oldAlarmeSensor;

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish Alarme = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/alarme");
Adafruit_MQTT_Publish Sensor = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/sensor");
void MQTT_connect();

// ----------------------------------------------------------------------------
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ----------------------------------------------------------------------------

void setup() {

  flagPublicado = false;
  flagErroPublicado = false;
  contador = 0;
  contadorErro = 0;
  alarme = 0;
  oldAlarme = 0;
  
  alarmeSensor =0;
  oldAlarmeSensor =0;
  
  Serial.begin(115200);

  pinMode(alarmeNivel_01, INPUT);
  pinMode(alarmeNivel_02, INPUT);
  pinMode(alarmeNivel_03, INPUT);
  pinMode(alarmeNivel_04, INPUT);

  pinMode(LED, OUTPUT);

  pinMode(rele_01, OUTPUT);
  pinMode(rele_02, OUTPUT);
  pinMode(rele_03, OUTPUT);
  pinMode(rele_04, OUTPUT);

  digitalWrite(rele_01, LOW);
  digitalWrite(rele_02, LOW);
  digitalWrite(rele_03, LOW);
  digitalWrite(rele_04, LOW);

  delay(10);
  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  digitalWrite(LED, HIGH);  // Turn the LED off by making the voltage HIGH
  Serial.println();
  delay(1000);
  digitalWrite(LED, HIGH);  // Turn the LED off by making the voltage HIGH
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

}

// ----------------------------------------------------------------------------
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ----------------------------------------------------------------------------

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  delay(1000);                      // Wait for a second
  digitalWrite(LED, HIGH);          // Turn the LED off by making the voltage HIGH
  delay(1000);                      // Wait for two seconds (to demonstrate the active low LED)
  digitalWrite(LED, LOW);           // Turn the LED on (Note that LOW is the voltage level

  alarme_01 = digitalRead(alarmeNivel_01);
  alarme_02 = digitalRead(alarmeNivel_02);
  alarme_03 = digitalRead(alarmeNivel_03);
  alarme_04 = digitalRead(alarmeNivel_04);

  Serial.println(String("Contador: ") + contador);

  alarme = 0;
  alarmeSensor = 0;

  if (alarme_01) { alarme = 1; }
  if (alarme_02) { alarme = 2; }
  if (alarme_03) { alarme = 3; }
  if (alarme_04) { alarme = 4; }
  
  if (alarme == 1)
  {
    ligaDesligaLeds(LOW, HIGH, HIGH, HIGH);
    publicaNoTempo(alarme, tempoPublicacaoAlarme_01, "Alarme 01");
  }
  
  if (alarme == 2)
  {
    ligaDesligaLeds(HIGH, LOW, HIGH, HIGH);
    publicaNoTempo(alarme, tempoPublicacaoAlarme_02, "Alarme 02");
    alarmeSensor = 0;
    if (! alarme_01) { alarmeSensor = alarmeSensor +   1; }
  }
  if (alarme == 3)
  {
    ligaDesligaLeds(HIGH, HIGH, LOW, HIGH);
    publicaNoTempo(alarme, tempoPublicacaoAlarme_03, "Alarme 03");
    alarmeSensor = 0;
    if (! alarme_01) { alarmeSensor = alarmeSensor +   1; }
    if (! alarme_02) { alarmeSensor = alarmeSensor +  10; }
  }
  
  if (alarme == 4)
  {
    ligaDesligaLeds(HIGH, HIGH, HIGH, LOW);
    publicaNoTempo(alarme, tempoPublicacaoAlarme_04, "Alarme 04");
    alarmeSensor = 0;
    if (! alarme_01) { alarmeSensor = alarmeSensor +   1; }
    if (! alarme_02) { alarmeSensor = alarmeSensor +  10; }
    if (! alarme_03) { alarmeSensor = alarmeSensor + 100; }
  }

  if (alarme == 0)
  {
    ligaDesligaLeds(HIGH, HIGH, HIGH, HIGH);
    publicaNoTempo(alarme, tempoPublicacaoNivelNormal, String("Nível NORMAL"));
  }

  if (alarmeSensor > 0)
  {
    Serial.println(String("ERRO DOS SENSORES - CHECAR: ") + alarmeSensor);
    if (! flagErroPublicado)
    {
      Serial.println(String("ERRO DOS SENSORES - PUBLICADO: ") + alarmeSensor);
      publicaNoTempoSensor(alarmeSensor, tempoPublicacaoErroSensor, "Alarme do Sensor");
      flagErroPublicado = true;
      contadorErro = 0;
    }
    else
    {
      if (contadorErro > tempoPublicacaoErroSensor)
      {
        flagErroPublicado = false;
      }
    }
    contadorErro += 1;
    Serial.println(String("Contador de Erro do Sensor: ") + contadorErro);
  }
  
  if (oldAlarme != alarme)
  {
    oldAlarme = alarme;
    flagPublicado = false;
  }
  
  if (oldAlarmeSensor != alarmeSensor)
  {
    oldAlarmeSensor = alarmeSensor;
    flagErroPublicado = false;
  }

  Serial.println(String("Nível de Alarme Geral: ") + alarme);
  contador += 1;
  if (contador > 2147483640) { contador = 0; }
  if (contadorErro > 2147483640) { contadorErro = 0; }
}

// ----------------------------------------------------------------------------
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ----------------------------------------------------------------------------

void ligaDesligaLeds(int LED1, int LED2, int LED3, int LED4)
{
    digitalWrite(rele_01, LED1);
    digitalWrite(rele_02, LED2);
    digitalWrite(rele_03, LED3);
    digitalWrite(rele_04, LED4);
}

// ----------------------------------------------------------------------------
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ----------------------------------------------------------------------------

void publicaNoTempo(int alarmeAtivo, long tempoEntreAlarmes, String strTexto)
{
  char auxTxt[5];
  itoa(alarmeAtivo, auxTxt, 10);
  Serial.println(String(strTexto + " - ATIVADO: ") + auxTxt);
  if (! flagPublicado)
  {
    Serial.println(String(strTexto + " - PUBLICADO: ") + auxTxt);
    publicar_alarme(alarmeAtivo);
    flagPublicado = true;
    contador = 0;
  }
  else
  {
    if (contador >= tempoEntreAlarmes)
    {
      flagPublicado = false;
    }
  }
}
// ----------------------------------------------------------------------------
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ----------------------------------------------------------------------------

void publicaNoTempoSensor(int alarmeSensor, long tempoEntreAlarmes, String strTexto)
{
  char auxTxt[5];
  itoa(alarmeSensor, auxTxt, 10);
  Serial.println(String(strTexto + " - ATIVADO: ") + auxTxt);
  if (! flagErroPublicado)
  {
    Serial.println(String(strTexto + " - PUBLICADO: ") + auxTxt);
    publicar_alarmeSensor(alarmeSensor);
    flagErroPublicado = true;
    contador = 0;
  }
  else
  {
    if (contador >= tempoEntreAlarmes)
    {
      flagErroPublicado = false;
    }
  }
}
// ----------------------------------------------------------------------------
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ----------------------------------------------------------------------------

void publicar_alarme(int alarme) {
  if (! Alarme.publish(alarme))
  {
    Serial.println(F("Failed"));
  }
  else
  {
    Serial.println(F("OK!"));
  }
}

// ----------------------------------------------------------------------------
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ----------------------------------------------------------------------------

void publicar_alarmeSensor(int alarme) {
  if (! Sensor.publish(alarme))
  {
    Serial.println(F("Failed"));
  }
  else
  {
    Serial.println(F("OK!"));
  }
}

// ----------------------------------------------------------------------------
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ----------------------------------------------------------------------------

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected())
  {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0)
  { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(1000);  // wait 5 seconds
       retries--;
       if (retries == 0)
       {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
