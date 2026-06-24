#include "esp_random.h"
#define SERIAL_BAUD 115200

#define uS_TO_S_FACTOR 1000000

const String namesOfWeekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const String namesOfWeekDaysSmall[7] = {"sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday"};

void wrapData(char *destination, String title, String appenStr);
void appendString(char *destination, String appenStr);
void timeToSleep(u64_t hour, u64_t minute, u64_t second);
void showTime();

void appendString(char *destination, String appenStr)
{
    strcat(destination, appenStr.c_str());
}
void wrapData(char *destination, String title, String appenStr)
{
    appendString(destination, (String) "<code>");
    appendString(destination, title);
    appendString(destination, appenStr);
    appendString(destination, (String) "</code>\n");
}
// display.rtcClearAlarmFlag();
// display.rtcSetAlarmEpoch(display.rtcGetEpoch() + 10, RTC_ALARM_MATCH_DHHMMSS);

void timeToSleep(u64_t hour, u64_t minute, u64_t second)
{
    // display.rtcSetAlarmEpoch(display.rtcGetEpoch() + 10, RTC_ALARM_MATCH_DHHMMSS);
    display.rtcClearAlarmFlag();
    u64_t t = 0;
    if (second > 0)
    {
        t = second;
    }
    if (minute > 0)
    {
        t += minute * 60;
    }
    if (hour > 0)
    {
        t += hour * 60 * 60;
    }
    Serial.printf("Going to sleep: %d, %lld", t, t * uS_TO_S_FACTOR);
    // display.rtcSetAlarmEpoch(display.rtcGetEpoch() + 30, RTC_ALARM_MATCH_DHHMMSS);
    esp_sleep_enable_timer_wakeup(t * uS_TO_S_FACTOR);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, LOW);
    esp_deep_sleep_start();
}
void showTime()
{
    char yy[5];
    char MM[3];
    char dd[3];
    char hh[3];
    char mm[3];
    sprintf(yy, "%d", display.rtcGetYear());
    sprintf(MM, "%02d", display.rtcGetMonth() + 1);
    sprintf(dd, "%02d", display.rtcGetDay());

    // sprintf(yy, "%d", 8888);
    // sprintf(MM, "%02d", 88);
    // sprintf(dd, "%02d", 88);

    sprintf(hh, "%02d", display.rtcGetHour());
    sprintf(mm, "%02d", display.rtcGetMinute());
    // sprintf(hh, "%02d", 88);
    // sprintf(mm, "%02d", 88);
    Serial.printf("%s.%s.%s %s:%s\n", yy, MM, dd, hh, mm);
    display.clearDisplay();
    display.setFont(&Helvetica_Neue_Condensed_Bold90pt7b);
    display.setTextSize(3);

    u8_t backgroundColor = 4;
    u8_t rectangles = 0;
    u8_t digitsColor = 7;
    display.fillRect(0, 0, 1200, 825, backgroundColor);

    u16_t clockW = 280;
    u8_t clockH = 220;
    u8_t margin = 10;
    u8_t radius = 30;
    u8_t thickLine = 5;
    u16_t timeTextHeightPosition = 413;

    display.fillRoundRect(margin, margin, clockW, clockH, radius, rectangles);
    display.fillRoundRect(300, margin, clockW, clockH, radius, rectangles);
    display.fillRoundRect(620, margin, clockW, clockH, radius, rectangles);
    display.fillRoundRect(910, margin, clockW, clockH, radius, rectangles);

    display.fillRoundRect(margin, clockH + margin + thickLine, clockW, clockH, radius, rectangles);
    display.fillRoundRect(300, clockH + margin + thickLine, clockW, clockH, radius, rectangles);
    display.fillRoundRect(620, clockH + margin + thickLine, clockW, clockH, radius, rectangles);
    display.fillRoundRect(910, clockH + margin + thickLine, clockW, clockH, radius, rectangles);

    display.setTextColor(digitsColor);
    display.setCursor(23, timeTextHeightPosition);
    display.print(hh[0]);
    display.setCursor(312, timeTextHeightPosition);
    display.print(hh[1]);
    display.setCursor(632, timeTextHeightPosition);
    display.print(mm[0]);
    display.setCursor(922, timeTextHeightPosition);
    display.print(mm[1]);
    display.drawThickLine(0, clockH + margin, 1200, clockH + margin, backgroundColor, thickLine);

    clockW = 130;
    clockH = 88;
    margin = 10;
    radius = 15;
    thickLine = 4;
    timeTextHeightPosition = 640;

    display.fillRoundRect(margin, timeTextHeightPosition, clockW, clockH, radius, rectangles);
    display.fillRoundRect((margin * 2) + clockW, timeTextHeightPosition, clockW, clockH, radius, rectangles);

    display.fillRoundRect((margin + clockW) * 2 + margin * 5 - 2, timeTextHeightPosition, clockW, clockH, radius, rectangles);
    display.fillRoundRect((margin + clockW) * 3 + margin * 5 - 2, timeTextHeightPosition, clockW, clockH, radius, rectangles);

    display.fillRoundRect((margin + clockW) * 4 + margin * 8 + 5, timeTextHeightPosition, clockW, clockH, radius, rectangles);
    display.fillRoundRect((margin + clockW) * 5 + margin * 8 + 5, timeTextHeightPosition, clockW, clockH, radius, rectangles);
    display.fillRoundRect((margin + clockW) * 6 + margin * 8 + 5, timeTextHeightPosition, clockW, clockH, radius, rectangles);
    display.fillRoundRect((margin + clockW) * 7 + margin * 8 + 5, timeTextHeightPosition, clockW, clockH, radius, rectangles);

    // second squares
    timeTextHeightPosition = 732;
    display.fillRoundRect(margin, timeTextHeightPosition, clockW, clockH, radius, rectangles);
    display.fillRoundRect((margin * 2) + clockW, timeTextHeightPosition, clockW, clockH, radius, rectangles);

    display.fillRoundRect((margin + clockW) * 2 + margin * 5, timeTextHeightPosition, clockW, clockH, radius, rectangles);
    display.fillRoundRect((margin + clockW) * 3 + margin * 5, timeTextHeightPosition, clockW, clockH, radius, rectangles);

    display.fillRoundRect((margin + clockW) * 4 + margin * 8 + 5, timeTextHeightPosition, clockW, clockH, radius, rectangles);
    display.fillRoundRect((margin + clockW) * 5 + margin * 8 + 5, timeTextHeightPosition, clockW, clockH, radius, rectangles);
    display.fillRoundRect((margin + clockW) * 6 + margin * 8 + 5, timeTextHeightPosition, clockW, clockH, radius, rectangles);
    display.fillRoundRect((margin + clockW) * 7 + margin * 8 + 5, timeTextHeightPosition, clockW, clockH, radius, rectangles);
    // text
    timeTextHeightPosition = 798;
    display.setFont(&Helvetica_Neue_Condensed_Bold35pt7b);
    display.setTextSize(3);

    // display.setCursor(627, timeTextHeightPosition);
    display.setCursor(24, timeTextHeightPosition);
    display.print(dd[0]);
    // display.setCursor(767, timeTextHeightPosition);
    display.setCursor(166, timeTextHeightPosition);
    display.print(dd[1]);

    display.setCursor(344, timeTextHeightPosition);
    display.print(MM[0]);
    display.setCursor(484, timeTextHeightPosition);
    display.print(MM[1]);

    display.setCursor(660, timeTextHeightPosition);
    display.print(yy[0]);
    display.setCursor(800, timeTextHeightPosition);
    display.print(yy[1]);
    display.setCursor(941, timeTextHeightPosition);
    display.print(yy[2]);
    display.setCursor(1080, timeTextHeightPosition);
    display.print(yy[3]);
    // line
    display.drawThickLine(0, 728, 1200, 728, backgroundColor, thickLine);

    clockW = 125;
    clockH = 80;
    margin = 7;
    radius = 15;
    thickLine = 4;
    timeTextHeightPosition = 466;

    u16_t t = 590;
    // display.setFont(&Helvetica_Neue_Condensed_Bold75pt7b);
    display.setFont(&JetBrainsMono_ExtraBold38pt7b);
    // display.setTextColor(0);
    display.setTextSize(2);

    char day[15];
    // sprintf(day, "%1s%s%1s", " ", namesOfWeekDaysSmall[display.rtcGetWeekday()], " ");
    // if (display.rtcGetWeekday() != 3)
    // {
    //     sprintf(day, " %s  ", namesOfWeekDaysSmall[display.rtcGetWeekday()]);
    // }
    // else
    // {
    //     sprintf(day, "%s", namesOfWeekDaysSmall[display.rtcGetWeekday()]);
    // }
    u8_t numDayOfWeek = display.rtcGetWeekday();
    sprintf(day, "%s    ", namesOfWeekDaysSmall[numDayOfWeek]);

    // squares day of week
    for (u8_t i = 0; i < 9; i++)
    {
        display.fillRoundRect((margin + clockW) * i + margin + 4, timeTextHeightPosition, clockW, clockH, radius, rectangles);
        display.fillRoundRect((margin + clockW) * i + margin + 4, timeTextHeightPosition + thickLine + clockH, clockW, clockH, radius, rectangles);
        display.setCursor((margin + clockW) * i + 28, t);
        display.print(day[i]);
    }
    display.drawThickLine(0, 545, 1200, 545, backgroundColor, thickLine);

    // day of week

    display.display();

    // if (getLocalTime(&myTimeInfo))
    // {
    //     char mbstr[3];
    //     strftime(mbstr, sizeof(mbstr), "%V", &myTimeInfo);
    //     display.rtcSetWeekOfYear(atoi(mbstr));
    //     display.rtcSetTime(myTimeInfo.tm_hour, myTimeInfo.tm_min, myTimeInfo.tm_sec);
    //     display.rtcSetDate(myTimeInfo.tm_wday, myTimeInfo.tm_mday, myTimeInfo.tm_mon, 1900 + myTimeInfo.tm_year);
    //     display.rtcGetRtcData();

    //     Serial.printf("\nTIME: %d.%02d.%02d %d:(%s) %02d:%02d:%02d W(%d)\n",
    //                   1900 + myTimeInfo.tm_year,
    //                   myTimeInfo.tm_mon,
    //                   myTimeInfo.tm_mday,
    //                   myTimeInfo.tm_wday,
    //                   namesOfWeekDays[myTimeInfo.tm_wday],
    //                   myTimeInfo.tm_hour, myTimeInfo.tm_min, myTimeInfo.tm_sec,
    //                   atoi(mbstr));
    //     Serial.printf("RTC : %d.%02d.%02d %d:(%s) %02d:%02d:%02d W(%d)\n",
    //                   display.rtcGetYear(),
    //                   display.rtcGetMonth(),
    //                   display.rtcGetDay(),
    //                   display.rtcGetWeekday(),
    //                   namesOfWeekDays[display.rtcGetWeekday()],
    //                   display.rtcGetHour(),
    //                   display.rtcGetMinute(),
    //                   display.rtcGetSecond(),
    //                   display.rtcGetWeekOfYear());
    // }
}