//librerias y definiciones
#include <Ubidots.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <WiFi.h>

//declaracion de variables
#define DHTTYPE DHT11
#define TOKEN "BBFF-2Y0drBPmuIgktt1rqr6wrMAv5u3rqd"
#define DEVICE "esp-32"
#define leerTemp "leertemperatura"
#define leerMov "movimiento"
#define leerHum "leerhumedad"
#define escribirTemp "escribirtemperatura"
#define escribirHum "escribirhumedadlimite"
#define prenderVent "prenderventilador"

//coneccion con ubidots mediante un token
Ubidots client(TOKEN);

//nombre del wifi y contraseña para conectarse
const char* WIFISSID = "WifiDylan";
const char* PASSWORD = "Dylan212508632";

//declaracion de pines
const int sensorPir = 23;
const int sensorTemp = 22;
const int ledVerde = 18;
const int ledRojo = 19;
const int ledAzul = 0;
const int motor = 15;
const int buzzer = 2;

//declaracion de variables para la temperatura y humedad
int valorMov, tLim, vent, hLim, h;
float t;

//inicio del sensor de temperatura y humedad DHT11 en el pin 22
DHT dht(sensorTemp, DHTTYPE);

void setup() {
  //al arrancar el circuito, iniciarà primero este void setup una sola vez
  //modos de pin para cada componente
  pinMode(sensorPir, INPUT);
  pinMode(ledVerde, OUTPUT);
  pinMode(ledRojo, OUTPUT);
  pinMode(ledAzul, OUTPUT);
  pinMode(motor, OUTPUT);
  pinMode(buzzer, OUTPUT);
  dht.begin();

  //para poder ver la terminal y los datos que entrega
  Serial.begin(9600);

  //conexion al wifi
  if (client.wifiConnect(WIFISSID, PASSWORD)) {
    colorAmarillo();
    delay(500);
    apagar();
    delay(500);
    colorAmarillo();
    delay(500);
    apagar();
    delay(500);
    colorAmarillo();
    delay(500);
    apagar();
    delay(500);
  }
}


void loop() {
  //luego del void setup, se ejecutara esto en un ciclo
  vent = client.get(DEVICE, prenderVent);  //toma el valor del switch (encendido o apagado) para activar el ventilador
  escribirTyHLim();                        //toma los valores que se ingresaron en ubidots para la temperatura y humedad maxima para que se active el ventilador
  escanearTyH();                           //toma la temperatura y humedad que hay en el ambiente mediente el DHT11
  leerTyH();                               //almacena la temperatura y humedad leida por el DHT11 para despues enviarla a la pagina de ubidots
  client.send();                           //envia los datos a la pagina

  //si el switch esta encendido, el ventilador se activa hasta que se apague, a su vez, envia los datos a la pagina
  if (vent == HIGH) {
    while (vent == HIGH) {
      vent = client.get(DEVICE, prenderVent);
      escribirTyHLim();
      escanearTyH();
      delay(500);
      leerTyH();
      client.send();
      digitalWrite(motor, HIGH);
      if (vent == LOW) {
        apagar();
      }
    }
  }

  //si detecta movimiento el sensor, este se activa hasta que no detecte nada, a su vez, envia los datos a la pagina y enciende la luz roja del rgb
  valorMov = digitalRead(sensorPir);
  if (valorMov == HIGH) {
    while (valorMov == HIGH) {
      valorMov = digitalRead(sensorPir);
      escribirTyHLim();
      escanearTyH();
      delay(500);
      encenderRojo();
      client.add(leerMov, 1);
      leerTyH();
      client.send();
      digitalWrite(motor, HIGH);
      if (valorMov == LOW) {
        apagar();
        client.add(leerMov, 0);
        client.send();
      }
    }
  }

  // si la temperatura supera la temperatura asignada en el ubidots, este se activa hasta que la temperatura baje, envia los datos a la pagina y enciende la luz verde del rgb
  if (t >= tLim) {
    while (t >= tLim) {
      escribirTyHLim();
      escanearTyH();
      delay(500);
      encenderVerde();
      leerTyH();
      client.send();
      digitalWrite(motor, HIGH);
      if (t < tLim) {
        apagar();
      }
    }
  }

  //si la humedad supera la humedad ingresada en el ubidots, este se activa hasta que la humedad baje, envia los datos a la pagina y enciende la luz azul del rgb
  if (h >= hLim) {
    while (h >= hLim) {
      escribirTyHLim();
      escanearTyH();
      encenderAzul();
      delay(500);
      leerTyH();
      client.send();
      digitalWrite(motor, HIGH);
      if (h < hLim) {
        apagar();
      }
    }
  }
}

void apagar() {
  delay(1000);
  digitalWrite(ledRojo, LOW);
  digitalWrite(ledAzul, LOW);
  digitalWrite(ledVerde, LOW);
  digitalWrite(motor, LOW);
}

void encenderRojo() {
  digitalWrite(ledRojo, HIGH);
  digitalWrite(ledAzul, LOW);
  digitalWrite(ledVerde, LOW);
}

void encenderVerde() {
  digitalWrite(ledRojo, LOW);
  digitalWrite(ledAzul, LOW);
  digitalWrite(ledVerde, HIGH);
}

void encenderAzul() {
  digitalWrite(ledRojo, LOW);
  digitalWrite(ledAzul, HIGH);
  digitalWrite(ledVerde, LOW);
}

void leerTyH() {
  client.add(leerTemp, t);
  client.add(leerHum, h);
}

void escribirTyHLim() {
  tLim = client.get(DEVICE, escribirTemp);
  hLim = client.get(DEVICE, escribirHum);
}

void escanearTyH() {
  t = dht.readTemperature();
  h = dht.readHumidity();
}

void colorAmarillo() {
  digitalWrite(ledRojo, LOW);
  digitalWrite(ledAzul, HIGH);
  digitalWrite(ledVerde, HIGH);
}