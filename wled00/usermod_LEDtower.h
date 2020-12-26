#pragma once

#include "wled.h"

//This is an empty v2 usermod template. Please see the file usermod_v2_example.h in the EXAMPLE_v2 usermod folder for documentation on the functions you can use!

#define TOWERHEIGHT 4 // number of LEDs in vertical direction
#define TOWERWIDTH 4 // number of LEDs in horizontal (around the tower) direction
#define WIRING 0 // number of LEDs in horizontal (around the tower) direction

//WLED_GLOBAL byte towerMode _INIT(0); // 0 = disabled, 1 = horizontal alignement, 2 = vertical alignemnt

class LEDtower : public Usermod {
  public:

  private:
    uint32_t color = 0;
    uint8_t
      // standard values for horizontal alignment
      master = TOWERHEIGHT,
      slave = TOWERWIDTH,
      masterIncrement = 1,
      slaveIncrement = TOWERHEIGHT,
      masterPxl = 0,
      startCopy = 1,
      incrCopy = 1;
    bool 
      updateConfig = false,
      invert = false,
      reversed = false;
    // WS2812FX::Segment& masterSeg = strip.getSegment(0);
    // WS2812FX::Segment* allSegs = strip.getSegments();

    // TODO: add restore function
    // void restoreSegments() {
    //   allSegs = strip.getSegments();
    //   for(uint16_t i=0; i < sizeof(allSegs); i++) {
    //     strip.setSegment(i) = allSegs[i];
    //   }
    // }

    // TODO: add reset all segments

    void copyPixels() {
        for(uint8_t i=0; i < master; i++) {
          masterPxl = i*masterIncrement;
          // compensate moved pixels when segment is reversed in towerMode 2
          if(strip.getSegment(0).getOption(SEG_OPTION_REVERSED) && towerMode == 2) {
            masterPxl += TOWERHEIGHT-1;
            reversed = true;
          }
          else { reversed = false; }

          color = strip.getPixelColor(masterPxl);

          switch(WIRING) {
            case 1: // bottom to top
              startCopy = 1;
              incrCopy = 1;
              invert = false;
              break;

            default: // zig zag
              switch(towerMode) {
                case 1: // horizontal
                  startCopy = 2;
                  incrCopy = 2;
                  invert = true; // odd columns in towerMode 1 must be copied in reverse
                  break;
                default: // vertical
                  startCopy = 1;
                  incrCopy = 1;
                  invert = false;
              }
              
          }
          
          //set all other pixels in the row/column to color
          for(uint8 ii=startCopy; ii < slave + startCopy-1; ii+=incrCopy) {
            if(!reversed) {
              strip.setPixelColor(masterPxl + (ii * slaveIncrement), color);
              if(invert) strip.setPixelColor(masterPxl + (ii * slaveIncrement - 1) - (i * 2), color);
            }
            else strip.setPixelColor(masterPxl + (ii * slaveIncrement)-TOWERHEIGHT, color);
              
          }
        }
        // strip.show();
    }

    void updateMode() {
      switch(towerMode){
        case 0:
          // restoreSegments();
          strip.setSegment(0, 0, TOWERHEIGHT*TOWERWIDTH, 1, 0);
          break;
        case 1: // horizontal alignment (all LEDs at the same height are the same)
          strip.setSegment(0, 0, TOWERHEIGHT, 1, 0);
          master = TOWERHEIGHT;
          slave = TOWERWIDTH;
          masterIncrement = 1;
          slaveIncrement = TOWERHEIGHT;
          break;

        default: // vertical alignment (all LEDs on one side are the same)
          strip.setSegment(0, 0, TOWERHEIGHT*TOWERWIDTH, 1, TOWERHEIGHT-1);
          master = TOWERWIDTH;
          slave = TOWERHEIGHT;
          masterIncrement = TOWERHEIGHT;
          slaveIncrement = 1;
      }
    }

  public:
    void setup() {
      updateMode();
    }

    void loop() {

      if(towerMode) {
        copyPixels();
      }

      if(updateConfig) {
        updateMode();
        serializeConfig();
        updateConfig = false;
      }
    }

    /**
   * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
   * 
   * Add LEDtower towerMode to jsoninfo
   */
  void addToJsonInfo(JsonObject &root)
  {
    //this code adds "u":{"LEDtower sync":uiDomString} to the info object
    // the value contains a button to flip through the modes
    JsonObject user = root["u"];
    if (user.isNull())
      user = root.createNestedObject("u");

    JsonArray infoArr = user.createNestedArray("LEDtower sync"); //name
    String uiDomString = "<button class=\"btn infobtn\" onclick=\"requestJson({TowerMode:";
    String towerModeInfo;

    // wrapper to flip through all modes
    switch(towerMode) {
      case 0: uiDomString += "1"; towerModeInfo = "Disabled"; break; // uiDomString += "x" holds next towerMode (added to onclick listener)
      case 1: uiDomString += "2"; towerModeInfo = "Horizontal"; break;
      default: uiDomString += "0"; towerModeInfo = "Vertical";
    }

    uiDomString += "});return false;\">";
    uiDomString += towerModeInfo;
    uiDomString += "</button>";
    infoArr.add(uiDomString);
  }

    /**
   * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
   * Values in the state object may be modified by connected clients
   * Add "TowerMode" to json state. This can be used to flip through all modes.
   */
  void addToJsonState(JsonObject &root)
  {
    root["TowerMode"] = towerMode;
  }

  /**
   * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
   * Values in the state object may be modified by connected clients
   * Add "TowerMode" to json state. This can be used to flip through all modes.
   */
  void readFromJsonState(JsonObject &root)
  {
    if (root["TowerMode"] != nullptr)
    {
      towerMode = root["TowerMode"];
      updateConfig = true;
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
     * addToConfig() will also not yet add your setting to one of the settings pages automatically.
     * To make that work you still have to add the setting to the HTML, xml.cpp and set.cpp manually.
     * 
     * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
     */
  void addToConfig(JsonObject &root)
  {
    JsonObject top = root.createNestedObject("LEDtower");
    top["TowerMode"] = towerMode;
  }

  /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens once immediately after boot)
     * 
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     */
  void readFromConfig(JsonObject &root)
  {
    JsonObject top = root["LEDtower"];
    towerMode = top["TowerMode"] | 0;
    updateConfig = true;
  }
};
