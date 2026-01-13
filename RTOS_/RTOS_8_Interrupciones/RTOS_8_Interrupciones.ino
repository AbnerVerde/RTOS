#define pinButton 4
// Semaforo binario
SemaphoreHandle_t xSem= Null;

void IRAM_ATTR isr_gpio(){
  BaseType_t task_woken = pdFALSE;
  xSemaphoreGiveFromISR(xSem, &task_woken);
  if(task_woken){
    portYIELD_FROM_ISR();
  }
}

void taskButton(void * parameter){
  while(1){
    if (xSempahoreTake(xSem, portMAX_DELAY)){
      Serial.printl("Cambio de estado detectado");
    }
  }
}


void setup() {
  Serial.begin(115200);

  // Crear semáforo
  xSem = xSempahoreCreateBinary();

  // Configurar pin con interrupcion por flanco
  pinMode(pinButton, INPUT_PULLUP);
  attachInterrupt(pinButton,isr_gpio, CHANGE);

  // Crear tarea
  xTaskCreate(taskButton, "TaskButton", 2048, NULL, 1, NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

}