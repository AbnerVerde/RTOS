/*
Materia: RTOS

Equipo:
Abner Kalid Verde Padilla 
Diego Perez Dominguez 


-Este codigo se relaizo con el uso de RTOS en una esp32 

Se planea usar timer para poder hacer un semaforo un una secuencia donde el led verde va a encender por 200ms, luego se apaga y enciende el 
led amarillo parpadeando con periodo de 40ms por 200ms, ara despues apagarse y enecenderece el led rojo por 200ms complentando el ciclo al 
apagrse y volviendo a iniciar 

Por lo tanto se propone un timer de 200ms este sera nuestra base para poder controlar todo el encendido de nuestros leds, sin embargo
ocupamos un timer secundario para el parpadeo del led amarillo, ademas de los timers, es necesario hacer uso de contadores que fungiran 
como banderas para poder determinar cuando se enciende y apaga un led para darle paso al siguiente por lo que se usan las variables 
contador rojo y contador amarillo para este trabajo 

El procedimiento comenzo con la creacion de ambos timers, Timer_Base(200ms) y Timer_Secundario(400ms) y para ambos timers se crearon 2 
tareas, tarea base en donde se contepla el encendido del led rojo y verde por medio del contador contadorRojo, el cual incrementa cada que 
el timer de 200ms se completa, una vez el contadorRojo alcance el valor de 1, se inicial la secuencia de la segunda tarea en donde el led rojo
se apaga y se enciende el amarillo, aumentando una unidad al contadorAmarillo cada vez que se completan 400ms, y por medio de operadores matematicos
determinamos si el led amarillo esta encendido o apagado, una vez completados 200ms el contador rojo aumenta en 1 y el contador amarillo regresa a 0
regresando a la tareaBase en donde ahora se prende el verde por otro periodo y se apaga para finalmente encender el led rojo y reiniciar el ciclo 


*/




#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

//declaracion de timers 

static TimerHandle_t Timer_Base= NULL;
static TimerHandle_t Timer_secundario = NULL;

//pines en la ESP 32

const int ledRojo = 5;
const int ledVerde = 18;
const int ledAmarillo = 19;

//Contadores

int contadorRojo = 0;
int contadorAmarillo = 0;

//tareas 

void TareaBase(TimerHandle_t xTimer) {
  contadorRojo++;

  if (contadorRojo == 0) {
    digitalWrite(ledRojo, HIGH);   // Encender rojo
    digitalWrite(ledVerde, LOW);
  }
  else if ((contadorRojo == 2)) {
    digitalWrite(ledRojo, LOW);    // Apagar rojo, encender verde
    digitalWrite(ledVerde, HIGH);
  }
  else if (contadorRojo == 3) {
    digitalWrite(ledRojo, HIGH);   // Rojo otra vez
    digitalWrite(ledVerde, LOW);
    contadorRojo = 0; // Reinicia ciclo
  }
}

void TareaSecundario(TimerHandle_t xTimer) {
  if (contadorRojo == 1) {
    digitalWrite(ledRojo, LOW); 
    contadorAmarillo++;
    if (contadorAmarillo % 2 == 0) {
      digitalWrite(ledAmarillo, HIGH);
    } else {
      digitalWrite(ledAmarillo, LOW);
    }
  } else {
    digitalWrite(ledAmarillo, LOW); // Apagado cuando no toca
    contadorAmarillo = 0;
  }
}

void setup() {
  pinMode(ledRojo, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  pinMode(ledAmarillo, OUTPUT);

  Serial.begin(115200);
//configuracion timers 
  Timer_Base= xTimerCreate(
    "Timer1",
    200 / portTICK_PERIOD_MS,
    pdTRUE,
    (void *)1,
    TareaBase
  );

  Timer_secundario = xTimerCreate(
    "Timer2",
    40 / portTICK_PERIOD_MS,
    pdTRUE,
    (void *)2,
    TareaSecundario
  );

  xTimerStart(Timer_Base, portMAX_DELAY);
  xTimerStart(Timer_secundario, portMAX_DELAY);
}

void loop() {

}

