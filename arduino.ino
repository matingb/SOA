// COLORES DE CABLES
// Rojo = positivo
// Negro = negativo
// Amarillo = Digital Input
// Azul = Digital Output
// Verde = Analog Input


const int ENCENDER = HIGH;
const int APAGAR = LOW;
const int LED = 2; 
const int PULSADOR = 12;
const int POTENCIOMETRO = A4;
const int SENSOR_TEMPERATURA = A5;
float temperaturaEsperada;
float temperatura;
int boton;


void setup() { 
  Serial.begin(9600);
  pinMode(PULSADOR,INPUT);
  pinMode(SENSOR_TEMPERATURA, INPUT);
  pinMode(LED, OUTPUT);
} 

void loop() {
  leerEventos();
  if(boton == HIGH) {
    encenderCalentador();
  }

  if(temperatura > temperaturaEsperada) {
    apagarCalentador();
  }
}

void leerEventos() {
  leerEncendido();
  leerTemperatura();
}

void leerEncendido() {
    boton = digitalRead(PULSADOR);
}

void leerTemperatura() {
  float tempPotenciometro=analogRead(POTENCIOMETRO);
  temperaturaEsperada = map(tempPotenciometro,0,1023,0,100);
  temperatura = (analogRead(SENSOR_TEMPERATURA)*(0.49))-50;
}

void encenderCalentador() {
	cambiarEstadoCalentador(ENCENDER);
}

void apagarCalentador() {
	cambiarEstadoCalentador(APAGAR);
}

void cambiarEstadoCalentador(int estado) {
	digitalWrite(LED, estado);
}
