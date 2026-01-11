#pragma once

#include "wled.h"

#define WQ_MASK_SIZE 12

struct WordclockLanguage {
    const char *id;
    const char *name;
    const int8_t maskPrefix[WQ_MASK_SIZE];
    const int8_t maskMinutes[12][WQ_MASK_SIZE];
    const int8_t maskHours[14][WQ_MASK_SIZE];
    uint8_t (*getHourIndex)(uint8_t my_hour, uint8_t my_minute);
};

/*
available stencils/languages:
    EN      (english)
    DE      (german)
    D3      (swabian)
    D4      (swabian with DE stencil)
    DE_ALT  (german with alternative stencil)
    D4_ALT  (swabian with DE_ALT stencil)
*/

/*
EN (english)
    I T L I S A S A M P M
    A C Q U A R T E R D C 
    T W E N T Y F I V E X
    H A L F S T E N F T O
    P A S T E R U N I N E
    O N E S I X T H R E E
    F O U R F I V E T W O
    E I G H T E L E V E N
    S E V E N T W E L V E
    T E N S E O' C L O C K
*/
static WordclockLanguage language_en PROGMEM = {
    "EN", // id
    "english", // name
    {0, 1, 3, 4, -1}, // maskPrefix
    { // maskMinutes
        {104, 105, 106, 107, 108, 109, -1},
        {28, 29, 30, 31, 44, 45, 46, 47, -1}, 
        {38, 39, 40, 44, 45, 46, 47, -1}, 
        {13, 14, 15, 16, 17, 18, 19, 44, 45, 46, 47, -1}, 
        {22, 23, 24, 25, 26, 27, 44, 45, 46, 47, -1}, 
        {22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 44, -1}, 
        {33, 34, 35, 36, 44, 45, 46, 47, -1}, 
        {22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 42, -1}, 
        {22, 23, 24, 25, 26, 27, 42, 43, -1}, 
        {13, 14, 15, 16, 17, 18, 19, 42, 43, -1}, 
        {38, 39, 40, 42, 43, -1}, 
        {28, 29, 30, 31, 42, 43, -1},
    },
    { // maskHours
        {93, 94, 95, 96, 97, 98, -1}, 
        {55, 56, 57, -1}, 
        {74, 75, 76, -1}, 
        {61, 62, 63, 64, 65, -1}, 
        {66, 67, 68, 69, -1}, 
        {70, 71, 72, 73}, 
        {58, 59, 60, -1}, 
        {88, 89, 90, 91, 92, -1}, 
        {77, 78, 79, 80, 81, -1}, 
        {51, 52, 53, 54, -1}, 
        {99, 100, 101, -1}, 
        {82, 83, 84, 85, 86, 87, -1}, 
        {93, 94, 95, 96, 97, 98, -1},
    },
    [](uint8_t my_hour, uint8_t my_minute) -> uint8_t {
        my_hour += my_minute/5 >= 7 ? 1 : 0;
        return my_hour;
    }
};


/*
DE (german)
    E S K I S T A F Ü N F
    Z E H N Z W A N Z I G
    D R E I V I E R T E L
    V O R F U N K N A C H
    H A L B A E L F Ü N F
    E I N S X A M Z W E I
    D R E I P M J V I E R
    S E C H S N L A C H T
    S I E B E N Z W Ö L F
    Z E H N E U N K U H R
*/
static WordclockLanguage language_de PROGMEM = {
    "DE", // id
    "german", // name
    {0, 1, 3, 4, 5, -1}, // maskPrefix
    { // maskMinutes
        {107, 108, 109, -1},
        {7, 8, 9, 10, 40, 41, 42, 43, -1},
        {11, 12, 13, 14, 40, 41, 42, 43, -1},
        {26, 27, 28, 29, 30, 31, 32, 40, 41, 42, 43, -1},
        {15, 16, 17, 18, 19, 20, 21, 40, 41, 42, 43, -1},
        {7, 8, 9, 10, 33, 34, 35, 44, 45, 46, 47, -1},
        {44, 45, 46, 47, -1},
        {7, 8, 9, 10, 40, 41, 42, 43, 44, 45, 46, 47},
        {15, 16, 17, 18, 19, 20, 21, 33, 34, 35, -1},
        {26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1},
        {11, 12, 13, 14, 33, 34, 35, -1},
        {7, 8, 9, 10, 33, 34, 35, -1}
    },
    { // maskHours
        {94, 95, 96, 97, 98, -1},
        {55, 56, 57, -1},
        {62, 63, 64, 65, -1},
        {66, 67, 68, 69, -1},
        {73, 74, 75, 76, -1},
        {51, 52, 53, 54, -1},
        {77, 78, 79, 80, 81, -1},
        {88, 89, 90, 91, 92, 93, -1},
        {84, 85, 86, 87, -1},
        {102, 103, 104, 105, -1},
        {99, 100, 101, 102, -1},
        {49, 50, 51, -1},
        {94, 95, 96, 97, 98, -1},
        {55, 56, 57, 58, -1}
    },
    [](uint8_t my_hour, uint8_t my_minute) -> uint8_t {
        my_hour += my_minute/5 > 4 ? 1 : 0;
        if(my_hour == 1 && ((my_minute/5) != 0)) my_hour = 13; // eins
        return my_hour;
    }
};

/*
D3 (swabian)
    E S K I S C H F U N K
    D R E I V I E R T L A 
    Z E H N B I E F Ü N F
    N A C H G E R T V O R
    H A L B X F Ü N F E I 
    O I S E C H S E L F E 
    Z W O I E A C H T E D
    D R E I E Z W Ö L F E
    Z E H N E U N E U H L
    S I E B N E V I E R E 
*/
// not configured yet

/*
D4 (swabian)
    E S K I S T A F Ü N F
    Z E H N Z W A N Z I G
    D R E I V I E R T E L
    V O R F U N K N A C H
    H A L B A E L F Ü N F
    E I N S X A M Z W E I
    D R E I P M J V I E R
    S E C H S N L A C H T
    S I E B E N Z W Ö L F
    Z E H N E U N K U H R
*/
static WordclockLanguage language_d4 PROGMEM = {
    "D4",
    "swabian on DE layout",
    {0, 1, 3, 4, 5, -1},
    {
        {107, 108, 109, -1},                                    // UHR
        {7, 8, 9, 10, 40, 41, 42, 43, -1},                      // FÜNF NACH
        {11, 12, 13, 14, 40, 41, 42, 43, -1},                   // ZEHN NACH
        {26, 27, 28, 29, 30, 31, 32, -1},                       // VIERTEL
        {11, 12, 13, 14, 33, 34, 35, 44, 45, 46, 47, -1},       // ZEHN VOR HALB
        {7, 8, 9, 10, 33, 34, 35, 44, 45, 46, 47, -1},          // FÜNF VOR HALB
        {44, 45, 46, 47, -1},                                   // HALB
        {7, 8, 9, 10, 40, 41, 42, 43, 44, 45, 46, 47},          // FÜNF NACH HALB
        {15, 16, 17, 18, 19, 20, 21, 33, 34, 35, -1},           // ZWANZIG VOR
        {22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, -1},       // DREIVIERTEL
        {11, 12, 13, 14, 33, 34, 35, -1},                       // ZEHN VOR
        {7, 8, 9, 10, 33, 34, 35, -1},                          // FÜNF VOR
    },
    {
        {94, 95, 96, 97, 98, -1},       // ZWÖLF
        {55, 56, 57, -1},               // EIN
        {62, 63, 64, 65, -1},           // ZWEI
        {66, 67, 68, 69, -1},           // DREI
        {73, 74, 75, 76, -1},           // VIER
        {51, 52, 53, 54, -1},           // FÜNF
        {77, 78, 79, 80, 81, -1},       // SECHS
        {88, 89, 90, 91, 92, 93, -1},   // SIEBEN
        {84, 85, 86, 87, -1},           // ACHT
        {102, 103, 104, 105, -1},       // NEUN
        {99, 100, 101, 102, -1},        // ZEHN
        {49, 50, 51, -1,},              // ELF
        {94, 95, 96, 97, 98, -1},       // ZWÖLF
        {55, 56, 57, 58, -1},           // EINS
    },
    [](uint8_t my_hour, uint8_t my_minute) -> uint8_t {
        my_hour += my_minute/5 > 2 ? 1 : 0;
        if(my_hour == 1 && ((my_minute/5) != 0)) my_hour = 13; // eins
        return my_hour;
    }
};


/*
DE_ALT (german alternative stencil)
    E S K I S T L F Ü N F
    Z E H N Z W A N Z I G
    D R E I V I E R T E L
    T G N A C H V O R J M
    H A L B Q Z W Ö L F P
    Z W E I N S I E B E N
    K D R E I R H F Ü N F
    E L F N E U N V I E R
    W A C H T Z E H N R S
    B S E C H S F M U H R
*/
static WordclockLanguage language_de_alt PROGMEM = {
    "DE_ALT",
    "german alternative layout",
    {0, 1, 3, 4, 5, -1},
    {
        {107, 108, 109},
        {7, 8, 9, 10, 35, 36, 37, 38, -1},
        {11, 12, 13, 14, 35, 36, 37, 38, -1},
        {26, 27, 28, 29, 30, 31, 32, 35, 36, 37, 38, -1},
        {15, 16, 17, 18, 19, 20, 21, 35, 36, 37, 38, -1},
        {7, 8, 9, 10, 39, 40, 41, 44, 45, 46, 47, -1},
        {44, 45, 46, 47, -1},
        {7, 8, 9, 10, 35, 36, 37, 38, 44, 45, 46, -1},
        {15, 16, 17, 18, 19, 20, 21, 39, 40, 41, -1},
        {26, 27, 28, 29, 30, 31, 32, 39, 40, 41, -1},
        {11, 12, 13, 14, 39, 40, 41, -1},
        {7, 8, 9, 10, 39, 40, 41, -1}
    },
    {
        {49, 50, 51, 52, 53, -1},
        {57, 58, 59, -1},
        {55, 56, 57, 58, -1},
        {67, 68, 69, 70, -1},
        {84, 85, 86, 87, -1},
        {73, 74, 75, 76, -1},
        {100, 101, 102, 103, 104, -1},
        {60, 61, 62, 63, 64, 65, -1},
        {89, 90, 91, 92, -1},
        {80, 81, 82, 83, -1},
        {93, 94, 95, 96, -1},
        {77, 78, 79, -1},
        {49, 50, 51, 52, 53, -1},
        {55, 56, 57, 58, -1}
    },
    [](uint8_t my_hour, uint8_t my_minute) -> uint8_t {
        my_hour += my_minute/5 > 4 ? 1 : 0;
        if(my_hour == 1 && ((my_minute/5) != 0)) my_hour = 13; // eins
        return my_hour;
    }
};

/*
D4 (swabian alternative stencil)
    E S K I S T L F Ü N F
    Z E H N Z W A N Z I G
    D R E I V I E R T E L
    T G N A C H V O R J M
    H A L B Q Z W Ö L F P
    Z W E I N S I E B E N
    K D R E I R H F Ü N F
    E L F N E U N V I E R
    W A C H T Z E H N R S
    B S E C H S F M U H R
*/
static WordclockLanguage language_d4_alt PROGMEM = {
    "D4_ALT",
    "swabian alternative layout",
    {0, 1, 3, 4, 5, -1},
    {
        {107, 108, 109, -1},
        {7, 8, 9, 10, 35, 36, 37, 38, -1}, 
        {11, 12, 13, 14, 35, 36, 37, 38, -1}, 
        {26, 27, 28, 29, 30, 31, 32, -1},
        {11, 12, 13, 14, 39, 40, 41, 44, 45, 46, 47, -1},
        {7, 8, 9, 10, 39, 40, 41, 44, 45, 46, 47, -1},
        {44, 45, 46, 47, -1}, 
        {7, 8, 9, 10, 35, 36, 37, 38, 44, 45, 46, -1},
        {15, 16, 17, 18, 19, 20, 21, 39, 40, 41, -1}, 
        {22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, -1},
        {11, 12, 13, 14, 39, 40, 41, -1},
        {7, 8, 9, 10, 39, 40, 41, -1},
    },
    {
        {49, 50, 51, 52, 53, -1},
        {57, 58, 59, -1},
        {55, 56, 57, 58, -1}, 
        {67, 68, 69, 70, -1}, 
        {84, 85, 86, 87, -1}, 
        {73, 74, 75, 76, -1}, 
        {100, 101, 102, 103, 104, -1},
        {60, 61, 62, 63, 64, 65, -1}, 
        {89, 90, 91, 92, -1}, 
        {80, 81, 82, 83, -1}, 
        {93, 94, 95, 96, -1}, 
        {77, 78, 79, -1},
        {49, 50, 51, 52, 53, -1},
        {57, 58, 59, 60, -1}
    },
    [](uint8_t my_hour, uint8_t my_minute) -> uint8_t {
        my_hour += my_minute/5 > 2 ? 1 : 0;
        if(my_hour == 1 && ((my_minute/5) != 0)) my_hour = 13; // eins
        return my_hour;
    }
};

#define WQ_NUMBER_OF_LANGUAGES 5
WordclockLanguage* getLanguageByIndex(uint8_t idx) {
  switch(idx) {
    case 0: return &language_en;
    case 1: return &language_de;
    case 2: return &language_d4;
    case 3: return &language_de_alt;
    case 4: return &language_d4_alt;
  }
  return nullptr;
}
