#include "display.h"
#include "auxilary.h"
#include "logo.h"
#include "portal.h"

// Show the Cleere Auto Services logo (stays on screen during deep sleep).
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
    display.begin();
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

    static int lastMinute = -1;
    if (m != lastMinute)
    {
        lastMinute = m;
        showTime();
    }
    delay(1000);
}
