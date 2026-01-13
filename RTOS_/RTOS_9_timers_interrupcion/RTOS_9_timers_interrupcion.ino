static const uint64_t timer_max_cont = 1000000;
static const int led=19;
static const uint64_t divisor = 80;
//static timer_t *timer=NULL;
static hw_timer_t *timer = timerBegin(1000000,divisor);


//ISRs Rutinas de servicio de interrupcion 
void IRAM_ATTR onTimer(){
  //char str[20];
  //sprintf(str,"Nucleo %i", xPortGetCoreID);
  //Serial.println(str);
  int estado_pin = digitalRead(led);
  digitalWrite(led, !estado_pin);

}



void setup() {
  //Serial.begin(115200);
 pinMode(led,OUTPUT);
 // (cantidad, divisor, conteo up/down)
 timer=timerBegin(1000000,divisor);
  //ISR(temporizador,funcion)
 timerAttachInterrupt(timer, &onTimer);
 // temportizado, contador, auto-recarga
 timerAlarm(timer, timer_max_cont,true,0);
 //timerAlarmEnable(timer);
}

void loop() {
  // put your main code here, to run repeatedly:

}
