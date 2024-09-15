// ----------------------- Constantes -----------------------------

// Valores de inicialización
#define BAUD_RATE 9600
// Valores lógicos pulsador
#define ENCENDER HIGH
#define APAGAR LOW
// Número de pin
#define PIN_LAMPARA 2
#define PIN_RGB_ROJO 3
#define PIN_RGB_VERDE 6
#define PIN_RGB_AZUL 5
#define PIN_PULSADOR 12
#define PIN_POTENCIOMETRO A4
#define PIN_SENSOR_TEMPERATURA A5
#define PIN_DISTANCIA_ECHO 9
#define PIN_DISTANCIA_TRIGGER 10
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
// Valores RGB
#define RGB_LOW 0
#define RGB_HIGH 255
// Valores sensor distancia
#define DELAY_LIMPIEZA_SENSOR_DISTANCIA 2
#define DELAY_TRIGGER_SENSOR_DISTANCIA 10
#define FACTOR_CONVERSION_DISTANCIA_CM 0.01723
#define DISTANCIA_AGUA_SUFICIENTE 10

// Senales Bluetooth
#define BT_ON_OFF 'O'
#define BT_TEMP 'T'

// -------------------- Variables globales ---------------------
int temperaturaDeseada;
int temperaturaActual;
int temperaturaAnteriorPotenciometro;
int temperaturaActualPotenciometro;

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
    EVENTO_ON_OFF,
    EVENTO_AGUA_SUFICIENTE,
    EVENTO_AGUA_INSUFICIENTE,
    EVENTO_TEMPERATURA_DESEADA_ALCANZADA,
    EVENTO_PERDIDA_TEMPERATURA_DESEADA,
    EVENTO_CALENTANDO,
    EVENTO_APAGAR
} eventoNuevo;
int indiceEvento = 0;
const int CANTIDAD_EVENTOS_VERIFICAR = 5;
void (*verificarSensor[CANTIDAD_EVENTOS_VERIFICAR])() = {verificarEstadoSensorPulsador, verificarEstadoSensorDistancia, verificarEstadoSensorPotenciometroYTemperatura, verificarPerdidaTemperatura, verificarSenialBT};

void setup()
{
    inicializacion();
}

void loop()
{
    leerEventos();
    maquinaDeEstado();
}

void inicializacion()
{
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
}

void maquinaDeEstado ()
{
    switch (estadoActual)
    {
        case ESTADO_NO_PREPARADO:
            switch(eventoNuevo)
            {
                case EVENTO_ON_OFF:
                    notificarAguaInsuficiente();
                    estadoActual = ESTADO_NO_PREPARADO;
                    Serial.println("Estado: No preparado, Evento: Encendido agua insuficiente --> ESTADO_NO_PREPARADO");
                    break;
                case EVENTO_AGUA_SUFICIENTE:
                    prenderRGBAzul();
                    estadoActual = ESTADO_PREPARADO;
                    Serial.println("Estado: No preparado, Evento: Agua suficiente --> ESTADO_PREPARADO");
                    break;
                case EVENTO_CONTINUE:
                    Serial.println("Estado: No preparado, Evento: CONTINUE --> ESTADO_NO_PREPARADO");
                    estadoActual = ESTADO_NO_PREPARADO;
                    break;
                default:
                    break;
            }
            break;
        case ESTADO_PREPARADO:
            switch(eventoNuevo)
            {
                case EVENTO_ON_OFF:
                    encenderCalentador();
                    regularIntensidadRGBRojo();
                    estadoActual = ESTADO_CALENTANDO;
                    Serial.println("Estado: Preparado, Evento: Presionar pulsador --> ESTADO_CALENTANDO");
                    break;
                case EVENTO_AGUA_INSUFICIENTE:
                    apagarRGB();
                    estadoActual = ESTADO_NO_PREPARADO;
                    Serial.println("Estado: Preparado, Evento: Agua insuficiente --> ESTADO_NO_PREPARADO");
                    break;
                case EVENTO_CONTINUE:
                    Serial.println("Estado: preparado, Evento: CONTINUE --> ESTADO_PREPARADO");
                    estadoActual = ESTADO_PREPARADO;
                    break;
                default:
                    break;
            }
            break;
        case ESTADO_CALENTANDO:
            switch(eventoNuevo)
            {
                case EVENTO_ON_OFF:
                    apagarCalentador();
                    prenderRGBAzul();
                    estadoActual = ESTADO_PREPARADO;
                    Serial.println("Estado: Calentando, Evento: Presionar pulsador --> ESTADO_PREPARADO");
                    break;
                case EVENTO_AGUA_INSUFICIENTE:
                    apagarCalentador();
                    apagarRGB();
                    estadoActual = ESTADO_NO_PREPARADO;
                    Serial.println("Estado: Calentando, Evento: Presionar pulsador --> ESTADO_NO_PREPARADO");
                    break;
                case EVENTO_CALENTANDO:
                    regularIntensidadRGBRojo();
                    estadoActual = ESTADO_CALENTANDO;
                    Serial.println("Estado: Calentando, Evento: Calentando --> ESTADO_CALENTANDO");
                    break;
                case EVENTO_TEMPERATURA_DESEADA_ALCANZADA:
                    apagarCalentador();
                    prenderRGBVerde();
                    estadoActual = ESTADO_ESPERANDO;
                    Serial.println("Estado: Calentando, Evento: temperatura deseada --> ESTADO_ESPERANDO");
                    break;
                default:
                    break;
            }
            break;
        case ESTADO_ESPERANDO:
            switch(eventoNuevo)
            {
                case EVENTO_ON_OFF:
                    apagarCalentador();
                    prenderRGBAzul();
                    estadoActual = ESTADO_PREPARADO;
                    Serial.println("Estado: Esperando, Evento: Presionar pulsador --> ESTADO_PREPARADO");
                    break;
                case EVENTO_PERDIDA_TEMPERATURA_DESEADA:
                    encenderCalentador();
                    regularIntensidadRGBRojo();
                    estadoActual = ESTADO_CALENTANDO;
                    Serial.println("Estado: Esperando, Evento: Temperatura deseada --> ESTADO_CALENTANDO");
                    break;
                case EVENTO_AGUA_INSUFICIENTE:
                    apagarCalentador();
                    apagarRGB();
					estadoActual = ESTADO_NO_PREPARADO;
                    break;
                case EVENTO_CONTINUE:
                    Serial.println("Estado: Esperando, Evento: CONTINUE --> ESTADO_ESPERANDO");
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
        indiceEvento = ++indiceEvento % CANTIDAD_EVENTOS_VERIFICAR;
}

void verificarEstadoSensorPulsador()
{
    int valorLogicoPulsador = digitalRead(PIN_PULSADOR);

    if (valorLogicoPulsador == HIGH)
        eventoNuevo = EVENTO_ON_OFF;
    else
        eventoNuevo = EVENTO_CONTINUE;
}

void verificarEstadoSensorPotenciometroYTemperatura()
{
    temperaturaActualPotenciometro = leerTemperaturaPotenciometro();
    if(temperaturaActualPotenciometro != temperaturaAnteriorPotenciometro)
    {
        temperaturaDeseada = temperaturaActualPotenciometro;
        temperaturaAnteriorPotenciometro = temperaturaActualPotenciometro;
    }
    temperaturaActual = leerValorSensorTemperatura();
	Serial.println(temperaturaDeseada);
  
    if (temperaturaActual >= temperaturaDeseada)
        eventoNuevo = EVENTO_TEMPERATURA_DESEADA_ALCANZADA;
    else
        eventoNuevo = EVENTO_CALENTANDO;
}

void verificarEstadoSensorDistancia()
{
    float distanciaEnCM = calcularDistanciaCM();
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
    if (Serial.available() > 0)
    {
        char comandoBT = Serial.read();

        switch(comandoBT)
        {
            case BT_ON_OFF:
                eventoNuevo = EVENTO_ON_OFF;
            break;
            case BT_TEMP:
              	temperaturaDeseada = Serial.parseInt();
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

float calcularDistanciaCM ()
{
  digitalWrite(PIN_DISTANCIA_TRIGGER, APAGAR);
  delayMicroseconds(DELAY_LIMPIEZA_SENSOR_DISTANCIA);
  digitalWrite(PIN_DISTANCIA_TRIGGER, ENCENDER);
  delayMicroseconds(DELAY_TRIGGER_SENSOR_DISTANCIA);
  digitalWrite(PIN_DISTANCIA_TRIGGER, APAGAR);
  return (pulseIn(PIN_DISTANCIA_ECHO, ENCENDER) * FACTOR_CONVERSION_DISTANCIA_CM);
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
    return convertirValorSensorTemperaturaACelsius(analogRead(PIN_SENSOR_TEMPERATURA));
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
    Serial.println("Intensidad" + String(intensidadRojo));
    analogWrite(PIN_RGB_ROJO, intensidadRojo);
    digitalWrite(PIN_RGB_VERDE, RGB_LOW);
    digitalWrite(PIN_RGB_AZUL, RGB_LOW);
}

void notificarAguaInsuficiente ()
{
    Serial.println("Agua insuficiente");
}
