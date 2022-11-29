#ifndef WC_LANGUAGES_H
#define WC_LANGUAGES_H

#include "wled.h"

#define german2

#ifdef german
    #define WC_LANG "german"
    // PREFIX
    const int8_t maskPrefix[11] = {0, 1, 3, 4, 5, -1, -1, -1, -1, -1, -1};
    // MINUTES
    const int8_t maskMinutes[12][11] = {
        {107, 108, 109, -1, -1, -1, -1, -1, -1, -1, -1},
        {7, 8, 9, 10, 35, 36, 37, 38, -1, -1, -1},
        {11, 12, 13, 14, 35, 36, 37, 38, -1, -1, -1},
        {26, 27, 28, 29, 30, 31, 32, 35, 36, 37, 38},
        {15, 16, 17, 18, 19, 20, 21, 35, 36, 37, 38},
        {7, 8, 9, 10, 39, 40, 41, 44, 45, 46, 47},
        {44, 45, 46, 47, -1, -1, -1, -1, -1, -1, -1},
        {7, 8, 9, 10, 35, 36, 37, 38, 44, 45, 46},
        {15, 16, 17, 18, 19, 20, 21, 39, 40, 41, -1},
        {26, 27, 28, 29, 30, 31, 32, 39, 40, 41, -1},
        {11, 12, 13, 14, 39, 40, 41, -1, -1, -1, -1},
        {7, 8, 9, 10, 39, 40, 41, -1, -1, -1, -1}
    };
    // HOURS
    const int8_t maskHours[13][11] = {
        {49, 50, 51, 52, 53, -1, -1, -1, -1, -1, -1},
        {57, 58, 59, -1, -1, -1, -1, -1, -1, -1, -1},
        {55, 56, 57, 58, -1, -1, -1, -1, -1, -1, -1},
        {67, 68, 69, 70, -1, -1, -1, -1, -1, -1, -1},
        {84, 85, 86, 87, -1, -1, -1, -1, -1, -1, -1},
        {73, 74, 75, 76, -1, -1, -1, -1, -1, -1, -1},
        {100, 101, 102, 103, 104, -1, -1, -1, -1, -1, -1},
        {60, 61, 62, 63, 64, 65, -1, -1, -1, -1, -1},
        {89, 90, 91, 92, -1, -1, -1, -1, -1, -1, -1},
        {80, 81, 82, 83, -1, -1, -1, -1, -1, -1, -1},
        {93, 94, 95, 96, -1, -1, -1, -1, -1, -1, -1},
        {77, 78, 79, -1, -1, -1, -1, -1, -1, -1, -1},
        {49, 50, 51, 52, 53, -1, -1, -1, -1, -1, -1}
    };
#endif

#ifdef german2
    #define WC_LANG "german2"
    // PREFIX
    const int8_t maskPrefix[11] = {0, 1, 3, 4, 5, -1, -1, -1, -1, -1, -1};
    // MINUTES
    const int8_t maskMinutes[12][11] = {
        {107, 108, 109, -1, -1, -1, -1, -1, -1, -1, -1},
        {7, 8, 9, 10, 40, 41, 42, 43, -1, -1, -1},
        {11, 12, 13, 14, 40, 41, 42, 43, -1, -1, -1},
        {26, 27, 28, 29, 30, 31, 32, 40, 41, 42, 43},
        {15, 16, 17, 18, 19, 20, 21, 40, 41, 42, 43},
        {7, 8, 9, 10, 33, 34, 35, 44, 45, 46, 47},
        {44, 45, 46, 47, -1, -1, -1, -1, -1, -1, -1},
        {7, 8, 9, 10, 40, 41, 42, 43, 44, 45, 46},
        {15, 16, 17, 18, 19, 20, 21, 33, 34, 35, -1},
        {26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1},
        {11, 12, 13, 14, 33, 34, 35, -1, -1, -1, -1},
        {7, 8, 9, 10, 33, 34, 35, -1, -1, -1, -1}
    };
    // HOURS
    const int8_t maskHours[14][11] = {
        {94, 95, 96, 97, 98, -1, -1, -1, -1, -1, -1},
        {55, 56, 57, -1, -1, -1, -1, -1, -1, -1, -1},
        {62, 63, 64, 65, -1, -1, -1, -1, -1, -1, -1},
        {66, 67, 68, 69, -1, -1, -1, -1, -1, -1, -1},
        {73, 74, 75, 76, -1, -1, -1, -1, -1, -1, -1},
        {51, 52, 53, 54, -1, -1, -1, -1, -1, -1, -1},
        {77, 78, 79, 80, 81, -1, -1, -1, -1, -1, -1},
        {88, 89, 90, 91, 92, 93, -1, -1, -1, -1, -1},
        {84, 85, 86, 87, -1, -1, -1, -1, -1, -1, -1},
        {102, 103, 104, 105, -1, -1, -1, -1, -1, -1, -1},
        {99, 100, 101, 102, -1, -1, -1, -1, -1, -1, -1},
        {49, 50, 51, -1, -1, -1, -1, -1, -1, -1, -1},
        {94, 95, 96, 97, 98, -1, -1, -1, -1, -1, -1},
        {55, 56, 57, 58, -1, -1, -1, -1, -1, -1, -1}
    };

    uint8_t getHourIndex(uint8_t my_hour, uint8_t my_minute) {
        my_hour += my_minute/5 > 4 ? 1 : 0;
        if(my_hour = 1 && ((my_minute/5) != 0)) my_hour = 13; // eins
        return my_hour;
    }

#endif

#endif