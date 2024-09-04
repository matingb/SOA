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
	fsm();
}

void fsm() {
  leerEventos();

  /*switch(estadoActual) {
    case ESTADO_NO_PREPARADO:
        switch(evento) {
            case EVENTO_ENCENDER:
                enviarNotificacionError();
                estadoActual = ESTADO_NO_PREPARADO;
                break;
            case EVENTO_AGUA_SUFICIENTE:
                estadoActual = ESTADO_PREPARADO;
                break;
            case EVENTO_CONTINUE:
                estadoActual = ESTADO_NO_PREPARADO;
                break;
            default:
                break;
        }
        break;
    case ESTADO_PREPARADO:
        switch(evento)
            case EVENTO_ENCENDER:
                encenderCalentador();
                regularIntensidadRGBRojo();
                estadoActual = ESTADO_CALENTANDO;
                break;
            case EVENTO_AGUA_NO_SUFICIENTE:
                estadoActual = ESTADO_NO_PREPARADO;
                break;
            case EVENTO_CONTINUE:
                estadoActual = ESTADO_PREPARADO;
                break;
            default:
                break;
    case ESTADO_CALENTANDO:
        switch(evento)
            case EVENTO_TEMPERATURA_DESEADA_ALCANZADA:
                apagarCalentador();
                enviarNotificacion();
                prenderRBGVerde();
                estadoActual = ESTADO_FINALIZADO;
                break;
            case EVENTO_APAGAR:
                apagarCalentador();
                apagarRGB();
                estadoActual = ESTADO_NO_PREPARADO;
            case EVENTO_CONTINUE:
                regularIntensidadRGBRojo();
                estadoActual = ESTADO_CALENTANDO;
                break;
            default:
                break;
    case ESTADO_FINALIZADO:
        switch(evento)
            case EVENTO_PERDIDA_TEMPERATURA_DESEADA:
                encenderCalentador();
                regularIntensidadRGBRojo();
                estadoActual = ESTADO_CALENTANDO;
                break;
            case EVENTO_APAGAR:
                apagarCalentador();
                apagarRGB();
                estadoActual = ESTADO_NO_PREPARADO;
                break;
            case EVENTO_CONTINUE:
                estadoActual = ESTADO_FINALIZADO;
                break;
            default:
                break;
  }*/
}

void leerEventos() {
  leerEncendido();
  leerTemperatura();

  if(boton == HIGH) {
    encenderCalentador();
  }

  if(temperatura > temperaturaEsperada) {
    apagarCalentador();
  }
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
