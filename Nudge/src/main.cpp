#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include "esp_sleep.h"
#include "Adafruit_DRV2605.h"

#define SLEEP_PIN GPIO_NUM_2
#define SDA_PIN 6
#define SCL_PIN 7

Adafruit_DRV2605 drv;
WebServer server(80);
int effectID = 1;

void hapticEffect(int id){
  drv.setWaveform(0, id);
  drv.setWaveform(1, 0);
  drv.go();
}

void setup(){
  Serial.begin(115200);
  pinMode(SLEEP_PIN, INPUT_PULLUP);

  Wire.begin(SDA_PIN, SCL_PIN);
  if (!drv.begin()){
    Serial.println("DRV Failed");
    while (1);
  }
  drv.selectLibrary(1);
  drv.setMode(DRV2605_MODE_INTTRIG);

  if(esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0){
    hapticEffect(effectID);
  }

  WiFi.softAP("Nudge_config", "passwordpassword");
  server.on("/", [](){
    String html = "<h1>Settings</h1><p>Current Effect: " + String(effectID) + "</p>";
    html += "<form action='/s'>New ID (1-123): <input name='v'><input type='submit'></form>";
    server.send(200, "text/html", html);
  });
  server.on("/s", [](){
    if (server.hasArg("v")) effectID = server.arg("v").toInt();
    server.send(200, "text/html", "Updated. <a href='/'>Back</a>");
  });
  server.begin();

  esp_deep_sleep_enable_gpio_wakeup(1 << SLEEP_PIN, ESP_GPIO_WAKEUP_GPIO_LOW);
  
  esp_sleep_enable_timer_wakeup(15 * 60 * 1000000);

  esp_deep_sleep_start();
  
}

void loop(){
  server.handleClient();

  if(millis() > 30000){
    Serial.println("Sleep");
    esp_deep_sleep_start();
  }
}