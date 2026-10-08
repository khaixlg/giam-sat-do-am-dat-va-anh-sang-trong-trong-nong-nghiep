#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#define Soil 34
#define LDR_PIN 35
#define Led 15
#define BUZZER_PIN 32

// Điền thông tin thật trên máy, đừng đẩy lên GitHub
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
const char* serverName = "Your_server_address"; // Ví dụ: "http://

void Task_LDR_LED(void *pvParameters);
void Task_Buzzer_Soil(void *pvParameters);
void Task_SendData(void *pvParameters);

volatile int Soil_val = 0;
volatile int rawValue = 0;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Dang ket noi Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Da ket noi voi Wi-Fi!");
  Serial.print("Dia chi IP: ");
  Serial.println(WiFi.localIP());

  pinMode(Led, OUTPUT);

  // PWM cho còi (ESP32 core 3.x): chân, tần số 1500Hz, độ phân giải 8 bit
  ledcAttach(BUZZER_PIN, 1500, 8);

  xTaskCreatePinnedToCore(Task_SendData, "Task_SendData", 8192, NULL, 2, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(Task_Buzzer_Soil, "Task_Buzzer_Soil", 2048, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(Task_LDR_LED, "Task_LDR_LED", 2048, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
}

void loop() {
}

void Task_LDR_LED(void *pvParameters) {
  (void) pvParameters;
  while (1) {
    long total = 0;
    for (int i = 0; i < 5; i++) {
      total += analogRead(LDR_PIN);
      vTaskDelay(5 / portTICK_PERIOD_MS);
    }
    rawValue = total / 5;
    if (rawValue < 750) {
      digitalWrite(Led, HIGH);
    } else {
      digitalWrite(Led, LOW);
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void Task_Buzzer_Soil(void *pvParameters) {
  (void) pvParameters;
  int Count_Buz_Warning = 0;
  int Count_Buz_Danger = 0;
  bool buzzOn = false;

  while (1) {
    float Soil_temp = analogRead(Soil);
    Soil_val = (int)(100.0 - (Soil_temp / 4095.0) * 100.0);

    if (Soil_val > 10 && Soil_val <= 50) {
      // Cảnh báo: bíp tối đa 6 lần
      if (Count_Buz_Warning < 6) {
        buzzOn = !buzzOn;
        ledcWrite(BUZZER_PIN, buzzOn ? 128 : 0);
        Count_Buz_Warning++;
      } else {
        ledcWrite(BUZZER_PIN, 0);
      }
    } else if (Soil_val > 50) {
      // Nguy hiểm: bíp tối đa 10 lần
      if (Count_Buz_Danger < 10) {
        buzzOn = !buzzOn;
        ledcWrite(BUZZER_PIN, buzzOn ? 255 : 0);
        Count_Buz_Danger++;
      } else {
        ledcWrite(BUZZER_PIN, 0);
      }
    } else {
      // Độ ẩm <= 10%: tắt còi và reset bộ đếm
      Count_Buz_Danger = 0;
      Count_Buz_Warning = 0;
      buzzOn = false;
      ledcWrite(BUZZER_PIN, 0);
    }
    vTaskDelay(300 / portTICK_PERIOD_MS);
  }
}

void Task_SendData(void *pvParameters) {
  (void) pvParameters;
  while (1) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String postData = "light=" + String(rawValue) + "&humidity=" + String(Soil_val);
      http.begin(serverName);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      int httpResponseCode = http.POST(postData);
      if (httpResponseCode > 0) {
        String payload = http.getString();
        StaticJsonDocument<100> doc;
        DeserializationError err = deserializeJson(doc, payload);
        if (!err && doc.containsKey("led")) {
          int ledState = doc["led"];
          if (ledState == 1) {
            digitalWrite(Led, HIGH);
          } else if (ledState == 0) {
            digitalWrite(Led, LOW);
          }
        }
      }
      http.end();
    }
    vTaskDelay(700 / portTICK_PERIOD_MS);  // nằm TRONG vòng while
  }
}
