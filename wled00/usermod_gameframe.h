#pragma once

#include "wled.h"

#include "IniFileLite.h"

//TODO add description
//TODO improve commenring

#define GF_DEBUG_FILES 0

#define BUFFPIXEL 16 // number of pixels to buffer when reading BMP files

class GameFrame : public Usermod {
  public:

  private:
    bool
      usermodActive = false,
      prevState = false,
      updateConfig = false,
      showOnce = false,
      logoPlayed = false, // plays logo animation correctly reardless of playMode
      folderLoop = true, // animation looping
      moveLoop = false, // translation/pan looping
      menuActive = false, // showing menus (set brightness, playback mode, etc.)
      panoff = true, // movement scrolls off screen
      singleGraphic = false, // single BMP file or multiple BMP files
      abortImage = false, // image is corrupt; abort, retry, fail?
      clockShown = false, // clock mode?
      finishBeforeProgressing = false, // finish the animation before progressing?
      breakout = false, // breakout playing?
      clockAnimationActive = false, // currently showing clock anim
      timerLapsed = false; // timer lapsed, queue next animation when current one finishes

    byte
      displayMode = 0, // 0 = slideshow, 1 = clock
      playMode = 0, // 0 = sequential, 1 = random, 2 = pause animations
      cycleTimeSetting = 2, // time before next animation: 1=10 secs, 2=30 secs, 3=1 min... 8=infinity
      clockAnimationLength = 5, // seconds to play clock animations
      lastSecond = 255, // a moment ago
      currentSecond = 255; // current second

    int
      secondCounter = 0, // counts up every second
      fileIndex = 0, // current frame
      chainIndex = -1, // for chaining multiple folders
      numFolders = 0, // number of folders on sd in /animations
      cycleTime = 30, // seconds to wait before progressing to next folder
      offsetSpeedX = 0, // number of pixels to translate each frame
      offsetSpeedY = 0, // number of pixels to translate each frame
      offsetX = 0, // for translating images x pixels
      offsetY = 0, // for translating images y pixels
      imageWidth = 0,
      imageHeight = 0;

    int16_t 
      folderIndex = 0; // current folder

    unsigned long
      lastTime = 0, // used to calculate draw time
      drawTime = 0, // time to read from sd
      holdTime = 200, // millisecods to hold each .bmp frame
      swapTime = 0, // system time to advance to next frame
      baseTime = 0; // time we enter menu

    char
      chainRootFolder[9], // chain game
      nextFolder[21], // dictated next animation
      curFolder[21]; // current animation

      CRGB
        matrix[256] = {0};

    #ifdef SD_ADAPTER
      UsermodSdCard *sdCard;
    #else
      void* sdCard = nullptr;
    #endif
  
  uint8_t dim8_jer( uint8_t x )
  {
    return ((uint16_t)x * (uint16_t)(x) ) >> 8;
  }

  byte getIndex(byte x, byte y)
  {
    byte index;
    if(strip.isMatrix){
      if (y == 0)
      {
        index = x;
      }
      else if (y % 2 == 0)
      {
        index = y * 16 + x;
      }
      else
      {
        index = y * 16 + x;
      }
    } 
    else {
      if (y == 0)
      {
        index = x;
      }
      else if (y % 2 == 0)
      {
        index = y * 16 + x;
      }
      else
      {
        index = (y * 16 + 15) - x;
      }
    }
    return index;
  }

  void clearStripBuffer()
  {
    strip.fill(0);
  }

  void closeMyFile()
  {
    if (myFile.isOpen()) 
    {
      if (GF_DEBUG_FILES) DEBUG_PRINTLN(F("Closing Image..."));
      myFile.close();
    }
  }

  void updateState() {
    if(usermodActive){
      if(usermodActive != prevState) savePreset(42); // save current settings in a temporary preset

      strip.setSegment(0, 0, 16, 1, 0, 0, 0, 16);
      strip.setPixelSegment(0);
      strip.setMode(0, FX_MODE_BPM);
      Segment &seg = strip.getSegment(0);
      setValuesFromSegment(0);
      seg.setColor(0, 0);
      seg.setColor(1, 0);
      seg.setColor(2, 0);
      stateChanged = true;  // inform external devices/UI of change
      colorUpdated(CALL_MODE_DIRECT_CHANGE);
    }
    else {
      if(usermodActive != prevState) {
        // strip.setMode(0, FX_MODE_STATIC);
        applyPreset(42); // apply previous settings
        
        strip.trigger();  // force strip refresh
        stateChanged = true;  // inform external devices/UI of change
        colorUpdated(CALL_MODE_DIRECT_CHANGE);
        // deletePreset(42); // delete temporary preset
      }
    }
  }

  void initGameFrame() {
    updateState();

    closeMyFile();
    SD_ADAPTER.chdir("/");

    char folder[13];

    if (numFolders == 0)
    {
      numFolders = 0;
      // file indexes appear to loop after 2048
      for (int fileIndex = 0; fileIndex < 2048; fileIndex++)
      {
        myFile.open(SD_ADAPTER.vwd(), fileIndex, O_READ);
        if (myFile.isDir()) {
          DEBUG_PRINTLN("---");
          numFolders++;
          DEBUG_PRINT("File Index: ");
          DEBUG_PRINTLN(fileIndex);
          myFile.getName(folder, 13);
          DEBUG_PRINT("Folder: ");
          DEBUG_PRINTLN(folder);
          closeMyFile();
        }
        else closeMyFile();
      }
      DEBUG_PRINT(numFolders);
      DEBUG_PRINTLN(" folders found.");
    }
  }

  void setCycleTime()
  {
    if (cycleTimeSetting == 2)
    {
      cycleTime = 30;
    }
    else if (cycleTimeSetting == 3)
    {
      cycleTime = 60;
    }
    else if (cycleTimeSetting == 4)
    {
      cycleTime = 300;
    }
    else if (cycleTimeSetting == 5)
    {
      cycleTime = 900;
    }
    else if (cycleTimeSetting == 6)
    {
      cycleTime = 1800;
    }
    else if (cycleTimeSetting == 7)
    {
      cycleTime = 3600;
    }
    else if (cycleTimeSetting == 8)
    {
      cycleTime = -1;
    }
    else
    {
      cycleTime = 10;
    }
  }

  void nextImage()
  {
    Serial.println(F("---"));
    Serial.println(F("Next Folder..."));
    closeMyFile();
    boolean foundNewFolder = false;
    // reset secondCounter if not playing clock animations
    if (!clockAnimationActive) secondCounter = 0;
    baseTime = millis();
    holdTime = 0;
    char folder[9];
    SD_ADAPTER.chdir("/");
    fileIndex = 0;
    offsetX = 0;
    offsetY = 0;
    singleGraphic = false;
    finishBeforeProgressing = false;
    timerLapsed = false;
    // if (!logoPlayed) logoPlayed = true;

    // are we chaining folders?
    if (chainIndex > -1)
    {
      char chainChar[6];
      char chainDir[23];
      strcpy_P(chainDir, PSTR("/"));
      strcat(chainDir, chainRootFolder);
      strcat(chainDir, "/");
      itoa(chainIndex, chainChar, 10);
      strcat(chainDir, chainChar);
      if (file_onSD(chainDir))
      {
        Serial.print(F("Chaining: "));
        Serial.println(chainDir);
        SD_ADAPTER.chdir(chainDir);
        chainIndex++;
      }
      else
      {
        // chaining concluded
        chainIndex = -1;
        chainRootFolder[0] = '\0';
        SD_ADAPTER.chdir("/");
      }
    }

    // has the next animation has been dictated by the previous .INI file?
    if (nextFolder[0] != '\0' && chainIndex == -1)
    {
      Serial.print(F("Forcing next: "));
      Serial.println(nextFolder);
      if (file_onSD(nextFolder))
      {
        SD_ADAPTER.chdir(nextFolder);
        strcpy_P(curFolder, nextFolder);
      }
      else
      {
        nextFolder[0] = '\0';
        Serial.println(F("Not exists!"));
      }
    }

    // next folder not assigned by .INI
    if (nextFolder[0] == '\0' && chainIndex == -1)
    {
      // Getting next folder
      // shuffle playback using random number
      if (playMode != 0) // check we're not in a sequential play mode
      {
        int targetFolder = random(0, numFolders);

        // don't repeat the same image, please.
        if (targetFolder <= 0 or targetFolder == numFolders or targetFolder == numFolders - 1)
        {
          // Repeat image detected! Incrementing targetFolder.
          targetFolder = targetFolder + 2;
        }

        Serial.print(F("Randomly advancing "));
        Serial.print(targetFolder);
        Serial.println(F(" folder(s)."));
        int i = 1;
        while (i < targetFolder)
        {
          foundNewFolder = false;
          while (foundNewFolder == false)
          {
            myFile.open(SD_ADAPTER.vwd(), folderIndex, O_READ);
            if (myFile.isDir()) {
              foundNewFolder = true;
              i++;
            }
            closeMyFile();
            folderIndex++;
          }
        }
      }

      foundNewFolder = false;

      while (foundNewFolder == false)
      {
        myFile.open(SD_ADAPTER.vwd(), folderIndex, O_READ);
        myFile.getName(folder, 13);

        // ignore system folders that start with "00"
        if (myFile.isDir() && folder[0] != 48 && folder[1] != 48) {
          foundNewFolder = true;
          Serial.print(F("Folder Index: "));
          Serial.println(folderIndex);
          Serial.print(F("Opening Folder: "));
          Serial.println(folder);

          SD_ADAPTER.chdir(folder);
          closeMyFile();
          strcpy_P(curFolder, folder);
        }
        else closeMyFile();
        folderIndex++;
      }
    }

    // is this the start of a folder chain?
    char chainDir[2];
    strcpy_P(chainDir, PSTR("0"));
    if (file_onSD(chainDir))
    {
      Serial.print(F("Chaining detected: "));
      Serial.println(folder);
      memcpy(chainRootFolder, folder, 8);
      SD_ADAPTER.chdir(chainDir);
      chainIndex = 1;
    }

    char firstImage[6];
    strcpy_P(firstImage, PSTR("0.bmp"));
    if (file_onSD(firstImage))
    {
      Serial.print(F("Opening File: "));
      Serial.print(folder);
      Serial.println(F("/config.ini"));
      readIniFile();

      char tmp[6];
      strcpy_P(tmp, PSTR("0.bmp"));
      refreshImageDimensions(tmp);

      Serial.print(F("Hold (in ms): "));
      Serial.println(holdTime);
      swapTime = millis() + holdTime;

      // setup image for x/y translation as needed if animations aren't paused
      if (playMode != 2)
      {
        if (offsetSpeedX > 0)
        {
          if (panoff == true) offsetX = (imageWidth * -1);
          else offsetX = (imageWidth * -1 + 16);
        }
        else if (offsetSpeedX < 0)
        {
          if (panoff == true) offsetX = 16;
          else offsetX = 0;
        }
        if (offsetSpeedY > 0)
        {
          if (panoff == true) offsetY = -16;
          else offsetY = 0;
        }
        else if (offsetSpeedY < 0)
        {
          if (panoff == true) offsetY = imageHeight;
          else offsetY = imageHeight - 16;
        }
      }
      // center image if animations are paused
      else
      {
        offsetX = imageWidth / -2 + 8;
        offsetY = imageHeight / 2 - 8;
      }

      // test for single frame

      char tmp_0[6];
      char tmp_1[6];
      strcpy_P(tmp_0, PSTR("0.bmp"));
      strcpy_P(tmp_1, PSTR("1.bmp"));
      if (file_onSD(tmp_0) && (!file_onSD(tmp_1)))
      {
        singleGraphic = true;
        // check for pan settings
        if (offsetSpeedX == 0 && offsetSpeedY == 0)
        {
          // single frame still
          holdTime = -1;
        }
      }
    }

    // empty folder
    else
    {
      Serial.println(F("Empty folder!"));
      nextImage();
    }
  }

  void drawFrame()
  {
    if (panoff == true)
    {
      if (offsetX > 16 || offsetX < (imageWidth * -1) || offsetY > imageHeight || offsetY < -16)
      {
        if (moveLoop == false || finishBeforeProgressing == true)
        {
          fileIndex = 0;
          nextImage();
        }
        else
        {
          if (offsetSpeedX > 0 && offsetX >= 16)
          {
            offsetX = (imageWidth * -1);
          }
          else if (offsetSpeedX < 0 && offsetX <= imageWidth * -1)
          {
            offsetX = 16;
          }
          if (offsetSpeedY > 0 && offsetY >= imageHeight)
          {
            offsetY = -16;
          }
          else if (offsetSpeedY < 0 && offsetY <= -16)
          {
            offsetY = imageHeight;
          }
        }
      }
    }
    else
    {
      if (offsetX > 0 || offsetX < (imageWidth * -1 + 16) || offsetY > imageHeight - 16 || offsetY < 0)
      {
        if (moveLoop == false || finishBeforeProgressing == true)
        {
          fileIndex = 0;
          nextImage();
        }
        else
        {
          if (offsetSpeedX > 0 && offsetX >= 0)
          {
            offsetX = (imageWidth * -1 + 16);
          }
          else if (offsetSpeedX < 0 && offsetX <= imageWidth - 16)
          {
            offsetX = 0;
          }
          if (offsetSpeedY > 0 && offsetY >= imageHeight - 16)
          {
            offsetY = 0;
          }
          else if (offsetSpeedY < 0 && offsetY <= 0)
          {
            offsetY = imageHeight - 16;
          }
        }
      }
    }

    if (singleGraphic == false)
    {
      char bmpFile[13]; // 8-digit number + .bmp + null byte
      itoa(fileIndex, bmpFile, 10);
      strcat(bmpFile, ".bmp");
      if (!file_onSD(bmpFile))
      {
        fileIndex = 0;
        itoa(fileIndex, bmpFile, 10);
        strcat(bmpFile, ".bmp");
        if (finishBeforeProgressing && (offsetSpeedX != 0 || offsetSpeedY != 0)); // translating image - continue animating until moved off screen
        else if (folderLoop == false || timerLapsed == true)
        {
          if (displayMode == 0) nextImage();
          else if (displayMode == 1)
          {
            // just displayed logo, enter clock mode
            // initClock();
            return;
          }
        }
      }
      bmpDraw(bmpFile, 0, 0);
    }
    else bmpDraw("0.bmp", 0, 0);

    // print draw time in milliseconds
    drawTime = millis() - lastTime;
    lastTime = millis();
    if(GF_DEBUG_FILES) {
      Serial.print(F("ttd: "));
      Serial.print(drawTime);
      Serial.print(F("  "));
      Serial.println(holdTime);
    }

    if (offsetSpeedX != 0) offsetX += offsetSpeedX;
    if (offsetSpeedY != 0) offsetY += offsetSpeedY;
  }

  void refreshImageDimensions(char *filename) {

    const uint8_t  gridWidth = 16;
    const uint8_t  gridHeight = 16;

    if ((0 >= gridWidth) || (0 >= gridHeight)) {
      Serial.print(F("Abort."));
      return;
    }

    // storing dimentions for image
    // Open requested file on SD card
    if (!myFile.open(filename, O_READ)) {
      Serial.println(F("File open failed"));
      // // sdErrorMessage();
      return;
    }

    // Parse BMP header
    if (read16(myFile) == 0x4D42) { // BMP signature
      (void)read32(myFile); // Read & ignore file size
      (void)read32(myFile); // Read & ignore creator bytes
      (void)read32(myFile); // skip data
      // Read DIB header
      (void)read32(myFile); // Read & ignore Header size
      imageWidth  = read32(myFile);
      imageHeight = read32(myFile);
      if(GF_DEBUG_FILES){
        Serial.print(F("Image resolution: "));
        Serial.print(imageWidth);
        Serial.print(F("x"));
        Serial.println(imageHeight);
      }
    }
    closeMyFile();
  }

  // This function opens a Windows Bitmap (BMP) file and
  // displays it at the given coordinates.  It's sped up
  // by reading many pixels worth of data at a time
  // (rather than pixel by pixel).  Increasing the buffer
  // size takes more of the Arduino's precious RAM but
  // makes loading a little faster.  20 pixels seems a
  // good balance.

  void bmpDraw(char *filename, uint8_t x, uint8_t y) {

    int  bmpWidth, bmpHeight;   // W+H in pixels
    uint8_t  bmpDepth;              // Bit depth (currently must be 24 or 32)
    uint8_t  sdbuffer[3 * BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
    uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
    uint32_t bmpImageoffset;        // Start of image data in file
    uint32_t  rowSize;               // Not always = bmpWidth; may have padding
    boolean  goodBmp = false;       // Set to true on valid header parse
    boolean  flip    = true;        // BMP is stored bottom-to-top
    int  w, h, row, col;
    uint8_t  r, g, b;
    uint32_t pos = 0;
    const uint8_t  gridWidth = 16;
    const uint8_t  gridHeight = 16;

    if ((x >= gridWidth) || (y >= gridHeight)) {
      Serial.print(F("Abort."));
      return;
    }

    if (!myFile.isOpen())
    {
      if(GF_DEBUG_FILES) {
        Serial.println();
        Serial.print(F("Loading image '"));
        Serial.print(filename);
        Serial.println('\'');

        // Serial.println("Directory:");
        // SD_ADAPTER.ls(LS_R | LS_DATE | LS_SIZE);
        // Serial.println();
      }

      // Open requested file on SD card
      if (!myFile.open(filename, O_READ)) {
        Serial.println(F("File open failed"));
        // sdErrorMessage();
        return;
      }
    }
    else myFile.rewind();

    // Parse BMP header
    if (read16(myFile) == 0x4D42) { // BMP signature
      if (GF_DEBUG_FILES)
      {
        Serial.print(F("File size: ")); Serial.println(read32(myFile));
      }
      else (void)read32(myFile);
      (void)read32(myFile); // Read & ignore creator bytes
      bmpImageoffset = read32(myFile); // Start of image data
      //    Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
      // Read DIB header
      (void)read32(myFile); // Read & ignore Header size
      bmpWidth  = read32(myFile);
      bmpHeight = read32(myFile);
      if (read16(myFile) == 1) { // # planes -- must be '1'
        bmpDepth = read16(myFile); // bits per pixel
        //      Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
        if ((bmpDepth == 24 || bmpDepth == 32) && (read32(myFile) == 0)) { // 0 = uncompressed
          goodBmp = true; // Supported BMP format -- proceed!
          if (GF_DEBUG_FILES)
          {
            Serial.print(F("Image size: "));
            Serial.print(bmpWidth);
            Serial.print('x');
            Serial.println(bmpHeight);
          }

          // image smaller than 16x16?
          if ((bmpWidth < 16 && bmpWidth > -16) || (bmpHeight < 16 && bmpHeight > -16))
          {
            clearStripBuffer();
          }

          // BMP rows are padded (if needed) to 4-byte boundary
          rowSize = (bmpWidth * bmpDepth/8 + bmpDepth/8) & ~(bmpDepth/8); // 32-bit BMP support
          //        Serial.print(F("Row size: "));
          //        Serial.println(rowSize);

          // If bmpHeight is negative, image is in top-down order.
          // This is not canon but has been observed in the wild.
          if (bmpHeight < 0) {
            bmpHeight = -bmpHeight;
            flip      = false;
          }

          // initialize our pixel index
          byte index = 0; // a byte is perfect for a 16x16 grid

          // Crop area to be loaded
          w = bmpWidth;
          h = bmpHeight;
          if ((x + w - 1) >= gridWidth)  w = gridWidth - x;
          if ((y + h - 1) >= gridHeight) h = gridHeight - y;

          for (row = 0; row < h; row++) { // For each scanline...

            // Seek to start of scan line.  It might seem labor-
            // intensive to be doing this on every line, but this
            // method covers a lot of gritty details like cropping
            // and scanline padding.  Also, the seek only takes
            // place if the file position actually needs to change
            // (avoids a lot of cluster math in SD library).

            if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
              pos = (bmpImageoffset + (offsetX * -3) + (bmpHeight - 1 - (row + offsetY)) * rowSize);
            else     // Bitmap is stored top-to-bottom
              pos = bmpImageoffset + row * rowSize;
            if (myFile.curPosition() != pos) { // Need seek?
              myFile.seekSet(pos);
              buffidx = sizeof(sdbuffer); // Force buffer reload
            }

            for (col = 0; col < w; col++) { // For each pixel...
              // Time to read more pixel data?
              
              if (bmpDepth == 24)
              {
                if (buffidx >= sizeof(sdbuffer)) { // Indeed
                  myFile.read(sdbuffer, sizeof(sdbuffer));
                  buffidx = 0; // Set index to beginning
                }
              }
              else if (bmpDepth == 32)
              {
                if (buffidx >= 3) { // 32-bit file compatibility forces buffer to 1 pixel at a time
                  myFile.read(sdbuffer, 3); 
                  myFile.read(); // eat the alpha channel
                  buffidx = 0; // Set index to beginning
                }
              }

              // push to LED buffer
              b = sdbuffer[buffidx++];
              g = sdbuffer[buffidx++];
              r = sdbuffer[buffidx++];

              // apply contrast
              r = dim8_jer(r);
              g = dim8_jer(g);
              b = dim8_jer(b);

              // offsetY is beyond bmpHeight
              if (row >= bmpHeight - offsetY)
              {
                // black pixel
                matrix[getIndex(col, row)] = 0;
              }
              // offsetY is negative
              else if (row < offsetY * -1)
              {
                // black pixel
                matrix[getIndex(col, row)] = 0;
              }
              // offserX is beyond bmpWidth
              else if (col >= bmpWidth + offsetX)
              {
                // black pixel
                matrix[getIndex(col, row)] = 0;
              }
              // offsetX is positive
              else if (col < offsetX)
              {
                // black pixel
                matrix[getIndex(col, row)] = 0;
              }
              // all good
              else matrix[getIndex(col + x, row)] = CRGB(r, g, b);
              // paint pixel color
            } // end pixel
          } // end scanline
        } // end goodBmp
      }
    }
    if (!clockShown || breakout == true)
    {
      showOnce = true;
      // FastLED.show();
    }
    if (singleGraphic == false || breakout == true)
    {
      closeMyFile();
    }
    if (!goodBmp) Serial.println(F("Format unrecognized"));
  }

  void readIniFile()
  {
    const size_t bufferLen = 50;
    char buffer[bufferLen];
    char configFile[11];
    strcpy_P(configFile, PSTR("config.ini"));
    const char *filename = configFile;
    IniFile ini(filename);
    if (!ini.open()) {
      DEBUG_PRINT(filename);
      DEBUG_PRINTLN(F(" does not exist"));
      // Cannot do anything else
    }
    else
    {
      DEBUG_PRINTLN(F("Ini file exists"));
    }

    // Check the file is valid. This can be used to warn if any lines
    // are longer than the buffer.
    if (!ini.validate(buffer, bufferLen)) {
      DEBUG_PRINT(F("ini file "));
      DEBUG_PRINT(ini.getFilename());
      DEBUG_PRINT(F(" not valid: "));
      // printErrorMessage(ini.getError());
      // Cannot do anything else
    }
    char section[10];
    strcpy_P(section, PSTR("animation"));
    char entry[11];
    strcpy_P(entry, PSTR("hold"));

    // Fetch a value from a key which is present
    if (ini.getValue(section, entry, buffer, bufferLen)) {
      DEBUG_PRINT(F("hold value: "));
      DEBUG_PRINTLN(buffer);
      holdTime = atol(buffer);
    }
    else {
      // printErrorMessage(ini.getError());
      holdTime = 200;
    }

    strcpy_P(entry, PSTR("loop"));

    // Fetch a boolean value
    bool loopCheck;
    bool found = ini.getValue(section, entry, buffer, bufferLen, loopCheck);
    if (found) {
      DEBUG_PRINT(F("animation loop value: "));
      // Print value, converting boolean to a string
      DEBUG_PRINTLN(loopCheck ? F("TRUE") : F("FALSE"));
      folderLoop = loopCheck;
    }
    else {
      // printErrorMessage(ini.getError());
      folderLoop = true;
    }

    strcpy_P(entry, PSTR("finish"));

    // Fetch a boolean value
    bool finishCheck;
    bool found4 = ini.getValue(section, entry, buffer, bufferLen, finishCheck);
    if (found4) {
      DEBUG_PRINT(F("finish value: "));
      // Print value, converting boolean to a string
      DEBUG_PRINTLN(finishCheck ? F("TRUE") : F("FALSE"));
      finishBeforeProgressing = finishCheck;
    }
    else {
      // printErrorMessage(ini.getError());
      finishBeforeProgressing = false;
    }

    strcpy_P(section, PSTR("translate"));
    strcpy_P(entry, PSTR("moveX"));

    // Fetch a value from a key which is present
    if (ini.getValue(section, entry, buffer, bufferLen)) {
      DEBUG_PRINT(F("moveX value: "));
      DEBUG_PRINTLN(buffer);
      offsetSpeedX = atoi(buffer);
    }
    else {
      // printErrorMessage(ini.getError());
      offsetSpeedX = 0;
    }

    strcpy_P(entry, PSTR("moveY"));

    // Fetch a value from a key which is present
    if (ini.getValue(section, entry, buffer, bufferLen)) {
      DEBUG_PRINT(F("moveY value: "));
      DEBUG_PRINTLN(buffer);
      offsetSpeedY = atoi(buffer);
    }
    else {
      // printErrorMessage(ini.getError());
      offsetSpeedY = 0;
    }

    strcpy_P(entry, PSTR("loop"));

    // Fetch a boolean value
    bool loopCheck2;
    bool found2 = ini.getValue(section, entry, buffer, bufferLen, loopCheck2);
    if (found2) {
      DEBUG_PRINT(F("translate loop value: "));
      // Print value, converting boolean to a string
      DEBUG_PRINTLN(loopCheck2 ? F("TRUE") : F("FALSE"));
      moveLoop = loopCheck2;
    }
    else {
      // printErrorMessage(ini.getError());
      moveLoop = false;
    }

    strcpy_P(entry, PSTR("panoff"));

    // Fetch a boolean value
    bool loopCheck3;
    bool found3 = ini.getValue(section, entry, buffer, bufferLen, loopCheck3);
    if (found3) {
      DEBUG_PRINT(F("panoff value: "));
      // Print value, converting boolean to a string
      DEBUG_PRINTLN(loopCheck3 ? F("TRUE") : F("FALSE"));
      panoff = loopCheck3;
    }
    else {
      // printErrorMessage(ini.getError());
      panoff = true;
    }

    strcpy_P(entry, PSTR("nextFolder"));

    // Fetch a value from a key which is present
    if (ini.getValue(section, entry, buffer, bufferLen)) {
      DEBUG_PRINT(F("nextFolder value: "));
      DEBUG_PRINTLN(buffer);
      memcpy(nextFolder, buffer, 8);
    }
    else {
      // printErrorMessage(ini.getError());
      nextFolder[0] = '\0';
    }

    if (ini.isOpen()) ini.close();
  }

  // These read 16- and 32-bit types from the SD card file.
  // BMP data is stored little-endian, Arduino is little-endian too.
  // May need to reverse subscript order if porting elsewhere.

  uint16_t read16(SdFile& f) {
    uint16_t result;
    f.read(&result, 2);
    return result;
  }

  uint32_t read32(SdFile& f) {
    uint32_t result;
    f.read(&result, 4);
    return result;
  }


  public:
    void setup() {
      // initSD();
      #ifdef SD_ADAPTER
        sdCard = (UsermodSdCard*) usermods.lookup(USERMOD_ID_SD_CARD)
      #endif
      prevState = usermodActive;
    }


  /*
    * connected() is called every time the WiFi is (re)connected
    * Use it to initialize network interfaces
    */
  void connected() {
    //Serial.println("Connected to WiFi!");
    // TODO: add wifi symbol animation
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

    // TODO add strip.isMatrix() check including 16x16 dimension check
      if(!usermodActive) return; 
      if(sdCard == nullptr) return;
      if(!sdCard->configSdEnabled) return;

      // periodically check if SD card is inserted
      if(!file_onSD("/") && millis() % 5000 == 0) {
        sdCard.setup();
        if(file_onSD("/")) {
          initGameFrame();
          nextImage();
          drawFrame();
        }
      }

      currentSecond = second(localTime);
      // currently playing images?
      if (menuActive == false && breakout == false)
      {
        if (clockShown == false || clockAnimationActive == true)
        {
          // advance counter
          if (currentSecond != lastSecond)
          {
            lastSecond = currentSecond;
            secondCounter++;
            // revert to clock display if animation played for 5 seconds
            if (clockAnimationActive == true && secondCounter >= clockAnimationLength)
            {
              // initClock();
            }
          }

          // did image load fail?
          if (abortImage == true && clockShown == false && logoPlayed == true)
          {
            abortImage = false;
            nextImage();
            drawFrame();
          }

          // progress to next folder if cycleTime is up
          // check for infinite mode
          else if (cycleTimeSetting != 8  && clockShown == false && clockAnimationActive == false)
          {
            if (secondCounter >= cycleTime)
            {
              if (finishBeforeProgressing == true)
              {
                if (timerLapsed == false) timerLapsed = true;
              }
              else
              {
                nextImage();
                drawFrame();
              }
            }
          }

          // animate if not a single-frame & animations are on
          if (holdTime != -1 && playMode != 2 || logoPlayed == false)
          {
            if (millis() >= swapTime && clockShown == false)
            {
              // statusLedFlicker();
              swapTime = millis() + holdTime;
              fileIndex++;
              drawFrame();
            }
          }
        }

        // show clock
        else if (clockShown == true && clockAnimationActive == false)
        {
          // showClock();
        }
      }

      if(usermodActive != prevState) {
        updateState();
        serializeConfig();
        prevState = usermodActive;
      }
    }

    /**
   * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
   * 
   * Add 
   */
  void addToJsonInfo(JsonObject &root)
  {
    //this code adds "u":{"GameFrame":uiDomString} to the info object
    // the value contains a button to toggle gameframe activation
    JsonObject user = root["u"];
    if (user.isNull())
      user = root.createNestedObject("u");

    JsonArray gameframeArr = user.createNestedArray("GameFrame"); //name
    String uiDomString = "<button class=\"btn infobtn\" onclick=\"requestJson({GameFrameActive:";
    String str_gameframe_state;

    // wrapper to flip through all modes
    if(usermodActive) {
      uiDomString += "0"; str_gameframe_state = "Enabled"; } // uiDomString += "x" holds next mode (added to onclick listener)
    else {
      uiDomString += "1"; str_gameframe_state = "Disabled"; }

    uiDomString += "});return false;\">";
    uiDomString += str_gameframe_state;
    uiDomString += "</button>";
    gameframeArr.add(uiDomString);

    // Animation Control butttons 
    // TODO: make it more appealing
    if(usermodActive) {
      JsonArray animationSettingsArr = user.createNestedArray("Animation: " + String(curFolder)); //name

      // playMode 0 = sequential, 1 = random, 2 = pause animation
      String animationDomString = "<button class=\"btn infobtn\" onclick=\"requestJson({playMode:";
      String playModeFriendlyName;
      switch(playMode) {
        case 0:
          animationDomString += "1";
          playModeFriendlyName = "Sequential";
          break;
        case 1:
          animationDomString += "0";
          playModeFriendlyName = "Random";
          break;
        // case 2:
        //   animationDomString += "0";
        //   playModeFriendlyName = "Pause";
        //   break;
      }
      animationDomString +=  "});return false;\">";
      animationDomString += playModeFriendlyName;
      animationDomString += "</button>";

      // cycleTimeSettings button
      animationDomString += "<button class=\"btn infobtn\" onclick=\"requestJson({cycleTimeSetting:";
      String cycleTimeStr;
      switch(cycleTimeSetting) {
        case 1:
          animationDomString += "2";
          cycleTimeStr = "10 sec";
          break;
        case 2:
          animationDomString += "3";
          cycleTimeStr = "30 sec";
          break;
        case 3:
          animationDomString += "4";
          cycleTimeStr = "1 min";
          break;
        case 4:
          animationDomString += "5";
          cycleTimeStr = "5 min";
          break;
        case 5:
          animationDomString += "6";
          cycleTimeStr = "15 min";
          break;
        case 6:
          animationDomString += "7";
          cycleTimeStr = "30 min";
          break;
        case 7:
          animationDomString += "8";
          cycleTimeStr = "60 min";
          break;
        case 8:
          animationDomString += "1";
          cycleTimeStr = "Infinite";
          break;
        default:
          animationDomString += "1";
          cycleTimeStr = "10 sec";
          break;
      }
      animationDomString +=  "});return false;\">";
      animationDomString += cycleTimeStr;
      animationDomString += "</button>";

      // NextImage trigger button
      animationDomString += "<button class=\"btn infobtn\" onclick=\"requestJson({NextImage:";
      animationDomString += "1});return false;\">";
      animationDomString += "Next Animation";
      animationDomString += "</button>";
      animationSettingsArr.add(animationDomString);
    }

    // --- add SD card infos ---

    JsonArray sd1Arr = user.createNestedArray("SD status"); //name
    // show SD init status
    uiDomString = sdReady ? "SD card present" : "no SD card";

    uiDomString += "</td></tr>";
    sd1Arr.add(uiDomString);
  }

    /**
   * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
   * Values in the state object may be modified by connected clients
   * Add "GameFrameActive" to json state. This can be used to flip through all modes.
   */
  void addToJsonState(JsonObject &root)
  {
    root["GameFrameActive"] = usermodActive;
    root["NextImage"] = 0;
    root["playMode"] = playMode;
    root["cycleTimeSetting"] = cycleTimeSetting;
  }

  /**
   * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
   * Values in the state object may be modified by connected clients
   * Add "GameFrameActive" to json state. This can be used to flip through all modes.
   */
  void readFromJsonState(JsonObject &root)
  {
    if (root["GameFrameActive"] != nullptr)
    {
      usermodActive = root["GameFrameActive"];
      // updateConfig = usermodActive;
    }

    if (root["NextImage"] != nullptr)
    {
      if(root["NextImage"]) {
        nextImage();
        drawFrame();
        root["NextImage"] = 0;
      }
    }

    if (root["playMode"] != nullptr)
    {
      playMode = root["playMode"];
    }

    if (root["cycleTimeSetting"] != nullptr)
    {
      if(cycleTimeSetting != root["cycleTimeSetting"]) {
        cycleTimeSetting = root["cycleTimeSetting"];
        setCycleTime();
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
    JsonObject top = root.createNestedObject("GameFrame");

    top["GameFrameActive"] = usermodActive;
    top["playMode"] = playMode;
    top["cycleTimeSetting"] = cycleTimeSetting;
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

    JsonObject top = root["GameFrame"];

    bool configComplete = !top.isNull();

    configComplete &= getJsonValue(top["GameFrameActive"], usermodActive);
    configComplete &= getJsonValue(top["playMode"], playMode);
    configComplete &= getJsonValue(top["cycleTimeSetting"], cycleTimeSetting);

    // usermodActive = 0;
    // updateConfig = usermodActive;

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
    
    // check if usermod is active
    if (usermodActive)
    {
      // loop over all leds
      for (int x = 0; x < 256; x++)
      {
        strip.setPixelColor(x, matrix[x]);
      }
      showOnce = false;
    }

  }

  
  /*
    * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
    * This could be used in the future for the system to determine whether your usermod is installed.
    */
  uint16_t getId()
  {
    return USERMOD_ID_GAMEFRAME;
  }

  //More methods can be added in the future, this example will then be extended.
  //Your usermod will remain compatible as it does not need to implement all methods from the Usermod base class!
};