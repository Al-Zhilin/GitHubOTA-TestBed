#include <Arduino.h>
#include <GitHubOTA.h>

#if defined(ESP32)
    #include <WiFi.h>
    #include <WiFiClientSecure.h>
    WiFiClientSecure client;
#elif defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <WiFiClientSecureBearSSL.h>
    BearSSL::WiFiClientSecure client;
#endif

#ifndef FW_VERSION
    #define FW_VERSION "v0.0.0"                                // дефолт для локальной сборки без CI (CI подставляет реальный тег)
#endif

#define WiFi_SSID "ваше имя сети"
#define WiFi_PASS "ваш пароль от сети"

GitHubOTA updater;

void onStateChanged(GitHubOTA::State state) {
    switch (state) {
        case GitHubOTA::State::IDLE:           Serial.println("[GitHubOTA] Холостая работа");                                  break;
        case GitHubOTA::State::CHECKING:       Serial.println("[GitHubOTA] Проверка наличия обновлений...");                    break;
        case GitHubOTA::State::UP_TO_DATE:     Serial.println("[GitHubOTA] Установлена последняя версия");                      break;
        case GitHubOTA::State::UPDATE_AVAILABLE: Serial.println("[GitHubOTA] Доступно обновление, начинаем установку...");      break;
        case GitHubOTA::State::DOWNLOADING:    Serial.println("[GitHubOTA] Скачиваем найденное обновление...");                 break;
        case GitHubOTA::State::VERIFYING:      Serial.println("[GitHubOTA] Проверка целостности файла обновления...");          break;
        case GitHubOTA::State::REBOOTING:      Serial.println("[GitHubOTA] Обновление установлено, перезагружаемся...");        break;
        case GitHubOTA::State::ROLLING_BACK:   Serial.println("[GitHubOTA] Откат: новая прошивка не подтвердила себя вовремя"); break;
        case GitHubOTA::State::CAUGHT_ERROR:   Serial.printf("[GitHubOTA] Ошибка, код: %d\n", updater.getLastError());          break;
    }
}

void onDownloadProgress(size_t written, size_t total) {
    Serial.printf("[GitHubOTA] Скачано %zu из %zu байт (%.1f%%)\n", written, total, (float)written / total * 100.0f);
}

void setup() {
    Serial.begin(115200);

    Serial.printf("Прошивка %s, подключение к %s\n", FW_VERSION, WiFi_SSID);
    WiFi.begin(WiFi_SSID, WiFi_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(50);
        Serial.print(".");
    }
    Serial.println("Подключено к сети WiFi!");

    client.setInsecure();

    GitHubOTA::Config cfg;
    cfg.repoOwner = "Al-Zhilin";
    cfg.repoName = "GitHubOTA-TestBed";                        // проверяем обновления в самом себе
    cfg.currentVersion = FW_VERSION;
    cfg.checkIntervalMs = 120000;
    cfg.policy = GitHubOTA::Config::Policy::AutoInstall;

    // Регистрируем колбэки ДО begin() — begin() сам может дойти до отката
    // (ROLLING_BACK) и вызвать колбэк ещё до возврата из функции.
    updater.onStateChange(onStateChanged);
    updater.onProgress(onDownloadProgress);

    updater.begin(cfg, client);

    // Если begin() выше не откатил устройство (ROLLING_BACK) — значит мы
    // либо на обычной загрузке, либо на испытательном сроке после недавнего
    // OTA-обновления. Второй случай стоит явно показать в Serial, иначе
    // весь этап "подтверждение работоспособности" проходит незаметно.
    if (updater.isPendingValidation()) {
        Serial.println("[GitHubOTA] Испытательный срок после обновления — "
                        "ждём автоподтверждения (или явного confirmValid()/rejectAndRollback())");
    } else {
        Serial.println("[GitHubOTA] Обычная загрузка, подтверждать нечего");
    }
}

void loop() {
    updater.handle();
}
