#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static const int led_pin = 2;
static SemaphoreHandle_t bin_sem;

void blinkLED(void *parameter){
int num = *(int*)parameter;
//liberamos semaforo binario
xSemaphoreGive(bin_sem);
Serial.println("liberamos semaforo .....");
pinMode(led_pin,OUTPUT);
while(1){
    digitalWrite(led_pin,HIGH);
    vTaskDelay(num/portTICK_PERIOD_MS);
    digitalWrite(led_pin,LOW);
    vTaskDelay(num/portTICK_PERIOD_MS);

}
}



void setup() {
  long int delay_arg;
  Serial.begin(115200);
  Serial.println("introduce el valor del retardo (milisegundos)");
  while(Serial.available()<=0);
  delay_arg=Serial.parseInt();
  Serial.println("creacion de semaforo");
  Serial.println(delay_arg);
  

  bin_sem = xSemaphoreCreateBinary();
  xTaskCreatedPinnedToCore(blinkLED,
                           "Blink Led",
                           (void*)&delay_arg,
                           1,
                           NULL,
                           app_cpu);
  xSemaphoreTake(bin_sem,portMAX_DELAY);
  Serial.println("Tomado .......");

}

void loop() {


}
