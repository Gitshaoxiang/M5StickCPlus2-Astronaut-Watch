#include "M5StickCPlus2.h"
#include "M5GFX.h"
#include "animation.h"
#include "FxLED_32.h"

#include <esp_sntp.h>

#define WIFI_SSID     "WIFI_SSID"
#define WIFI_PASSWORD "WIFI_PASSWORD"
#define NTP_TIMEZONE  "CST-8"
#define NTP_SERVER1   "0.pool.ntp.org"
#define NTP_SERVER2   "1.pool.ntp.org"
#define NTP_SERVER3   "2.pool.ntp.org"

#include <WiFi.h>

M5Canvas canvas(&StickCP2.Display);

const uint8_t* list[] = {
    Astronaut_1_0, Astronaut_1_1, Astronaut_1_2, Astronaut_1_3, Astronaut_1_4,
    Astronaut_1_5, Astronaut_1_6, Astronaut_1_7, Astronaut_1_8, Astronaut_1_9};

void updateTime();
void updateAnimation();
void updateBat();
void updateInit(String str);

void setup() {
    auto cfg = M5.config();
    StickCP2.begin(cfg);
    StickCP2.Display.setRotation(3);
    canvas.setTextColor(BLACK);
    canvas.createSprite(StickCP2.Display.width(), StickCP2.Display.height());
    canvas.setTextDatum(top_center);

    if (!StickCP2.Rtc.isEnabled()) {
        Serial.println("RTC not found.");
        StickCP2.Display.println("RTC not found.");
        for (;;) {
            vTaskDelay(500);
        }
    }

    StickCP2.Display.print("WiFi:");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        updateInit("WiFi Connecting...");
        canvas.pushSprite(0, 0);
    }
    Serial.println("\r\n WiFi Connected.");

    configTzTime(NTP_TIMEZONE, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);

    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED) {
        Serial.print('.');
        updateInit("SNTP Connecting...");
    }

    Serial.println("\r\n NTP Connected.");

    time_t t = time(nullptr) + 1;  // Advance one second.
    while (t > time(nullptr))
        ;  /// Synchronization in seconds

    StickCP2.Rtc.setDateTime(gmtime(&t));
}

uint8_t animation_index = 0;

static constexpr const char* const wd[7] = {"Sun", "Mon", "Tue", "Wed",
                                            "Thr", "Fri", "Sat"};

void loop() {
    canvas.fillScreen(WHITE);
    updateAnimation();
    updateTime();
    updateBat();
    canvas.pushSprite(0, 0);
}

void updateInit(String str) {
    canvas.fillScreen(WHITE);

    canvas.drawJpg(list[animation_index], sizeof(Astronaut_1_0), 90, 24);
    if (animation_index < 9) {
        animation_index++;
    } else {
        animation_index = 0;
    }

    canvas.setTextFont(&fonts::FreeSansBold9pt7b);
    canvas.drawString(str, 120, 92);
    canvas.pushSprite(0, 0);
}

void updateAnimation() {
    canvas.drawJpg(list[animation_index], sizeof(Astronaut_1_0), 0, 24);
    if (animation_index < 9) {
        animation_index++;
    } else {
        animation_index = 0;
    }
}

void updateTime() {
    auto dt = StickCP2.Rtc.getDateTime();
    Serial.printf("RTC   UTC  :%04d/%02d/%02d (%s)  %02d:%02d:%02d\r\n",
                  dt.date.year, dt.date.month, dt.date.date,
                  wd[dt.date.weekDay], dt.time.hours, dt.time.minutes,
                  dt.time.seconds);

    char sec_str[5]   = {0};
    char time_str[40] = {0};
    char date_str[50] = {0};
    sprintf(sec_str, "%02d", dt.time.seconds);
    sprintf(time_str, "%02d:%02d", dt.time.hours + 8, dt.time.minutes);
    sprintf(date_str, "%04d/%02d/%02d (%s)", dt.date.year, dt.date.month,
            dt.date.date, wd[dt.date.weekDay]);

    canvas.loadFont(FxLED_32);
    canvas.setTextSize(2);
    canvas.drawString(time_str, 120, 24);
    canvas.setTextSize(1);
    canvas.drawString(sec_str, 210, 44);
    canvas.setTextFont(&fonts::FreeSansBold9pt7b);
    canvas.drawString(date_str, 120, 92);
}

void updateBat() {
    uint8_t bat_level = StickCP2.Power.getBatteryLevel();
    Serial.printf("Battery Level:%d\r\n", bat_level);
    uint8_t icon_width = map(bat_level, 0, 100, 0, 40);
    canvas.fillSmoothRoundRect(190, 5, icon_width, 15, 4, GREEN);
    canvas.drawRoundRect(190, 5, 40, 15, 4, BLACK);
    canvas.setTextSize(0.7);
    canvas.drawString(String(bat_level) + "%", 210, 7);
}