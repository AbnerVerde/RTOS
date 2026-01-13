//Configuración de núcleos a utilizar
#if CONFIG_FREERTOS_UNICORE
 // Si FreeRtos est{a comfigurado para usar un solo núcleo, se asigna el núcleo 0
  static const BaseType_t app_cpu = 0;
#else
  // Si se usan dos núcleos, se asigna el núcleo 1
  static const BaseType_t app_cpu=1;
#endif

// Led rates
static const int rate_1  = 500;  //ms
static const int rate_2  = 323;  //ms
static const int rate_3  = 10;  //ms

// Pins
static const int led_pin  = 2;
static const int led_pin2 = 4;
static const int led_pin3 = 6;

void toggleLed(void *parameter){
  while(1){
    digitalWrite(led_pin,HIGH);
    vTaskDelay(rate_1 / portTICK_PERIOD_MS);   //Este es como un delay pero general para cualquier tarjeta
    digitalWrite(led_pin,LOW);
    vTaskDelay(rate_1 / portTICK_PERIOD_MS);
  }
}

void toggleLed2(void *parameter){
  while(1){
    digitalWrite(led_pin,HIGH);
    vTaskDelay(rate_2 / portTICK_PERIOD)
    digitalWrite(led_pin,LOW);
    vTaskDelay(rate_2 / portTICK_PERIOD)
  }
}

void toggleLed3(void *parameter){
  while(1){
    digitalWrite(led_pin,HIGH);
    vTaskDelay(rate_3 / portTICK_PERIOD)
    digitalWrite(led_pin,LOW);
    vTaskDelay(rate_3 / portTICK_PERIOD)
  }
}

void setup() {

  pinMode(led_pin, OUTPUT);
  pinMode(led_pin2, OUTPUT);
  pinMode(led_pin3, OUTPUT);
  
  // Creacion de una tarea que se ejecutará de forma indefinida
  xTaskCreatePinnedToCore ( //En ESP32 se utiliza esta función, en FreeRTOS normal seria xTaskCreate()
                    toggleLed,    //Función que implementa la tarea
                    "Toggle LED", //Nombre descriptivo de la función
                    1024,         //Tamaño de la pila asignada
                    NULL,         //Parámetro a pasar de la funcion
                    1,            //Prioridad de la funcion
                    Null,         //Handle de la tarea
                    app_CPU)      //Núcleo en el que se ejecutará la tarea (0 ó 1)
  
  xTaskCreatePinnedToCore ( //En ESP32 se utiliza esta función, en FreeRTOS normal seria xTaskCreate()
                    toggleLed2,    //Función que implementa la tarea
                    "Toggle LED", //Nombre descriptivo de la función
                    1024,         //Tamaño de la pila asignada
                    NULL,         //Parámetro a pasar de la funcion
                    1,            //Prioridad de la funcion
                    Null,         //Handle de la tarea
                    app_CPU)      //Núcleo en el que se ejecutará la tarea (0 ó 1)
}

  xTaskCreatePinnedToCore ( //En ESP32 se utiliza esta función, en FreeRTOS normal seria xTaskCreate()
                    toggleLed3,    //Función que implementa la tarea
                    "Toggle LED", //Nombre descriptivo de la función
                    1024,         //Tamaño de la pila asignada
                    NULL,         //Parámetro a pasar de la funcion
                    1,            //Prioridad de la funcion
                    Null,         //Handle de la tarea
                    app_CPU)      //Núcleo en el que se ejecutará la tarea (0 ó 1)
}

void loop() {
  // put your main code here, to run repeatedly:

}