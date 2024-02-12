#pragma once

#include "wled.h"

#define NUM_AIRPLANES 7
#define MAX_AIRP_SIZE 15

typedef struct Airplane {
    String      make;                   // make of the airplane
    String      name;                   // name of the airplane (model)
    uint16_t    beacon[MAX_AIRP_SIZE];  // beacon light pattern (ms)
    uint8_t     beacon_size;            // size of beacon light pattern
    uint16_t    wing[MAX_AIRP_SIZE];    // wing light pattern (ms)
    uint8_t     wing_size;              // size of wing light pattern
    uint16_t    tail[MAX_AIRP_SIZE];    // tail light pattern (ms)
    uint8_t     tail_size;              // size of tail light pattern
    uint16_t    timeframe;              // time each light pattern lasts (ms)
} airplane_t;

// define all parameters for each airplane
// if the pattern array starts with 0 then the light is off (for the following amount of ms)
// each pattern must last as long as specified in timeframe and can hold a maximum of MAX_AIRP_SIZE elements
extern airplane_t airplanes[NUM_AIRPLANES] = {
    {
        "Airbus",
        "Generic",
        {0, 500, 100, 400},
        4,
        {50, 50, 50, 850},
        4,
        {100, 900},
        2,
        1000
    }, 
    {
        "Every",
        "Taxilight",
        {200, 800},
        2,
        {0, 1000},
        2,
        {0, 1000},
        2,
        1000
    },
    {
        "Bombardier",
        "CRJ9000",
        {20, 0, 50, 1000, 50, 1000, 50, 1000, 50, 1000, 50, 1000},
        12,
        {50, 700, 50, 700, 50, 700, 50, 700, 50, 700, 50, 700, 50, 720},
        16,
        {50, 700, 50, 700, 50, 700, 50, 700, 50, 700, 50, 700, 50, 720},
        16,
        5270
    },
    {
        "Boeing",
        "777",
        {50, 1350},
        2,
        {0, 400, 50, 950},
        4,
        {0, 300, 50, 1050},
        4,
        1400
    },
    {
        "Boeing",
        "737",
        {300, 900},
        2,
        {0, 200, 300, 700},
        4,
        {0, 200, 300, 700},
        4,
        1200
    },
    {
        "Boeing",
        "787",
        {0, 500, 200, 700},
        4,
        {300, 1100},
        2,
        {300, 1100},
        2,
        1400
    },
    {
        "Boeing",
        "747",
        {0, 130, 50, 1020},
        4,
        {0, 80, 50, 1070},
        4,
        {50, 1150},
        2,
        1200
    }
};
