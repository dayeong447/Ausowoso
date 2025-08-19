#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD Configuration
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2

// Disaster types
enum DisasterType {
  NONE,
  EARTHQUAKE,
  FIRE,
  FLOOD,
  GAS_LEAK
};

class LCDDisplay {
  private:
    unsigned long lastUpdateTime = 0;
    const unsigned long scrollDelay = 500;
    
  public:
    LiquidCrystal_I2C lcd; // Made public for direct access optimization
  
    LCDDisplay() : lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS) {}
    
    void initialize() {
      lcd.init();
      lcd.backlight();
      clear();
    }
    
    void clear() {
      lcd.clear();
    }
    
    // BACKWARD COMPATIBLE: Keep original String version
    void displayStatic(String line1, String line2 = "") {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(line1);
      if (line2 != "") {
        lcd.setCursor(0, 1);
        lcd.print(line2);
      }
    }
    
    // OPTIMIZED: Add const char* version for F() macro support
    void displayStatic(const char* line1, const char* line2 = nullptr) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(line1);
      if (line2 != nullptr) {
        lcd.setCursor(0, 1);
        lcd.print(line2);
      }
    }
    
    // OPTIMIZED: Support F() macro directly
    void displayStatic(const __FlashStringHelper* line1, const __FlashStringHelper* line2 = nullptr) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(line1);
      if (line2 != nullptr) {
        lcd.setCursor(0, 1);
        lcd.print(line2);
      }
    }
    
    void displayEmergency(uint8_t region, DisasterType type, float severity, String action, bool flash = false) {
        static bool flashState = false;
        displayDisasterWarning(region, type, severity, action.c_str());
      }
      
    void displayDisasterWarning(uint8_t region, DisasterType type, float severity, String action) {
      String typeStr;
      String unit;
      
      switch(type) {
        case EARTHQUAKE:
          typeStr = "Quake";
          unit = "M";
          break;
        case FIRE:
          typeStr = "Fire";
          unit = "°C";
          break;
        case FLOOD:
          typeStr = "Flood";
          unit = "cm";
          break;
        case GAS_LEAK:
          typeStr = "Gas Leak";
          unit = "ppm";
          break;
        default:
          typeStr = "Danger";
          unit = "";
      }
      
      String line1 = "Region " + String(region < 10 ? "0" + String(region) : String(region));
      String line2 = typeStr + " " + String(severity) + unit + " - " + action;
      
      // Handle scrolling for long messages
      if (line2.length() > LCD_COLUMNS) {
        scrollText(line1, line2);
      } else {
        displayStatic(line1, line2);
      }
    }
    
    void displayNormal(String message) {
      displayStatic("System Normal", message);
    }
    
    void displaySensorData(String sensorName, float value, String unit) {
      String line1 = sensorName;
      String line2 = String(value) + " " + unit;
      displayStatic(line1, line2);
    }
    
  private:
    void scrollText(String line1, String line2) {
      static unsigned int pos = 0;
      
      if (millis() - lastUpdateTime > scrollDelay) {
        lastUpdateTime = millis();
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(line1);
        
        lcd.setCursor(0, 1);
        if (pos >= line2.length()) {
          pos = 0;
        }
        
        String displayText = line2.substring(pos);
        if (displayText.length() > LCD_COLUMNS) {
          displayText = displayText.substring(0, LCD_COLUMNS);
        } else {
          while (displayText.length() < LCD_COLUMNS) {
            displayText += " ";
          }
        }
        
        lcd.print(displayText);
        pos++;
      }
    }
};

#endif
