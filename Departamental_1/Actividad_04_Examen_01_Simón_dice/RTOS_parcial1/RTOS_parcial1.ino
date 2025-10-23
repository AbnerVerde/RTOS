/*
Materia: RTOS

Equipo:
Abner Kalid Verde Padilla 
Diego Perez Dominguez 


-Este codigo se relaizo con el uso de RTOS en una esp32 

Se quiere realizar un juego de simon dice por medio de un teclado matricial y una pantalla LCD

Para realizar este juego se plantea la creacion de 4 tareas tareaescritura, tarealectura, tareacomparacion, estadoinicial:

tareaescritura: En esta tarea se genera un numero o caracter aleatorio que este dentro del teclado matricial, se guarda en un
arreglo de longitud 100 y el arreglo mustra los caracteres hasta una longitud establecida por un contador que depende de la cantidad 
de veces que se hace un ciclo de manera exitosa una vez acabado esto se pasa para la tarealectura

tarealectura: En esta tarea se espera por medio de un ciclo while a que se presione una tecla en el teclado matricial, una vez se 
presione una tecla se sale del ciclo while y almacenana este caracter en un arreglo de longitud 100 para despues pasar a la tareacomparacion

tareacomparacion: En esta tarea se comparan ambos arreglos y se parte de una bandera establecida como verdadera y si el arreglo de lectura 
y el arreglo de comparacion no son iguales, se imprime un mensaje de perder y se reinicia el sistema pasando al estado inicial, sin embargo si
ambos arreglos son iguales se regresa a la tarea de escritura para la generacion de otro caracter en el arreglo 

estadoinicial: se parte de este estado en donde se configuran todas las variables (incluidos arreglos) en 0, ya sea int o char, 
ademas en esta tarea se espera a que se de clic en el boton de inicio para que una vez este se active, inicie la tarea de escritura 

Para poder trancicionar entre tareas se hace uso de semaforos, en este caso se usaron 4, uno por tarea lcdd correspondiente a tareaescritura,
tecladom correspondiente a tarealectura, comparacion correspondiente a tareacomparacion, estado_inicial correspondiente a tareacomparacion, por
lo que la tarea que toma el semaforo al inicio es estadoinicial y acabando la tarea sale de la zona critica y una nuva tarea entra en zonacritica 
en este caso tarealectura y asi con las demas tareas.

En otros detalles se utilizo la libreria de Keypad para el teclado matricial y la libreria i2c para el control de la pantalla lcd, por ultimo 
para el trabajo con arreglos se usaron ciclos for 

*/



#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

//librerias 

#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//pantalla LCD

LiquidCrystal_I2C lcd(0x27, 16, 2);

//Pines I2C

#define sda 21
#define scl 22

//Variables

char memoria[100];      // Lo que escribe el usuario
char simon_dice[100];   // Secuencia del juego

const byte FILAS = 4;
const byte COLUMNAS = 4;
char teclas[FILAS][COLUMNAS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte pinesFilas[FILAS] = {26, 25, 33, 32};
byte pinesColumnas[COLUMNAS] = {13, 12, 14, 27};

char teclade[] = {'1','2','3','A','4','5','6','B','7','8','9','C','*','0','#','D'};
char letra;
char lectura = '\0';

Keypad teclado = Keypad(makeKeymap(teclas), pinesFilas, pinesColumnas, FILAS, COLUMNAS);

// Semáforos
SemaphoreHandle_t estado_inicial;
SemaphoreHandle_t tecladom;
SemaphoreHandle_t lcdd;
SemaphoreHandle_t comparacion;

// Botón de inicio
const int boton_inicio = 18;
bool flaginicio = false;
bool flag1=false;

// Control de secuencia
int contador = 0;

//tareas
void tareaescritura(void* parameter){
  while(1){
    xSemaphoreTake(lcdd, portMAX_DELAY);

    // Agregar nueva letra al final de la secuencia
    letra = teclade[random(0,16)];
    simon_dice[contador] = letra;

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Simon dice:");
    vTaskDelay(500 / portTICK_PERIOD_MS);

    for(int i=0; i<=contador; i++){
      lcd.setCursor(i,1);
      lcd.print(simon_dice[i]);
      vTaskDelay(800 / portTICK_PERIOD_MS);
    }
    vTaskDelay(400 / portTICK_PERIOD_MS);
    lcd.clear();

    xSemaphoreGive(tecladom);
  }
}

void tarealectura(void* parameter){
  while(1){
    xSemaphoreTake(tecladom, portMAX_DELAY);
    memset(memoria, 0, sizeof(memoria));

    for(int i=0; i<=contador; i++){
      lectura = '\0';
      while(lectura == '\0'){
        lectura = teclado.getKey();
        vTaskDelay(50 / portTICK_PERIOD_MS);
      }
      memoria[i] = lectura;

      lcd.setCursor(i,0);
      lcd.print(memoria[i]);
      vTaskDelay(300 / portTICK_PERIOD_MS);
    }
    xSemaphoreGive(comparacion);
  }
}

void tareacomparacion(void* parameter){
  while(1){
    xSemaphoreTake(comparacion, portMAX_DELAY);
    bool correcto = true;

    for(int i=0; i<=contador; i++){
      if(memoria[i] != simon_dice[i]){
        correcto = false;
        break;
      }
    }

    if(correcto){
      contador++;
      xSemaphoreGive(lcdd);
    } else {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Perdiste!");
      lcd.setCursor(0,1);
      lcd.print("Reiniciando...<3");
      flaginicio = false;
      vTaskDelay(3000 / portTICK_PERIOD_MS);
      //flag=true;
      xSemaphoreGive(estado_inicial);
    }
  }
}

void estadoinicial(void* parameter){
  while(1){
    xSemaphoreTake(estado_inicial, portMAX_DELAY);
   // if (flag=true)
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Inicio <3");
    lcd.setCursor(0,1);
    lcd.print("Presiona boton");

    contador = 0;
    memset(memoria, 0, sizeof(memoria));
    memset(simon_dice, 0, sizeof(simon_dice));
    letra = '\0';
    lectura = '\0';

    
    while(digitalRead(boton_inicio) == HIGH){
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    flaginicio = true;
    if(flaginicio){
      lcd.clear();
      xSemaphoreGive(lcdd);
    }
  }
}


void setup() {
  lcd.init();
  lcd.backlight();
  Wire.begin(sda,scl);
  pinMode(boton_inicio, INPUT_PULLUP);

  // Crear semáforos
  lcdd = xSemaphoreCreateBinary();
  tecladom = xSemaphoreCreateBinary();
  comparacion = xSemaphoreCreateBinary();
  estado_inicial = xSemaphoreCreateBinary();

  // Crear tareas
  xTaskCreatePinnedToCore(tareaescritura, "Escritura LCD", 2048, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(tarealectura, "Lectura Teclado", 2048, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(tareacomparacion, "Comparacion", 2048, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(estadoinicial, "Estado Inicial", 2048, NULL, 1, NULL, app_cpu);

  xSemaphoreGive(estado_inicial);
}

void loop() {

}
