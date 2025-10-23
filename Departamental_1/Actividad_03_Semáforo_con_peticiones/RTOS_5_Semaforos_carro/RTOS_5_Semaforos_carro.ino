/*
Materia: RTOS

Equipo:
Abner Kalid Verde Padilla 
Diego Perez Dominguez 


-Este codigo se relaizo con el uso de RTOS en una esp32 

El problema propuesto son dos semaforos para poder darle el paso a un carro en una calle de doble sentido pero que solo puede ser usada por un 
carro a la vez por lo que se tienen dos leds (rojo y verde) y dos botones (pedir paso y liberar paso) que funjen como sensores.

Para la resolucion del problema se hace uso de semaforos, por lo que se inicia analizando que se van a ocupar unicamente dos semaforos 
semaforo1 y semaforo2 que serviran para que las tareas1 y 2 puedan entrar a la zona critica, por otro lado hay dos tareas, una para el 
semaforo1 (tareaSemaforo1) y otra para el semaforo2 (tareaSemaforo2) estas tareas van a partir de un estado inicial en donde ambos semaforos 
estan en rojo y dependiendo de que boton se presione para pedir el paso, una de las dos tareas entrara la zona critica y no saldra hasta que 
se libere el semaforo con el boton pulsador, esto aplica para ambas tareas.  

*/





#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

//Pines tarjeta ESP 32
const int led_rojo1   = 2;
const int led_verde1  = 4;
const int boton_ini1  = 32;
const int boton_reset1= 33;

const int led_rojo2   = 19;
const int led_verde2  = 18;
const int boton_ini2  = 34;
const int boton_reset2= 35;

// Semaforos
SemaphoreHandle_t semaforo1;
SemaphoreHandle_t semaforo2;

// Variables
bool estado1 = false;
bool estado2 = false;

// Tarea semaforo 1
void tareaSemaforo1(void *parameter) {
  while (1) {

    if (digitalRead(boton_ini1) == LOW && !estado1) {
      if (xSemaphoreTake(semaforo2, 0) == pdTRUE) { // Ocupa el paso
        Serial.println("Carro 1 solicita paso...");
        digitalWrite(led_verde1, HIGH);
        digitalWrite(led_rojo1, LOW);
        digitalWrite(led_verde2, LOW);
        digitalWrite(led_rojo2, HIGH);
        estado1 = true;
      }
    }

    // Botón de reinicio
    if (digitalRead(boton_reset1) == LOW && estado1) {
      Serial.println("Carro 1 libera paso...");
      digitalWrite(led_verde1, LOW);
      digitalWrite(led_rojo1, HIGH);
      digitalWrite(led_verde2, LOW);
      digitalWrite(led_rojo2, HIGH);
      estado1 = false;
      xSemaphoreGive(semaforo2); // Libera el paso
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// tarea 2 
void tareaSemaforo2(void *parameter) {
  while (1) {
    // Botón de inicio
    if (digitalRead(boton_ini2) == LOW && !estado2) {
      if (xSemaphoreTake(semaforo1, 0) == pdTRUE) { // Ocupa el paso
        Serial.println("Carro 2 solicita paso...");
        digitalWrite(led_verde2, HIGH);
        digitalWrite(led_rojo2, LOW);
        digitalWrite(led_verde1, LOW);
        digitalWrite(led_rojo1, HIGH);
        estado2 = true;
      }
    }

    // Botón de reinicio
    if (digitalRead(boton_reset2) == LOW && estado2) {
      Serial.println("Carro 2 libera paso...");
      digitalWrite(led_verde2, LOW);
      digitalWrite(led_rojo2, HIGH);
      digitalWrite(led_verde1, LOW);
      digitalWrite(led_rojo1, HIGH);
      estado2 = false;
      xSemaphoreGive(semaforo1); // Libera el paso
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}


void setup() {
  Serial.begin(115200);
  Serial.println("Sistema de semáforos iniciado...");

  // Configurar pines
  pinMode(led_rojo1, OUTPUT);
  pinMode(led_verde1, OUTPUT);
  pinMode(boton_ini1, INPUT_PULLUP);
  pinMode(boton_reset1, INPUT_PULLUP);

  pinMode(led_rojo2, OUTPUT);
  pinMode(led_verde2, OUTPUT);
  pinMode(boton_ini2, INPUT_PULLUP);
  pinMode(boton_reset2, INPUT_PULLUP);

  // Estado inicial
  digitalWrite(led_rojo1, HIGH);
  digitalWrite(led_verde1, LOW);
  digitalWrite(led_rojo2, HIGH);
  digitalWrite(led_verde2, LOW);

  // Crear semáforos
  semaforo1 = xSemaphoreCreateBinary();
  semaforo2 = xSemaphoreCreateBinary();

  // Estado inicial semaforos 
  xSemaphoreGive(semaforo1);
  xSemaphoreGive(semaforo2);

  // Crear tareas
  xTaskCreatePinnedToCore(tareaSemaforo1, "Tarea Semaforo 1", 2048, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(tareaSemaforo2, "Tarea Semaforo 2", 2048, NULL, 1, NULL, app_cpu);
}

void loop() {
 
}
