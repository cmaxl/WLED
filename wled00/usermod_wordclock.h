#pragma once

#include "wled.h"

#include "wc_languages.h"
#include "wc_symbols.h"

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
class WordClock : public Usermod {
  private:
    //Private class members. You can declare variables and functions only accessible to your usermod here
    const int8_t maskMinuteDots[4] = {110, 111, 112, 113};
    uint8_t 
      languageIndex = 0,
      maskLEDsOn[114] = {0},
      maskLEDs_previous[114] = {0};
    int8_t maskBuffer[WQ_MASK_SIZE] = {0};

    const uint8_t numberOfLanguages = 5;
    static constexpr WordclockLanguage *wc_language_list[] = {
      &language_en,
      &language_de,
      &language_de_alt,
      // &language_d3,
      &language_d4,
      &language_d4_alt
    };
    WordclockLanguage *language = wc_language_list[languageIndex];
    String languageId = language->id;

    bool
      enabled = true,
      initDone = false,
      purist = true,
      doTransition = false, // enable transition effect
      inTransition = false; // transition in progress

    static const char _name[];
    static const char _shorthand[];
    static const char _enabled[];
    static const char _on[];
    static const char _language[];

    // writes the maskLEDsOn array with the LEDs that are relevant based on the
    // wordMask array. The wordMask array contains the indices of the LEDs.
    // Essentially writes one row of the matrix to the maskLEDsOn array.
    void writeBufferToMask() {
      for (int8_t i=0; i < WQ_MASK_SIZE; i++) {
        if(maskBuffer[i] >= 0) maskLEDsOn[maskBuffer[i]] = 1;
        else break;
      }
    }

    void updateDisplay(){
      uint8_t my_hour=hourFormat12(localTime);
      uint8_t my_minute=minute(localTime);

      // add maskLEDsOn to maskLEDs_previous by or-ing them
      for (uint8_t i=0; i<114; i++) maskLEDs_previous[i] |= maskLEDsOn[i]; 

      //clear maskLEDsOn
      memset(maskLEDsOn, 0, sizeof(maskLEDsOn));

      language = wc_language_list[languageIndex];

      uint8_t hourIndex = language->getHourIndex(my_hour, my_minute);
      uint8_t minuteIndex = my_minute/5;

      // PREFIX 
      //  purist mode: every full half hour (HH:3m or HH:0m)
      //  otherwise: always on
      if(!purist || !(minuteIndex%6)) {
        memcpy_P(maskBuffer, language->maskPrefix, WQ_MASK_SIZE);
        writeBufferToMask();
      }

      // MINUTES
      memcpy_P(maskBuffer, &(language->maskMinutes[minuteIndex]), WQ_MASK_SIZE);
      writeBufferToMask();

      // HOURS
      memcpy_P(maskBuffer, &(language->maskHours[hourIndex]), WQ_MASK_SIZE);
      writeBufferToMask();

      // MINUTE DOTS
      for (uint8_t i=0; i<(my_minute%5); i++) 
        maskLEDsOn[maskMinuteDots[i]] = 1;

      doTransition = true;

    }

    void changeLanguage(String newLang) {
      for (uint8_t i = 0; i < numberOfLanguages; i++) {
        if (newLang == wc_language_list[i]->id) {
          languageIndex = i;
          language = wc_language_list[languageIndex];
          languageId = language->id;
          updateDisplay();
          return;
        }
      }
    }
    

  public:
    //Functions called by WLED

    /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
    void setup() {
      updateDisplay();
      initDone = true;
    }


    /*
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
    void connected() {
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
      // if usermod is disabled or called during strip updating just exit
      // NOTE: on very long strips strip.isUpdating() may always return true so update accordingly
      if (!enabled || strip.isUpdating()) return;

      if(inTransition) strip.trigger();

      static uint32_t lastDisplayUpdate = 0;
      if(millis() - lastDisplayUpdate > 200) {
        lastDisplayUpdate = millis();
        int8_t my_minute = minute(localTime);
        int8_t my_hour = hourFormat12(localTime);
        static int8_t lastMinute = -1;
        static int8_t lastHour = -1;
        if(lastMinute != my_minute || lastHour != my_hour) {
          updateDisplay();
          lastMinute = my_minute;
          lastHour = my_hour;
        }
      }
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

      JsonArray infoArr = user.createNestedArray(FPSTR(_name));

      // On-Off
      String uiDomString = F("<button class=\"btn btn-xs\" onclick=\"requestJson({");
      uiDomString += FPSTR(_shorthand);
      uiDomString += F(":{");
      uiDomString += FPSTR(_on);
      uiDomString += enabled ? F(":false}});\">") : F(":true}});\">");
      uiDomString += F("<i class=\"icons");
      uiDomString += enabled ? F(" on") : F(" off");
      uiDomString += F("\">&#xe08f;</i>");
      uiDomString += F("</button>");
      infoArr.add(uiDomString);

      // Language
      infoArr = user.createNestedArray(FPSTR(_language));
      uiDomString = F("<select class=\"form-control\" onchange=\"requestJson({");
      uiDomString += FPSTR(_shorthand);
      uiDomString += F(":{language:this.value}})\">");
      for (uint8_t i = 0; i < numberOfLanguages; i++) {
        uiDomString += F("<option value=\"");
        uiDomString += wc_language_list[i]->id;
        uiDomString += F("\"");
        if (i == languageIndex) uiDomString += F(" selected");
        uiDomString += F(">");
        uiDomString += wc_language_list[i]->id;
        uiDomString += F(" - ");
        uiDomString += wc_language_list[i]->name;
        uiDomString += F("</option>");
      }
      infoArr.add(uiDomString);

      // Time setter gets currently set time (from server via json/info) 
      // and in/decreases to next 5 min or hour
      // then sends the new time to the server (requestJson({time: unixTime}))
      infoArr = user.createNestedArray(F("Time"));

      uiDomString = F("<button class=\"btn btn-xs\" onclick=\"");
            uiDomString += F("requestJson();");
            uiDomString += F("var d = new Date(lastinfo.time);");
            uiDomString += F("var t = Math.floor(d.getTime()/1000);");
            uiDomString += F("t -= 3600;");
            uiDomString += F("requestJson({time:t})\">");
        uiDomString += F("--");
      uiDomString += F("</button>");
      uiDomString += F("<button class=\"btn btn-xs\" onclick=\"");
            uiDomString += F("requestJson();");
            uiDomString += F("var d = new Date(lastinfo.time);");
            uiDomString += F("var t = Math.floor(d.getTime()/1000);");
            uiDomString += F("t -= t%300 + 300;");
            uiDomString += F("requestJson({time:t})\">");
        uiDomString += F("-");
      uiDomString += F("</button>");

      // uiDomString += F("<button class=\"btn btn-xs\" onclick=\"");
      //       uiDomString += F("var d = new Date();");
      //       uiDomString += F("var t = Math.floor(d.getTime()/1000);");
      //       uiDomString += F("requestJson({time:t})\">");
      //   uiDomString += F("now");
      // uiDomString += F("</button>");

      uiDomString += F("<button class=\"btn btn-xs\" onclick=\"");
            uiDomString += F("requestJson();");
            uiDomString += F("var d = new Date(lastinfo.time);");
            uiDomString += F("var t = Math.floor(d.getTime()/1000);");
            uiDomString += F("t -= t%300 - 300;");
            uiDomString += F("requestJson({time:t})\">");
        uiDomString += F("+");
      uiDomString += F("</button>");
      uiDomString += F("<button class=\"btn btn-xs\" onclick=\"");
            uiDomString += F("requestJson();");
            uiDomString += F("var d = new Date(lastinfo.time);");
            uiDomString += F("var t = Math.floor(d.getTime()/1000);");
            uiDomString += F("t += 3600;");
            uiDomString += F("requestJson({time:t})\">");
        uiDomString += F("++");
      uiDomString += F("</button>");


      infoArr.add(uiDomString);

    }


    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void addToJsonState(JsonObject& root)
    {
      if (!initDone) return;  // prevent crash on boot applyPreset()

      JsonObject usermod = root[FPSTR(_shorthand)];
      if(!root.containsKey(FPSTR(_shorthand)))  {
        if (usermod.isNull()) usermod = root.createNestedObject(FPSTR(_shorthand));
      }

      usermod[FPSTR(_language)] = languageId;

    }

    /*
     * addToJsonPreset() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * In contrast to addToJsonState the entries are added to the preset and will be considered in readFromJsonState
     * Useful if you need your usermod to be enabled and disabled by presets
     * Important to note is that the structure used must be consitent with the structure in readFromJsonState to take any affect
     * It is advised to use a shorthand for the key to reduce the footprint of the preset
     * Values in the state object may be modified by connected clients
     */
    void addToJsonPreset(JsonObject& root)
    {
      if (!initDone) return;  // prevent crash on boot applyPreset()

      JsonObject usermod = root[FPSTR(_shorthand)];
      if(!root.containsKey(FPSTR(_shorthand)))  {
        if (usermod.isNull()) usermod = root.createNestedObject(FPSTR(_shorthand));
      }
      
      usermod[FPSTR(_on)] = enabled;
    }


    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject& root)
    {
      if (!initDone) return;  // prevent crash on boot applyPreset()

      JsonObject usermod = root[FPSTR(_shorthand)];
      if (!usermod.isNull()) {
        enabled = usermod[FPSTR(_on)] | enabled;
        if (!usermod[FPSTR(_language)].isNull()) {
          changeLanguage(usermod[FPSTR(_language)]);
        }
      }
      //   if (usermod[FPSTR(_on)].is<bool>()) {
      //     enabled = usermod[FPSTR(_on)].as<bool>();
      //   }
      //   if (usermod[_language].is<String>()) {
      //     languageId = usermod[_language].as<String>();
      //     for (uint8_t i = 0; i < sizeof(wc_language_list)/sizeof(wc_language_list[0]); i++) {
      //       if (languageId == wc_language_list[i]->id) {
      //         languageIndex = i;
      //         break;
      //       }
      //     }
      //   }
      // }
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
     * addToConfig() will make your settings editable ^through the Usermod Settings page automatically.
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
      JsonObject top = root.createNestedObject(FPSTR(_name));

      top[FPSTR(_enabled)] = enabled;
      top["puristMode"] = purist;
      top["language"] = languageId;
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
      JsonObject top = root[FPSTR(_name)];

      bool configComplete = !top.isNull();

      configComplete &= getJsonValue(top[FPSTR(_enabled)], enabled);
      configComplete &= getJsonValue(top["puristMode"], purist);

      String tmp;
      configComplete &= getJsonValue(top["language"], tmp);
      if (configComplete) changeLanguage(tmp);

      return configComplete;
    }


    /*
     * appendConfigData() is called when user enters usermod settings page
     * it may add additional metadata for certain entry fields (adding drop down is possible)
     * be careful not to add too much as oappend() buffer is limited to 3k
     */
    void appendConfigData()
    {
      oappend(SET_F("dd=addDropdown('")); oappend(String(FPSTR(_name)).c_str()); oappend(SET_F("','language');"));
      for (uint8_t i = 0; i < numberOfLanguages; i++) {
        oappend(SET_F("addOption(dd,'"));
        // example: addOption(dd,'EN (english)','EN');
        oappend(wc_language_list[i]->id);
        oappend(SET_F(" ("));
        oappend(wc_language_list[i]->name);
        oappend(SET_F(")','"));
        oappend(wc_language_list[i]->id);
        oappend(SET_F("');"));
      }
    }


    /*
     * handleOverlayDraw() is called just before every show() (LED strip update frame) after effects have set the colors.
     * Use this to blank out some LEDs or set them to a different color regardless of the set effect mode.
     * Commonly used for custom clocks (Cronixie, 7 segment)
     */
    void handleOverlayDraw()
    {
      if(!enabled) return;

      // TODO: add background
      static uint32_t transitionStart = 0;
      static uint8_t transitionStep = 0;
      if (inTransition || doTransition) {
        // executed once a transition is triggered
        if (doTransition) {
          inTransition = true;
          doTransition = false;
          transitionStart = millis();
        }
        // stop transition after 500ms
        if (millis() - transitionStart > 500) {
          inTransition = false;
          memset(maskLEDs_previous, 0, sizeof(maskLEDsOn));
        }

        transitionStep = (millis() - transitionStart) >> 1;
        if (transitionStep > 255) transitionStep = 255;
      }

      // loop over all LEDs relevant to the clock
      // copy each color from the virtual strip as the background (will be black if not configured)
      for (int i=0; i<114; i++) {
        
        // continue if LED is already on and will stay on in a transition
        if(maskLEDsOn[i] || (inTransition && maskLEDsOn[i] && maskLEDs_previous[i])) {
          continue;
        }

        // copy color from virtual to physical strip (if not a dot) and apply transition
        uint32_t bg = (i < 110) ? strip.getPixelColor(i + 121) : 0;

        if (inTransition) {
          uint32_t pxl = (i < 110) ? strip.getPixelColor(i) : strip.getPixelColor(i);
          // Pixel needs fade in
          if(maskLEDsOn[i] && !maskLEDs_previous[i]) {
            bg = color_blend(bg, pxl, transitionStep);
          } 
          // Pixel needs fade out
          if(!maskLEDsOn[i] && maskLEDs_previous[i]) {
            bg = color_blend(pxl, bg, transitionStep);
          }
        }
        strip.setPixelColor(i, bg);
      }
    }

   
    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId()
    {
      return USERMOD_ID_EXAMPLE;
    }

   //More methods can be added in the future, this example will then be extended.
   //Your usermod will remain compatible as it does not need to implement all methods from the Usermod base class!
};

// strings to reduce flash memory usage (used more than twice)
const char WordClock::_name[]       PROGMEM = "Wordclock";
const char WordClock::_shorthand[]  PROGMEM = "wq";
const char WordClock::_enabled[]    PROGMEM = "enabled";
const char WordClock::_on[]         PROGMEM = "on";
const char WordClock::_language[]   PROGMEM = "language";