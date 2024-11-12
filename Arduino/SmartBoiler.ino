#include <DallasTemperature.h>
#include <OneWire.h>
#include <SoftwareSerial.h>

// ----------------------- Constantes -----------------------------
#define oneWirePin 12

// Valores de inicialización
#define BAUD_RATE 9600

// Valores lógicos pulsador
#define ENCENDER HIGH
#define APAGAR LOW

// Número de pin
#define PIN_LAMPARA 7
#define PIN_RGB_ROJO 6
#define PIN_RGB_VERDE 5
#define PIN_RGB_AZUL 3
#define PIN_PULSADOR 2
#define PIN_POTENCIOMETRO A4
#define PIN_SENSOR_TEMPERATURA A5
#define PIN_DISTANCIA_ECHO 9
#define PIN_DISTANCIA_TRIGGER 10
#define PIN_BT_TX 8
#define PIN_BT_RX 13

// Valores potenciómetro
#define MIN_VALOR_POTENCIOMETRO 0
#define MAX_VALOR_POTENCIOMETRO 1023

// Valores temperatura
#define MIN_TEMPERATURA 10
#define MAX_TEMPERATURA 99
#define FACTOR_ESCALA_TEMPERATURA 0.49
#define PUNTO_REFERENCIA_TEMPERATURA 50
#define PORCENTAJE_PERDIDA_TEMPERATURA 0.1
#define DECENA 10
#define TEMP_INICIAL_POTENCIOMETRO -1
#define CONVERTIR_ASCII_DECIMAL 48
#define VARIACION_RANGO_TEMPERATURA 1
#define INDICE_SENSOR_TEMP_LEER 0

// Valores RGB
#define RGB_LOW 0
#define RGB_HIGH 255

// Valores distancia
#define DELAY_LIMPIEZA_SENSOR_DISTANCIA 2
#define DELAY_TRIGGER_SENSOR_DISTANCIA 10
#define FACTOR_CONVERSION_DISTANCIA_CM 0.01723
#define DISTANCIA_AGUA_SUFICIENTE 10
#define CARACTER_SENIAL_DISTANCIA_SP 'D'
#define DISTANCIA_POR_DEFECTO 100

// Seniales Bluetooth
#define PRENDER_DESDE_BT 'E'
#define APAGAR_DESDE_BT 'A'
#define BT_TEMP 'T'

// -------------------- Variables globales ---------------------
int temperaturaDeseada;
int temperaturaActual;
int temperaturaAnteriorPotenciometro;
int temperaturaActualPotenciometro;
int distanciaActual;

// ------------------- Estados --------------------------------
enum estados
{
    ESTADO_PREPARADO,
    ESTADO_NO_PREPARADO,
    ESTADO_CALENTANDO,
    ESTADO_ESPERANDO
} estadoActual;

// ------------------ Eventos -----------------------------
enum eventos
{
    EVENTO_CONTINUE,
    EVENTO_ON,
    EVENTO_OFF,
    EVENTO_AGUA_SUFICIENTE,
    EVENTO_AGUA_INSUFICIENTE,
    EVENTO_TEMPERATURA_DESEADA_ALCANZADA,
    EVENTO_PERDIDA_TEMPERATURA_DESEADA,
    EVENTO_CALENTANDO,
    EVENTO_APAGAR
} eventoNuevo;
int indiceEvento = 0;
const int CANTIDAD_SENSORES_VERIFICAR = 5;
void verificarEstadoSensorPulsador();
void verificarDistancia();
void verificarEstadoSensorPotenciometroYTemperatura();
void verificarPerdidaTemperatura();
void verificarSenialBT();
void (*verificarSensor[CANTIDAD_SENSORES_VERIFICAR])() = {verificarEstadoSensorPulsador, verificarDistancia, verificarEstadoSensorPotenciometroYTemperatura, verificarPerdidaTemperatura, verificarSenialBT};

// --------------------- Configuración de librerias -----------------
OneWire oneWireBus(oneWirePin);
DallasTemperature sensor(&oneWireBus);
SoftwareSerial serialBT (PIN_BT_TX, PIN_BT_RX);

// ------------------------------------------------------------------
void setup()
{
    inicializacion();
    sensor.begin();
}

void loop()
{
    leerEventos();
    maquinaDeEstado();
}

void inicializacion()
{
    serialBT.begin(BAUD_RATE);
    Serial.begin(BAUD_RATE);
    pinMode(PIN_PULSADOR, INPUT);
    pinMode(PIN_SENSOR_TEMPERATURA, INPUT);
    pinMode(PIN_POTENCIOMETRO, INPUT);
    pinMode(PIN_DISTANCIA_ECHO, INPUT);
    pinMode(PIN_LAMPARA, OUTPUT);
    pinMode(PIN_RGB_AZUL, OUTPUT);
    pinMode(PIN_RGB_VERDE, OUTPUT);
    pinMode(PIN_RGB_ROJO, OUTPUT);
    pinMode(PIN_DISTANCIA_TRIGGER, OUTPUT);
    estadoActual = ESTADO_NO_PREPARADO;
    temperaturaAnteriorPotenciometro = TEMP_INICIAL_POTENCIOMETRO;
    distanciaActual = DISTANCIA_POR_DEFECTO;
}

void maquinaDeEstado ()
{
    switch (estadoActual)
    {
        case ESTADO_NO_PREPARADO:
            switch(eventoNuevo)
            {
                case EVENTO_ON:
                    notificarAguaInsuficiente();
                    estadoActual = ESTADO_NO_PREPARADO;
                    break;
                case EVENTO_AGUA_SUFICIENTE:
                    prenderRGBAzul();
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
            switch(eventoNuevo)
            {
                case EVENTO_ON:
                    encenderCalentador();
                    regularIntensidadRGBRojo();
                    estadoActual = ESTADO_CALENTANDO;
                    break;
                case EVENTO_AGUA_INSUFICIENTE:
                    apagarRGB();
                    estadoActual = ESTADO_NO_PREPARADO;
                    break;
                case EVENTO_CONTINUE:
                    estadoActual = ESTADO_PREPARADO;
                    break;
                default:
                    break;
            }
            break;
        case ESTADO_CALENTANDO:
            switch(eventoNuevo)
            {
                case EVENTO_OFF:
                    apagarCalentador();
                    prenderRGBAzul();
                    notificarApagarCalentador();
                    estadoActual = ESTADO_PREPARADO;
                    break;
                case EVENTO_AGUA_INSUFICIENTE:
                    apagarCalentador();
                    apagarRGB();
                    estadoActual = ESTADO_NO_PREPARADO;
                    break;
                case EVENTO_CALENTANDO:
                    regularIntensidadRGBRojo();
                    notificarTemperaturaActualYDeseada();
                    estadoActual = ESTADO_CALENTANDO;
                    break;
                case EVENTO_TEMPERATURA_DESEADA_ALCANZADA:
                    notificarTemperaturaDeseadaAlcanzada();
                    apagarCalentador();
                    prenderRGBVerde();
                    estadoActual = ESTADO_ESPERANDO;
                    break;
                default:
                    break;
            }
            break;
        case ESTADO_ESPERANDO:
            switch(eventoNuevo)
            {
                case EVENTO_OFF:
                    apagarCalentador();
                    prenderRGBAzul();
                    estadoActual = ESTADO_PREPARADO;
                    break;
                case EVENTO_PERDIDA_TEMPERATURA_DESEADA:
                    encenderCalentador();
                    regularIntensidadRGBRojo();
                    estadoActual = ESTADO_CALENTANDO;
                    break;
                case EVENTO_AGUA_INSUFICIENTE:
                    apagarCalentador();
                    apagarRGB();
		    estadoActual = ESTADO_NO_PREPARADO;
                    break;
                case EVENTO_CONTINUE:
                    estadoActual = ESTADO_ESPERANDO;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

// ------------------------- Captura de eventos ------------------------------------
void leerEventos()
{
        verificarSensor[indiceEvento]();
        indiceEvento = ++indiceEvento % CANTIDAD_SENSORES_VERIFICAR;
}

void verificarEstadoSensorPulsador()
{
    int valorLogicoPulsador = digitalRead(PIN_PULSADOR);

    if (valorLogicoPulsador == HIGH && (estadoActual == ESTADO_CALENTANDO || estadoActual == ESTADO_ESPERANDO))
    {
      eventoNuevo = EVENTO_OFF;
    }
    else if(valorLogicoPulsador == HIGH && (estadoActual == ESTADO_NO_PREPARADO || estadoActual == ESTADO_PREPARADO))
      eventoNuevo = EVENTO_ON;
    else
      eventoNuevo = EVENTO_CONTINUE;
}

void verificarEstadoSensorPotenciometroYTemperatura()
{
    temperaturaActualPotenciometro = leerTemperaturaPotenciometro();
    if(temperaturaActualPotenciometro > temperaturaAnteriorPotenciometro + VARIACION_RANGO_TEMPERATURA || temperaturaActualPotenciometro < temperaturaAnteriorPotenciometro - VARIACION_RANGO_TEMPERATURA)
    {
        temperaturaDeseada = temperaturaActualPotenciometro;
        temperaturaAnteriorPotenciometro = temperaturaActualPotenciometro;
    }
    temperaturaActual = leerValorSensorTemperatura();

    if (temperaturaActual >= temperaturaDeseada)
        eventoNuevo = EVENTO_TEMPERATURA_DESEADA_ALCANZADA;
    else
        eventoNuevo = EVENTO_CALENTANDO;
}

void verificarDistancia()
{
    int distanciaEnCM = calcularDistanciaCM();
    if(distanciaEnCM <= DISTANCIA_AGUA_SUFICIENTE)
        eventoNuevo = EVENTO_AGUA_SUFICIENTE;
    else
        eventoNuevo = EVENTO_AGUA_INSUFICIENTE;
}

void verificarPerdidaTemperatura()
{
    temperaturaActual = leerValorSensorTemperatura();

    if (temperaturaActual <= (temperaturaDeseada - (temperaturaDeseada * PORCENTAJE_PERDIDA_TEMPERATURA)))
        eventoNuevo = EVENTO_PERDIDA_TEMPERATURA_DESEADA;
    else
        eventoNuevo = EVENTO_CONTINUE;
}

void verificarSenialBT()
{
    if (serialBT.available())
    {
        char comandoBT = serialBT.read();

        switch(comandoBT)
        {
            case PRENDER_DESDE_BT:
              eventoNuevo = EVENTO_ON;
              break;
            case APAGAR_DESDE_BT:
              eventoNuevo = EVENTO_OFF;
              break;
            case BT_TEMP:
              	temperaturaDeseada = serialBT.parseInt();
              	break;
            default:
                eventoNuevo = EVENTO_CONTINUE;
            break;
        }
    }
    else
      eventoNuevo = EVENTO_CONTINUE;
}

// ------------- Funciones auxiliares de captura de eventos ----------------------

int calcularDistanciaCM ()
{
  if (Serial.available())
  {
    char comandoSP = Serial.read();

    if (comandoSP == CARACTER_SENIAL_DISTANCIA_SP)
    {
       distanciaActual = Serial.parseInt();
    }
  }

  return distanciaActual;
}


float leerTemperaturaPotenciometro()
{
    return convertirValorPotenciometroACelsius(analogRead(PIN_POTENCIOMETRO));
}

float convertirValorPotenciometroACelsius(float valor)
{
    return map(valor, MIN_VALOR_POTENCIOMETRO, MAX_VALOR_POTENCIOMETRO, MIN_TEMPERATURA, MAX_TEMPERATURA);
}

float leerValorSensorTemperatura()
{
  sensor.requestTemperatures();
  return sensor.getTempCByIndex(INDICE_SENSOR_TEMP_LEER);
}

float convertirValorSensorTemperaturaACelsius(float valorTemp)
{
    return ((valorTemp * FACTOR_ESCALA_TEMPERATURA) - PUNTO_REFERENCIA_TEMPERATURA);
}

// -------------- Funciones de acciones actuadores ----------------
void encenderCalentador()
{
    cambiarEstadoCalentador(ENCENDER);
}

void apagarCalentador()
{
    cambiarEstadoCalentador(APAGAR);
}

void cambiarEstadoCalentador(int estado)
{
    digitalWrite(PIN_LAMPARA, estado);
}

void prenderRGBVerde()
{
    digitalWrite(PIN_RGB_ROJO, RGB_LOW);
    digitalWrite(PIN_RGB_AZUL, RGB_LOW);
    digitalWrite(PIN_RGB_VERDE, RGB_HIGH);
}

void apagarRGB()
{
    digitalWrite(PIN_RGB_ROJO, RGB_LOW);
    digitalWrite(PIN_RGB_VERDE, RGB_LOW);
    digitalWrite(PIN_RGB_AZUL, RGB_LOW);
}

void prenderRGBAzul()
{
    digitalWrite(PIN_RGB_ROJO, RGB_LOW);
    digitalWrite(PIN_RGB_VERDE, RGB_LOW);
    digitalWrite(PIN_RGB_AZUL, RGB_HIGH);
}

int definirIntensidadColor()
{
    return map(temperaturaActual, MIN_TEMPERATURA, temperaturaDeseada, RGB_LOW, RGB_HIGH);
}

void regularIntensidadRGBRojo()
{
    const int intensidadRojo = definirIntensidadColor();
    analogWrite(PIN_RGB_ROJO, intensidadRojo);
    digitalWrite(PIN_RGB_VERDE, RGB_LOW);
    digitalWrite(PIN_RGB_AZUL, RGB_LOW);
}

//---------------------------- Notificación por Bluetooth --------------------------
void notificarAguaInsuficiente ()
{
    serialBT.println("Agua insuficiente");
}

void notificarApagarCalentador ()
{
    serialBT.println("Apagar calendator");
}

void notificarTemperaturaActualYDeseada ()
{
    serialBT.println(String(temperaturaActual) + "°/" + String(temperaturaDeseada));
}

void notificarTemperaturaDeseadaAlcanzada ()
{
    serialBT.println("Temperatura deseada alcanzada");
}
