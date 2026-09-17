/*
    - Прошивка стенда GitHubOTA-TestBed

    Единственный, неизменный релиз-цель для проверки examples библиотеки
    GitHubOTA на реальном железе. Сети не касается вообще: её единственная
    задача после OTA-обновления — подтвердить свою работоспособность
    (confirmValid(), закрывает pendingValidation/bootAttempts в EEPROM) и
    показать в Serial, что обновление прошло успешно.
*/

#include <Arduino.h>
#include <GitHubOTA.h>

#if defined(ESP32)
    #include <WiFiClientSecure.h>
    WiFiClientSecure client;                                 // сеть не используется, но begin() требует ссылку на клиент
#elif defined(ESP8266)
    #include <WiFiClientSecureBearSSL.h>
    BearSSL::WiFiClientSecure client;
#endif

#define FW_VERSION "v0.1.5"

GitHubOTA updater;

void setup() {
    Serial.begin(115200);

    GitHubOTA::Config cfg;
    cfg.repoOwner = "Al-Zhilin";
    cfg.repoName = "GitHubOTA-TestBed";
    cfg.currentVersion = FW_VERSION;

    updater.begin(cfg, client);          // читает/пишет только EEPROM — сеть не нужна
    updater.confirmValid();              // подтверждаем: новая прошивка работает штатно

    Serial.printf("Обновление установлено, текущая версия: %s\n", FW_VERSION);
}

void loop() {

}
