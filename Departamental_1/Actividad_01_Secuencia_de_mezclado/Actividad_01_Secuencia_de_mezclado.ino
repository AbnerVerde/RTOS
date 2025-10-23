/*
Materia: RTOS

Equipo:
Abner Kalid Verde Padilla 
Diego Perez Dominguez 


-Este codigo se realizo con el uso de RTOS en una esp32 

la secuencia automática de control de un sistema con varias válvulas y un motor, gestionados por sensores y un botón de inicio.

Se realizo una tarea para cada fase de nuestro sistema, en general todas las tareas son iguales, en el sentido de que encienden 
cuando se cumple una condicion dada por una bandera y se apaga cuando hay un cambio en la bandera, por otro lado estas banderas hacen 
que una tarea se pause o se le de continuidad, por lo que es necesario iniciar con todas las tareas pausadas y las vaiables y constantes 
de igual forma. para pausar tareas se usa un taskHadle.

En este sistema se pueden hacer varias tareas secuencialmente en donde se inicia con dos valvulas abiertas y justamente esto se logra por 
medio de las banderas usadas a lo largo del codigo.
*/


#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

//Manejadores de tareas
static TaskHandle_t tra_vA = NULL;
static TaskHandle_t tra_vB = NULL;
static TaskHandle_t tra_v1 = NULL;
static TaskHandle_t tra_v2 = NULL;
static TaskHandle_t tra_m  = NULL;
static TaskHandle_t tra_v3 = NULL;

//Salidas
const int vA = 5;   // Válvula A
const int vB = 18;  // Válvula B
const int v1 = 19;  // Válvula 1
const int v2 = 21;  // Válvula 2
const int v3 = 22;  // Válvula 3 (desagüe)
const int m  = 23;  // Motor

//Entradas
const int sp1 = 32; // Sensor 1
const int sp2 = 33; // Sensor 2
const int dl1 = 34; // Sensor 3
const int dl2 = 35; // Sensor 4
const int stb = 25; // Botón inicio

//Variables de control
unsigned int a=0,b=0,c=0,d=0,e=0,f=0,g=0;

// Función inicial
void inicial(){
  digitalWrite(vA, LOW);
  digitalWrite(vB, LOW);
  digitalWrite(v1, LOW);
  digitalWrite(v2, LOW);
  digitalWrite(v3, LOW);
  digitalWrite(m, LOW);
}

// Tareas
void vAon(void *parameter){
  while(1){
    if(digitalRead(sp1) == LOW){
      digitalWrite(vA, HIGH);
    } else {
      digitalWrite(vA, LOW);
      a = 1;  
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void vBon(void *parameter){
  while(1){
    int estado_sp1 = digitalRead(sp1);
    int estado_sp2 = digitalRead(sp2);

    if((estado_sp2 == LOW) && (estado_sp1 == HIGH)){
      digitalWrite(vB, HIGH);
    }
    else if ((estado_sp2 == HIGH) && (estado_sp1 == HIGH)){
      digitalWrite(vB, LOW);
      b = 1;
    }
    else digitalWrite(vB, LOW);

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void v1on(void *parameter){
  while(1){
    if(digitalRead(dl2) == LOW) {
      digitalWrite(v1, HIGH);
    } else {
      digitalWrite(v1, LOW);
      c = 1;
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void v2on(void *parameter){
  while(1){
    int estado_sp1 = digitalRead(sp1);
    int estado_sp2 = digitalRead(sp2);

    if ((estado_sp2 == HIGH)&&(estado_sp1 == HIGH)){
      digitalWrite(v2, HIGH);
    }
    else if ((estado_sp2 == LOW)&&(estado_sp1 == HIGH)){
      digitalWrite(v2, HIGH);
    }
    else {
      digitalWrite(v2, LOW);
      d = 1;
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void mon(void *parameter){
  while(1){
    if(digitalRead(dl1) == HIGH) {
      digitalWrite(m, HIGH);
      vTaskDelay(pdMS_TO_TICKS(5000));   // motor 5s
      e = 1;
    }
    else digitalWrite(m, LOW);

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void v3on(void *parameter){
  while(1){
    if ((digitalRead(dl2) == HIGH) && (e == 1)){
      digitalWrite(v3, HIGH);
      vTaskDelay(pdMS_TO_TICKS(5000));   // desagüe 5s
      f = 1;
    }
    else {
      digitalWrite(v3, LOW);
      inicial();
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}


void setup() {
  // Configuración salidas
  pinMode(vA, OUTPUT);
  pinMode(vB, OUTPUT);
  pinMode(v1, OUTPUT);
  pinMode(v2, OUTPUT);
  pinMode(v3, OUTPUT);
  pinMode(m, OUTPUT);

  // Configuración entradas
  pinMode(sp1, INPUT);
  pinMode(sp2, INPUT);
  pinMode(dl1, INPUT);
  pinMode(dl2, INPUT);
  pinMode(stb, INPUT_PULLUP);

  // Crear tareas
  xTaskCreatePinnedToCore(vAon, "vA", 2048, NULL, 1, &tra_vA, app_cpu);
  xTaskCreatePinnedToCore(vBon, "vB", 2048, NULL, 1, &tra_vB, app_cpu);
  xTaskCreatePinnedToCore(v1on, "v1", 2048, NULL, 1, &tra_v1, app_cpu);
  xTaskCreatePinnedToCore(v2on, "v2", 2048, NULL, 1, &tra_v2, app_cpu);
  xTaskCreatePinnedToCore(mon,  "m",  2048, NULL, 1, &tra_m,  app_cpu);
  xTaskCreatePinnedToCore(v3on, "v3", 2048, NULL, 1, &tra_v3, app_cpu);

  // Pausar tareas en estado inicial
  vTaskSuspend(tra_vA);
  vTaskSuspend(tra_vB);
  vTaskSuspend(tra_v1);
  vTaskSuspend(tra_v2);
  vTaskSuspend(tra_m);
  vTaskSuspend(tra_v3);

  inicial();
}


void loop() {
  bool stbr = digitalRead(stb); // botón activo en LOW
  if (stbr == LOW){   // pulsado
    g = 1;
  }
  
  if (g == 1){
    vTaskResume(tra_vA);
    vTaskResume(tra_v1);

    if (a==1){
      vTaskSuspend(tra_vA);
      vTaskResume(tra_vB);
    }
    if (b==1){
      vTaskSuspend(tra_vB);
      vTaskResume(tra_v1);
    }
    if (c==1){
      vTaskResume(tra_v2);
      vTaskSuspend(tra_v1);
    }
    if (d==1){
      vTaskResume(tra_m);
      vTaskSuspend(tra_v2);
    }
    if (e==1){
      vTaskResume(tra_v3);
      vTaskSuspend(tra_m);
    }
    if (f==1){
      vTaskSuspend(tra_v3);
      g=0;
    }
  } 
  else {
    inicial();
  }
}
