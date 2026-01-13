
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static const int adc_pin =A0;
static hw_timer_t *timer =NULL;
static volatile uint16_t valor;
static SemaphoreHandle_t bin_sem=NULL
//1Mhz
static const uint16_t timer_divisor =80;
static const uint16_t timer_max_contador =1000000;

//interrupcion
void IRAM_ATTR onTimer(){


  baseType_t task_woken = pdFALSE; //solo para ESP32
  //if(task_woken){portYIELD_FROM();}
  //portYIELD_FROM(task_woken); //para otras tarjetas con freeRTOS
  valor=analogRead(adc_pin);
  //liberamos el semaforo para informar que hay un dato nuevo 
  xSemaphoreGiveFromISR(bin_sem, %task_woken);
  //salimos de la interrupcion 
  if(task_woken){
    portYIELD_FROM()
  }
}

void printfvalores(void *parameters){
  while(1){
    xSemaphoreTake(bin_sem, portMax_DELAY);
    Serial.println(valor);
  }
}


void setup() {
  Serial.begin(115200);
  vTaskdelay(100/portTICK_PERIOD_MS);
  bin_sem=xSemaphoreCreateBinary();
  xTaskCreateBinary(printValores,"print valores",1024,NULL,1,NULL,app_cpu);
  timer=timerBegin(1000000);
  timerAttachInterrupt(timer,-max &onTimer, true);
  timerAlarm(timer,timer_max_contador,true,0);
  //timerAlarmWrite(timer,timer_max_contador,true);
  //TimerAlarmEnable(timer);

}

void loop() {
  // put your main code here, to run repeatedly:

}
