/*
Materia: RTOS

Equipo:
Abner Kalid Verde Padilla 
Diego Perez Dominguez 


-Este codigo se relaizo con el uso de RTOS en una esp32 

La funcion general del codigo es realizar varias tareas a la vez, por un lado se tienen 3 leds que parpadean,
teniendo periodos distintas 
*LED1 a 500ms 
*LED2 a 325ms 
*LED3 a 10ms 
Ademas de esto se tienen dos botones y se debe de cumplir unasecuencia de presionado por tiempo para que se 
encienda un tercer led que afirma que la secuencia de botones se ha completado. 

Para lograr esto se propuso lo siguiente:

La parte de los leds es la misma para los tres solo cambia el periodo pero en esencia es el mismo procedimiento, 
para empezar se crean tres tareas, una para cada led ledt01, ledt02 y ledt03, cada tarea tiene como instrucciones
encender el led, luego por medio de un delay dejarlo encendido por el tiempo establecido, y para finalizar 
una vez pasado el tiempo se apaga elled y se le aplica otro delay de mismo tiempo.

Por otro lado para los botenes se crea una tarea de una maquina de estados para poder cumplir la secuencia, ademas para 
el correcto uso de la maquina de estados se uso la funcion millis(), para poder medir la cantidad de tirmpo que un boton se 
deja presionado, por lo que si se ejecuta en tiempo y forma cada estado se avanzara hasta un estado final que es el encendido
de un cuarto led.  



*/



#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

const int led1=13; 
const int led2=12;
const int led3=14;

const int bo1=32; 
const int bo2=33;
const int led4=25;

int estado = 0;
unsigned long tiempoInicio = 0;

const unsigned long tiempoBase = 1000;     // tiempo esperado (1s)
const unsigned long tolerancia = 300;      // margen de ±300 ms

void ledt01(void *parameter){
  while(1){
    digitalWrite(led1, HIGH);
    delay(500);
    digitalWrite(led1, LOW);
    delay(500);
  }
}

void ledt02(void *parameter){
  while(1){
    digitalWrite(led2, HIGH);
    delay(325);
    digitalWrite(led2, LOW);
    delay(325);
  }
  }


void ledt03(void *parameter){
  while(1){
    digitalWrite(led3, HIGH);
    delay(10);
    digitalWrite(led3, LOW);
    delay(10);
  }
}

void botones(void *parameter) {
  while(1){
  bool b1 = !digitalRead(bo1); // activo en LOW
  bool b2 = !digitalRead(bo2);

  switch (estado) {
    case 0: // Esperando Botón 1
      if (b1 && !b2) {
        tiempoInicio = millis();
        estado = 1;
        Serial.println("Boton 1 presionado, esperando ~1s...");
      }
      break;

    case 1: // Botón 1 presionado, esperar 1s (con margen)
      if (millis() - tiempoInicio >= tiempoBase - tolerancia) {
        if (b1 && !b2) {
          estado = 2;
          Serial.println("Presiona Boton 2 (mantener ambos ~1s)...");
          tiempoInicio = millis();
        } else {
          estado = 0;
        }
      }
      break;

    case 2: // Ambos botones presionados
      if (b1 && b2) {
        if (millis() - tiempoInicio >= tiempoBase - tolerancia) {
          estado = 3;
          Serial.println("Suelta Boton 2 (mantener B1 ~1s)...");
          tiempoInicio = millis();
        }
      } else if (millis() - tiempoInicio > tiempoBase + tolerancia) {
        estado = 0; // se pasó de tiempo
      }
      break;

    case 3: // Botón 1 presionado, Botón 2 suelto
      if (b1 && !b2) {
        if (millis() - tiempoInicio >= tiempoBase - tolerancia) {
          estado = 4;
          Serial.println("Ahora suelta Boton 1...");
        }
      } else if (millis() - tiempoInicio > tiempoBase + tolerancia) {
        estado = 0;
      }
      break;

    case 4: // Esperar que suelte Botón 1
      if (!b1 && !b2) {
        digitalWrite(led4, HIGH);
        Serial.println(" Secuencia completada, LED encendido");
        estado = 5;
      }
      break;

    case 5: // Secuencia finalizada
      // LED se mantiene encendido
      break;
  }
  }
}


void setup() {
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);

 Serial.begin(9600);

  pinMode(bo1, INPUT_PULLUP);
  pinMode(bo2, INPUT_PULLUP);
  pinMode(led4, OUTPUT);

   xTaskCreatePinnedToCore(ledt01, "ledt01", 1024, NULL, 1, NULL, app_cpu);
   xTaskCreatePinnedToCore(ledt02, "ledt02", 1024, NULL, 2, NULL, app_cpu);
   xTaskCreatePinnedToCore(ledt03, "ledt03", 1024, NULL, 1, NULL, app_cpu);
   xTaskCreatePinnedToCore(botones, "botones", 1024, NULL, 1, NULL, app_cpu);
 
}

void loop() {
  // put your main code here, to run repeatedly:

}
