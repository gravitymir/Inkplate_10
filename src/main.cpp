#include "esp_task_wdt.h"
#include "display.h"
#include "auxilary.h"
#include "logo.h"
#include "portal.h"

// Hardware watchdog: reboot automatically if any single operation blocks longer
// than this. Must exceed the longest legitimate op — a full 3-bit refresh of the
// 10" panel is a couple of seconds — so 20 s is a safe margin.
#define WDT_TIMEOUT_S 20

// Free the I2C bus if a device (PMIC/RTC/expander) is holding SDA low after a
// power glitch: clock SCL up to 9 times until SDA releases. Default ESP32 Wire
// pins are SDA=21, SCL=22 (what the Inkplate library uses).
static void i2cBusRecover()
{
    const int SDA_PIN = 21, SCL_PIN = 22;
    pinMode(SDA_PIN, INPUT_PULLUP);
    pinMode(SCL_PIN, INPUT_PULLUP);
    if (digitalRead(SDA_PIN) == LOW)
    {
        pinMode(SCL_PIN, OUTPUT);
        for (int i = 0; i < 9 && digitalRead(SDA_PIN) == LOW; i++)
        {
            digitalWrite(SCL_PIN, LOW);
            delayMicroseconds(5);
            digitalWrite(SCL_PIN, HIGH);
            delayMicroseconds(5);
        }
        pinMode(SCL_PIN, INPUT_PULLUP);
    }
}

// Show the SHED 31 logo (stays on screen during deep sleep).
void showSleepLogo()
{
    display.clearDisplay();
    int x = (display.width() - LOGO_W) / 2;
    int y = (display.height() - LOGO_H) / 2;
    display.drawBitmap3Bit(x, y, logo3bit, LOGO_W, LOGO_H);
    display.display();
}

// Deep-sleep until the next g_workStart (e.g. 08:00). Button (GPIO36) also wakes.
void sleepUntilWorkStart()
{
    display.rtcGetRtcData();
    long secNow = (long)display.rtcGetHour() * 3600 +
                  (long)display.rtcGetMinute() * 60 +
                  (long)display.rtcGetSecond();
    long dur = (long)g_workStart * 3600 - secNow;
    if (dur <= 0)
        dur += 24L * 3600; // target is tomorrow
    Serial.printf("Outside working hours. Sleeping %ld s until %02d:00\n", dur, g_workStart);
    timeToSleep(0, 0, dur);
}

void setup()
{
    Serial.begin(SERIAL_BAUD);

    // Safety net: reboot the chip if anything blocks for WDT_TIMEOUT_S.
    esp_task_wdt_init(WDT_TIMEOUT_S, true); // true = panic -> reset
    esp_task_wdt_add(NULL);                 // watch this (setup/loop) task

    i2cBusRecover(); // release the I2C bus if it was left wedged
    display.begin();
    Wire.setTimeOut(50); // cap every PMIC/RTC/expander transaction at 50 ms
    display.setTextColor(0, 7);
    display.setRotation(2);
    display.clearDisplay();

    loadConfig(); // work hours from NVS

    // Open the AP config portal only on a real power-on / reset, not when
    // waking from our scheduled deep sleep (saves battery).
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    if (cause != ESP_SLEEP_WAKEUP_TIMER && cause != ESP_SLEEP_WAKEUP_EXT0)
    {
        runConfigPortal();
    }
}

void loop()
{
    esp_task_wdt_reset(); // feed the watchdog each pass

    // Refresh the clock once per minute, driven by the RTC.
    display.rtcGetRtcData();
    int h = display.rtcGetHour();
    int m = display.rtcGetMinute();

    // Show g_workStart:00 .. (g_workEnd-1):59 and g_workEnd:00 (last shown time).
    bool working = (h >= g_workStart && h < g_workEnd) ||
                   (h == g_workEnd && m == 0);
    // Outside working hours: show the logo, then deep-sleep until g_workStart.
    // (g_workEnd:00 stays as the last clock minute; logo appears from :01.)
    if (!working)
    {
        showSleepLogo();
        sleepUntilWorkStart();
        return;
    }

    // Draw the current minute, then deep-sleep until the next minute boundary.
    // Each wake is a clean boot->draw->sleep cycle, so a one-off glitch
    // self-heals on the next wake (and it saves battery/heat).
    showTime();
    int s = display.rtcGetSecond();
    long toNextMinute = 60 - s;
    if (toNextMinute < 1)
        toNextMinute = 1;
    timeToSleep(0, 0, toNextMinute); // deep sleep + ext0 (button) wake
}
