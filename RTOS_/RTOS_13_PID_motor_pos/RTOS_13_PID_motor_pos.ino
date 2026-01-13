#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// -------------------- CONFIGURACIONES --------------------
#define dt_us 2000
float dt = dt_us * 0.000001f;

// Encoder
volatile long Np = 0;
const float R = 0.1428;

// Pines
const int pinA = 25;
const int pinB = 26;
const int sen1 = 23;
const int sen2 = 22;


// PID
float kp = 1.25, kd = 0.25, ki = 0.07;
float e = 0, de = 0, inte = 0;
float th = 0, thp = 0;
float dth_f = 0;
float alpha = 0.05;

float th_des = 0;     // Consigna recibida por serial

// Control
float u = 0, usat = 0, PWM = 0;

// RTOS
SemaphoreHandle_t semEncoder;
TaskHandle_t taskControlHandle;

// ---------------------------------------------------------


// ===================== ISR ENCODER ======================
void IRAM_ATTR isrA() {
  BaseType_t hp = pdFALSE;

  if (digitalRead(pinB) == LOW) Np++;
  else Np--;

  xSemaphoreGiveFromISR(semEncoder, &hp);
  if (hp) portYIELD_FROM_ISR();
}

void IRAM_ATTR isrB() {
  BaseType_t hp = pdFALSE;

  if (digitalRead(pinA) == HIGH) Np++;
  else Np--;

  xSemaphoreGiveFromISR(semEncoder, &hp);
  if (hp) portYIELD_FROM_ISR();
}
// ========================================================



// ===================== TAREA CONTROL =====================
void taskControl(void *pv) {

  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(dt_us / 1000.0);

  while (1) {

    // ----- Leer encoder -----
    if (xSemaphoreTake(semEncoder, 0) == pdTRUE) {
      // Nada adicional, Np ya se actualiza en ISR
    }

    // ----- Cálculo de posición -----
    th = R * Np;

    // Derivada discreta
    float dth_d = (th - thp) / dt;

    // Filtro
    dth_f = alpha * dth_d + (1 - alpha) * dth_f;

    // ----- PID -----
    e = th_des - th;
    de = -dth_f;
    inte += e * dt;

    u = kp * e + kd * de + ki * inte;
    usat = constrain(u, -12, 12);

    PWM = usat * 21.25;   // Ajuste a 0–255

    // ----- Enviar PWM -----
    if (PWM >= 0) {
      analogWrite(sen1, PWM);
      analogWrite(sen2, 0);
    } else {
      analogWrite(sen1, 0);
      analogWrite(sen2, -PWM);
    }

    thp = th;

    // ----- Debug serial -----
    Serial.print(th);
    Serial.print(",");
    Serial.println(PWM);

    // Esperar siguiente periodo
    vTaskDelayUntil(&xLastWakeTime, period);
  }
}
// ========================================================



// ===================== TAREA SERIAL =====================
void taskSerial(void *pv) {
  String dato;
  while (1) {
    if (Serial.available()) {
      dato = Serial.readStringUntil('\n');
      th_des = dato.toFloat();
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
// ========================================================



// ======================== SETUP ==========================
void setup() {
  Serial.begin(115200);

  // Salidas
  pinMode(sen1, OUTPUT);
  pinMode(sen2, OUTPUT);

  // Encoder
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);

  // Semáforo tipo counting (1 pulso por interrupción)
  semEncoder = xSemaphoreCreateCounting(10000, 0);

  // Interrupciones
  attachInterrupt(pinA, isrA, RISING);
  attachInterrupt(pinB, isrB, RISING);

  // Tareas
  xTaskCreatePinnedToCore(taskControl, "Control", 4096, NULL, 2, &taskControlHandle, app_cpu);
  xTaskCreatePinnedToCore(taskSerial,  "Serial",  4096, NULL, 1, NULL, app_cpu);
}

void loop() {
  // No se usa
}
