unsigned int sensorValue=0;
unsigned long past_time=0;

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
if(millis()-past_time>=100)
  {
  sensorValue=analogRead(0);
  trama(sensorValue);
  past_time=millis();
  }
}


void trama(unsigned int value1){
  Serial.print("<");
  
  if (value1 > 999) {
    Serial.print(value1);
  }else if (value1 > 99) {
    Serial.print("0");
    Serial.print(value1);
  }else if(value1>9){
    Serial.print("00"); 
    Serial.print(value1);
  }else{
    Serial.print("000");
    Serial.print(value1);
    } 
  Serial.println(">");
  }
