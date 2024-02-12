#pragma once

#include "wled.h"

#include "airplanes.h"

/*
 * Usermods allow you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * 
 * This is an example for a v2 usermod.
 * v2 usermods are class inheritance based and can (but don't have to) implement more functions, each of them is shown in this example.
 * Multiple v2 usermods can be added to one compilation easily.
 * 
 * Creating a usermod:
 * This file serves as an example. If you want to create a usermod, it is recommended to use usermod_v2_empty.h from the usermods folder as a template.
 * Please remember to rename the class and file to a descriptive name.
 * You may also use multiple .h and .cpp files.
 * 
 * Using a usermod:
 * 1. Copy the usermod into the sketch folder (same folder as wled00.ino)
 * 2. Register the usermod by adding #include "usermod_filename.h" in the top and registerUsermod(new MyUsermodClass()) in the bottom of usermods_list.cpp
 */

//class name. Use something descriptive and leave the ": public Usermod" part :)
class AirplaneFlasher : public Usermod {
  private:
    //Private class members. You can declare variables and functions only accessible to your usermod here

  #ifdef USERMOD_FOUR_LINE_DISPLAY
    FourLineDisplayUsermod *display;
  #else
    void* display = nullptr;
  #endif

    uint8_t 
      airplaneChanged = false,
      beacon_size = 0,
      wing_size = 0,
      tail_size = 0,
      airplane_idx = 0;

    uint16_t 
      beacon[MAX_AIRP_SIZE] = {0},
      wing[MAX_AIRP_SIZE] = {0},
      tail[MAX_AIRP_SIZE] = {0},
      timeframe = 0;

    String 
      airplaneMake = "Make",
      airplaneName = "Airplane";

    void loadAirplane() {
      airplaneMake = airplanes[airplane_idx].make;
      airplaneName = airplanes[airplane_idx].name;
      timeframe = airplanes[airplane_idx].timeframe;
      beacon_size = airplanes[airplane_idx].beacon_size;
      wing_size = airplanes[airplane_idx].wing_size;
      tail_size = airplanes[airplane_idx].tail_size;
      for (uint8_t i = 0; i < MAX_AIRP_SIZE; i++) {
        beacon[i] = airplanes[airplane_idx].beacon[i];
        wing[i] = airplanes[airplane_idx].wing[i];
        tail[i] = airplanes[airplane_idx].tail[i];
      }
      airplaneChanged = true;

    #ifdef USERMOD_FOUR_LINE_DISPLAY
      if(millis()<5000) return; //skip overlay on boot

      if (display != nullptr) {
        // display->wakeDisplay()
        display->overlay(airplaneMake.c_str(), airplaneName.c_str(), 2000);
      }
      else DEBUG_PRINTLN("Display not found");
    #endif
      // display->sleepOrClock(true);
    }

    void nextAirplane() {
      airplane_idx = (airplane_idx + 1) % NUM_AIRPLANES;
      loadAirplane();
    }

    void randomAirplane() {
      airplane_idx = random(NUM_AIRPLANES);
      loadAirplane();
    }

  public:
    //Functions called by WLED

    /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
    void setup() {
      um_data = new um_data_t;
      um_data->u_size = 8;
      um_data->u_type = new um_types_t[um_data->u_size];
      um_data->u_data = new void*[um_data->u_size];
      um_data->u_type[0] = UMT_BYTE;
      um_data->u_data[0] = &airplaneChanged;
      um_data->u_type[1] = UMT_UINT16_ARR;
      um_data->u_data[1] = beacon;
      um_data->u_type[2] = UMT_UINT16_ARR;
      um_data->u_data[2] = wing;
      um_data->u_type[3] = UMT_UINT16_ARR;
      um_data->u_data[3] = tail;
      um_data->u_type[4] = UMT_BYTE;
      um_data->u_data[4] = &beacon_size;
      um_data->u_type[5] = UMT_BYTE;
      um_data->u_data[5] = &wing_size;
      um_data->u_type[6] = UMT_BYTE;
      um_data->u_data[6] = &tail_size;
      um_data->u_type[7] = UMT_UINT16;
      um_data->u_data[7] = &timeframe;

    #ifdef USERMOD_FOUR_LINE_DISPLAY
      // This Usermod uses FourLineDisplayUsermod for the best experience.
      // But it's optional. But you want it.
      display = (FourLineDisplayUsermod*) usermods.lookup(USERMOD_ID_FOUR_LINE_DISP);
    #endif

      loadAirplane();
    }


    /*
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
    void connected() {
      //Serial.println("Connected to WiFi!");
    }


    /*
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     * 
     * Tips:
     * 1. You can use "if (WLED_CONNECTED)" to check for a successful network connection.
     *    Additionally, "if (WLED_MQTT_CONNECTED)" is available to check for a connection to an MQTT broker.
     * 
     * 2. Try to avoid using the delay() function. NEVER use delays longer than 10 milliseconds.
     *    Instead, use a timer check as shown here.
     */
    void loop() {
      
    }

    bool getUMData(um_data_t **data)
    {
      if (!data) return false; // no pointer provided by caller
      *data = um_data;
      return true;
    }

    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    void addToJsonInfo(JsonObject& root)
    {
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      JsonArray infoArr = user.createNestedArray("Airplane");
      infoArr.add(airplaneMake + " " +airplaneName);


      infoArr = user.createNestedArray("index: " + String(airplane_idx)); //name
      String uiDomString = F("<button class=\"btn btn-p\" onclick=\"requestJson({next:");
      uiDomString += "1});\">";
      uiDomString += "Next";
      uiDomString += "</button>";
      uiDomString += F("<button class=\"btn btn-p\" onclick=\"requestJson({random:");
      uiDomString += "1});\">";
      uiDomString += "Random";
      uiDomString += "</button>";
      infoArr.add(uiDomString);
    }


    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void addToJsonState(JsonObject& root)
    {
      root["next"] = 0;
      root["random"] = 0;
    }


    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject& root)
    {
      if (root["next"] != nullptr)
      {
        if(root["next"]) {
          nextAirplane();
          root["next"] = 0;
        }
      }
      if (root["random"] != nullptr)
      {
        if(root["random"]) {
          randomAirplane();
          root["random"] = 0;
        }
      }
    }


    /*
     * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
     * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
     * If you want to force saving the current state, use serializeConfig() in your loop().
     * 
     * CAUTION: serializeConfig() will initiate a filesystem write operation.
     * It might cause the LEDs to stutter and will cause flash wear if called too often.
     * Use it sparingly and always in the loop, never in network callbacks!
     * 
     * addToConfig() will make your settings editable through the Usermod Settings page automatically.
     *
     * Usermod Settings Overview:
     * - Numeric values are treated as floats in the browser.
     *   - If the numeric value entered into the browser contains a decimal point, it will be parsed as a C float
     *     before being returned to the Usermod.  The float data type has only 6-7 decimal digits of precision, and
     *     doubles are not supported, numbers will be rounded to the nearest float value when being parsed.
     *     The range accepted by the input field is +/- 1.175494351e-38 to +/- 3.402823466e+38.
     *   - If the numeric value entered into the browser doesn't contain a decimal point, it will be parsed as a
     *     C int32_t (range: -2147483648 to 2147483647) before being returned to the usermod.
     *     Overflows or underflows are truncated to the max/min value for an int32_t, and again truncated to the type
     *     used in the Usermod when reading the value from ArduinoJson.
     * - Pin values can be treated differently from an integer value by using the key name "pin"
     *   - "pin" can contain a single or array of integer values
     *   - On the Usermod Settings page there is simple checking for pin conflicts and warnings for special pins
     *     - Red color indicates a conflict.  Yellow color indicates a pin with a warning (e.g. an input-only pin)
     *   - Tip: use int8_t to store the pin value in the Usermod, so a -1 value (pin not set) can be used
     *
     * See usermod_v2_auto_save.h for an example that saves Flash space by reusing ArduinoJson key name strings
     * 
     * If you need a dedicated settings page with custom layout for your Usermod, that takes a lot more work.  
     * You will have to add the setting to the HTML, xml.cpp and set.cpp manually.
     * See the WLED Soundreactive fork (code and wiki) for reference.  https://github.com/atuline/WLED
     * 
     * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
     */
    void addToConfig(JsonObject& root)
    {
      
    }


    /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens immediately after boot, or after saving on the Usermod Settings page)
     * 
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     * 
     * Return true in case the config values returned from Usermod Settings were complete, or false if you'd like WLED to save your defaults to disk (so any missing values are editable in Usermod Settings)
     * 
     * getJsonValue() returns false if the value is missing, or copies the value into the variable provided and returns true if the value is present
     * The configComplete variable is true only if the "exampleUsermod" object and all values are present.  If any values are missing, WLED will know to call addToConfig() to save them
     * 
     * This function is guaranteed to be called on boot, but could also be called every time settings are updated
     */
    bool readFromConfig(JsonObject& root)
    {
      // default settings values could be set here (or below using the 3-argument getJsonValue()) instead of in the class definition or constructor
      // setting them inside readFromConfig() is slightly more robust, handling the rare but plausible use case of single value being missing after boot (e.g. if the cfg.json was manually edited and a value was removed)

      JsonObject top = root["airplaneFlasher"];

      bool configComplete = !top.isNull();


      return configComplete;
    }


    /*
     * handleOverlayDraw() is called just before every show() (LED strip update frame) after effects have set the colors.
     * Use this to blank out some LEDs or set them to a different color regardless of the set effect mode.
     * Commonly used for custom clocks (Cronixie, 7 segment)
     */
    void handleOverlayDraw()
    {
      //strip.setPixelColor(0, RGBW32(0,0,0,0)) // set the first pixel to black
      
    }

   
    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId()
    {
      return USERMOD_ID_AIPLANE_FLASHER;
    }

   //More methods can be added in the future, this example will then be extended.
   //Your usermod will remain compatible as it does not need to implement all methods from the Usermod base class!
};