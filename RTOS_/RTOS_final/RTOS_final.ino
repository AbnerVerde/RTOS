#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif


// --- Librerías comunes ---
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>


#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>


// --- Pines TFT ---
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4


// --- Pines touch ---
#define TOUCH_CS 15
#define TOUCH_IRQ 27


// --- Pines hardware ---
const int pwm = 32;
const int resis = 35;


// --- Firebase y WiFi datos ---
#define WIFI_SSID "Iphone_av"
#define WIFI_PASSWORD "12345678"
#define Web_API_KEY "AIzaSyDct2EVeAaWrNBtmAFHyZR1uGj0CQK3RSw"
#define DATABASE_URL "https://rtos100-default-rtdb.firebaseio.com/"
#define USER_EMAIL "rtos2026@gmail.com"
#define USER_PASS "rtos2026"


// --- Variables globales ---
float T = 0.0;
float H = 0.0;
int Dec = 0;
int Uni = 0;
int tempdes = 0;
int timeSet = 0;
float Te = 0.0;
int pwmres = 0;
int pwmvent = 0;


// Firebase y WiFi objetos
UserAuth user_auth(Web_API_KEY, USER_EMAIL, USER_PASS);
FirebaseApp app;
WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);
RealtimeDatabase Database;


bool firebaseReady = false;


// --- Objetos ---
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);
Adafruit_AHTX0 aht;


// --- Estados del menú ---
enum MenuEstado {
  MENU_PRINCIPAL,
  MENU_TEMPERATURA,
  MENU_TIEMPO,
  MENU_ESTADO
};


MenuEstado estadoActual = MENU_PRINCIPAL;


// -------- Declaración funciones --------
void dibujarNumeros();
void dibujarBotones();
void processData(AsyncResult &aResult);


// -------- Tareas --------


void tareaAHT20(void *parameter) {
  sensors_event_t humidity, temperature;
  while (1) {
    aht.getEvent(&humidity, &temperature);
    T = temperature.temperature;
    H = humidity.relative_humidity;
    vTaskDelay(pdMS_TO_TICKS(1000));  // 1 segundo
  }
}


void tareatft(void *parameter) {
  while (1) {
    if (!ts.touched()) {
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }
    TS_Point p = ts.getPoint();
    int x = map(p.x, 200, 3800, 0, tft.width());
    int y = map(p.y, 200, 3800, 0, tft.height());
    delay(200); // Antirebote táctil


    switch (estadoActual) {
      case MENU_PRINCIPAL:
        if (x > 10 && x < 150 && y > 10 && y < 100) {
          Dec = tempdes / 10;
          Uni = tempdes % 10;
          dibujarNumeros();
          estadoActual = MENU_TEMPERATURA;
        }
        else if (x > 170 && x < 310 && y > 10 && y < 100) {
          Dec = timeSet / 10;
          Uni = timeSet % 10;
          dibujarNumeros();
          estadoActual = MENU_TIEMPO;
        }
        else if (x > 10 && x < 150 && y > 110 && y < 200) {
          tft.fillScreen(ILI9341_BLACK);
          tft.setTextSize(2);
          tft.setCursor(20, 10);
          tft.setTextColor(ILI9341_NAVY);
          tft.print("Temp deseada: ");
          tft.setTextColor(ILI9341_DARKGREEN);
          tft.print(tempdes);


          tft.setTextColor(ILI9341_NAVY);
          tft.setCursor(20, 30);
          tft.print("Tiempo: ");
          tft.setTextColor(ILI9341_DARKGREEN);
          tft.print(timeSet);


          tft.setTextColor(ILI9341_NAVY);
          tft.setCursor(20, 50);
          tft.print("Error temperatura: ");
          tft.setTextColor(ILI9341_DARKGREEN);
          tft.print(Te);


          tft.fillRect(90, 160, 140, 40, ILI9341_CYAN);
          tft.setCursor(120, 170);
          tft.setTextColor(ILI9341_BLACK);
          tft.print("Regresar");


          estadoActual = MENU_ESTADO;
        }
        else if (x > 170 && x < 310 && y > 110 && y < 200) {
          tempdes = 0;
          timeSet = 0;
          dibujarBotones();
        }
        break;


      case MENU_TEMPERATURA:
      case MENU_TIEMPO:
        if (x > 10 && x < 70 && y > 10 && y < 50) Dec = (Dec + 1) % 10;
        else if (x > 10 && x < 70 && y > 150 && y < 190 && Dec > 0) Dec--;
        else if (x > 130 && x < 190 && y > 10 && y < 50) Uni = (Uni + 1) % 10;
        else if (x > 130 && x < 190 && y > 150 && y < 190 && Uni > 0) Uni--;
        else if (x > 250 && x < 310 && y > 50 && y < 90) {
          int val = Dec * 10 + Uni;
          if (estadoActual == MENU_TEMPERATURA) {
            tempdes = val;
            Serial.println(tempdes);
          }
          else {
            timeSet = val;
            Serial.println(timeSet);
          }
          dibujarBotones();
          estadoActual = MENU_PRINCIPAL;
          break;
        }
        else if (x > 250 && x < 310 && y > 110 && y < 150) {
          dibujarBotones();
          estadoActual = MENU_PRINCIPAL;
          break;
        }
        dibujarNumeros();
        break;


      case MENU_ESTADO:
        if (x > 90 && x < 230 && y > 160 && y < 200) {
          dibujarBotones();
          estadoActual = MENU_PRINCIPAL;
        }
        break;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}


void tareaenvio(void *parameter) {
  while (1) {
    if (tempdes > 0) {
      Te = tempdes - T;
      Serial.print(H);
      Serial.print(",");
      Serial.println(Te);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}


void tarearecivir(void *parameter) {
  while (1) {
    if (Serial.available() > 0) {
      String linea = Serial.readStringUntil('\n');  // Lee hasta fin de línea
      int comaIndex = linea.indexOf(',');
      if (comaIndex > 0) {
        pwmres = linea.substring(0, comaIndex).toInt();   // Para resistencia (digital)
        pwmvent = linea.substring(comaIndex + 1).toInt(); // Para ventilador (PWM)


        // Control resistencia (salida digital)
        if (pwmres != 0) {
          digitalWrite(resis, HIGH);
        } else {
          digitalWrite(resis, LOW);
        }


        // Control ventilador (PWM)
        analogWrite(pwm, pwmvent);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}


void tareacontar(void *parameter) {
  while (1) {
    if (timeSet != 0 && tempdes != 0) {
      timeSet--;  // Disminuye en 1 minuto
      if (timeSet == 0) {
        // Reiniciar variables cuando el contador llega a 0
        tempdes = 0;
        Dec = 0;
        Uni = 0;
        pwmres = 0;
        pwmvent = 0;
        dibujarBotones();  // Refrescar pantalla
      }
    }
    vTaskDelay(pdMS_TO_TICKS(60000));  // Espera 1 minuto
  }
}


void tareaFirebase(void *parameter) {
  Serial.println("Conectando a WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    vTaskDelay(pdMS_TO_TICKS(300));
  }
  Serial.println("\nWiFi conectado!");


  ssl_client.setInsecure();
  ssl_client.setConnectionTimeout(1000);
  ssl_client.setHandshakeTimeout(5);


  initializeApp(aClient, app, getAuth(user_auth), processData, "🔐 authTask");
  app.getApp<RealtimeDatabase>(Database);
  Database.url(DATABASE_URL);


  firebaseReady = true;


  unsigned long lastSendTime = 0;
  const unsigned long sendInterval = 10000; // 10 segundos


  while (true) {
    app.loop();


    if (firebaseReady) {
      unsigned long currentTime = millis();
      if (currentTime - lastSendTime >= sendInterval) {
        lastSendTime = currentTime;


        // Subir temperatura deseada
        Database.set<int>(aClient, "/datos/temperatura_deseada", tempdes, processData, "RTDB_TempDeseada");


        // Subir error de temperatura
        Database.set<float>(aClient, "/datos/error_temperatura", Te, processData, "RTDB_ErrorTemp");


        // Subir humedad
        Database.set<float>(aClient, "/datos/humedad", H, processData, "RTDB_Humedad");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));  // Delay para no saturar CPU
  }
}


// ----- Funciones extras ------


void dibujarNumeros() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);


  // Decenas
  tft.fillRect(10, 10, 60, 40, ILI9341_PURPLE);
  tft.setCursor(30, 20);
  tft.setTextSize(3);
  tft.print("+");


  tft.fillRect(10, 60, 80, 80, ILI9341_BLUE);
  tft.setCursor(40, 100);
  tft.setTextSize(5);
  tft.print(Dec);


  tft.fillRect(10, 150, 60, 40, ILI9341_PURPLE);
  tft.setCursor(30, 160);
  tft.setTextSize(3);
  tft.print("-");


  // Unidades
  tft.fillRect(130, 10, 60, 40, ILI9341_PURPLE);
  tft.setCursor(150, 20);
  tft.setTextSize(3);
  tft.print("+");


  tft.fillRect(120, 60, 80, 80, ILI9341_BLUE);
  tft.setCursor(150, 100);
  tft.setTextSize(5);
  tft.print(Uni);


  tft.fillRect(130, 150, 60, 40, ILI9341_PURPLE);
  tft.setCursor(150, 160);
  tft.setTextSize(3);
  tft.print("-");


  // OK
  tft.fillRect(250, 50, 60, 40, ILI9341_GREEN);
  tft.setCursor(260, 60);
  tft.setTextSize(2);
  tft.print("OK");


  // Regresar
  tft.fillRect(250, 110, 60, 40, ILI9341_CYAN);
  tft.setCursor(255, 120);
  tft.setTextSize(2);
  tft.print("Back");
}


void dibujarBotones() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);


  tft.fillRect(10, 10, 140, 90, ILI9341_ORANGE);
  tft.setCursor(25, 50);
  tft.print("Temperatura");


  tft.fillRect(170, 10, 140, 90, ILI9341_BLUE);
  tft.setCursor(210, 50);
  tft.print("Tiempo");


  tft.fillRect(10, 110, 140, 90, ILI9341_GREEN);
  tft.setCursor(30, 150);
  tft.print("Estado");


  tft.fillRect(170, 110, 140, 90, ILI9341_RED);
  tft.setCursor(200, 150);
  tft.print("Reiniciar");
}


void processData(AsyncResult &aResult) {
  if (!aResult.isResult())
    return;
  if (aResult.isEvent())
    Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());
  if (aResult.isDebug())
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
  if (aResult.isError())
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
  if (aResult.available())
    Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
}


// --- SETUP y LOOP ---


void setup() {
  Serial.begin(115200);
  SPI.begin(18, 19, 23);


  pinMode(resis, OUTPUT);
  pinMode(pwm, OUTPUT);


  tft.begin();
  tft.setRotation(1);


  ts.begin();
  ts.setRotation(1);


  dibujarBotones();


  Wire.begin(21, 22);  // SDA, SCL


  if (!aht.begin()) {
    while (1);
  }


  // Crear tareas FreeRTOS
  xTaskCreatePinnedToCore(tareaAHT20, "Lectura AHT20", 2048, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(tareatft, "Lectura TFT", 2048, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(tareaenvio, "Envio Serial", 2048, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(tarearecivir, "Recepcion Serial", 2048, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(tareacontar, "Contador", 2048, NULL, 1, NULL, app_cpu);


  xTaskCreatePinnedToCore(tareaFirebase, "FirebaseTask", 8192, NULL, 1, NULL, app_cpu);
}


void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}


