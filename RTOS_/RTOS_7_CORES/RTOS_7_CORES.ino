 //variables 
static const TickType_t tiempo_pr =200;

static const BaseType_t pro_cpu=0;
static const BaseType_t app_cpu=1;



//funciones 

static void retardo(uint32_t ms){
for(uint32_t i=0;i<ms;i++){
  for(uint32_t j=0;j<40000;j++){
asm("nop");
  }
}
}
//tareas 

void TareaL(void *parameters){
  char str[20];
while(1){
sprintf(str,"Task Low P, core %i\r\n", xPortGetCoreID());
Serial.print(str);
retardo(tiempo_pr);

}
}

void TareaH(void *parameters){
  char str[20];
while(1){
sprintf(str,"Task Low P, core %i\r\n", xPortGetCoreID());
Serial.print(str);
retardo(tiempo_pr);

}
}

void setup() {
Serial.begin(115200);
xTaskCreatePinnedToCore(TareaL,"Tarea L",2048,NULL,1,NULL,tskNO_AFFINITY);
xTaskCreatePinnedToCore(TareaH,"Tarea L",2048,NULL,2,NULL,tskNO_AFFINITY);
vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

}
