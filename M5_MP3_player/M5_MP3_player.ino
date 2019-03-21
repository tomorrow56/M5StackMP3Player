//#pragma mark - Depend ESP8266Audio and ESP8266_Spiram libraries
/* 
cd ~/Arduino/libraries
git clone https://github.com/earlephilhower/ESP8266Audio
git clone https://github.com/Gianbacchio/ESP8266_Spiram
*/

#include "SPIFFS.h"
#include <M5Stack.h>
#include <WiFi.h>
#include "AudioFileSourceSD.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

#include "M5StackUpdater.h"

AudioGeneratorMP3 *mp3;
AudioFileSourceSD *file;
AudioOutputI2S *out;
AudioFileSourceID3 *id3;

String PlayFileName = "";
String DisplayFileName = "";
int selectNum;
String fileName[255];
int fileNum = 0;

void setup()
{
  M5.begin();

  if(digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }

  WiFi.mode(WIFI_OFF); 
  delay(500);
  
  M5.Lcd.setTextFont(2);
  M5.Lcd.println("Sample MP3 playback begins...");
  Serial.println("Sample MP3 playback begins...");

  // scan for SPIFFS files waiting to be moved onto the SD Card
  scanDataFolder();

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);

  DisplayFileName = "/jacket" + PlayFileName.substring(4, PlayFileName.length()-3) + "jpg";
  
  M5.Lcd.drawJpgFile(SD, DisplayFileName.c_str(), 0, 0, 320, 240);

  M5.Lcd.println(PlayFileName);
//  M5.Lcd.println(DisplayFileName);

 // pno_cs from https://ccrma.stanford.edu/~jos/pasp/Sound_Examples.html

  file = new AudioFileSourceSD(PlayFileName.c_str());
  id3 = new AudioFileSourceID3(file);
  out = new AudioOutputI2S(0, 1); // Output to builtInDAC
  out->SetOutputModeMono(false);
  mp3 = new AudioGeneratorMP3();
  mp3->begin(id3, out);
}

void loop()
{
  if (mp3->isRunning()) {
    if (!mp3->loop()){
      mp3->stop();
      M5.Lcd.println("MP3 done\n");
      file->close();
      M5.Lcd.println("Please reset player.\n");
    }
  } else {
    delay(1000);
  }
}

/* 
 *  Scan SPIFFS for binaries and move them onto the SD Card
 *  TODO: create an app manager for the SD Card
 */
void scanDataFolder() {
   File file;
   int i = 0;
   boolean exitMenu = false;

  Serial.println("scanDataFolder");

  if(!SPIFFS.begin(true)){
    M5.Lcd.println("SPIFFS Mount Failed");
    Serial.println("SPIFFS Mount Failed");
  } else {
    M5.Lcd.println("Scan data folder");
    Serial.println("Scan data folder");
    File root = SD.open("/mp3");
    if(!root){
      M5.Lcd.println("Failed to open directory");
    } else {
      M5.Lcd.println("Scan: " + (String)root.name());
      if(!root.isDirectory()){
        M5.Lcd.println("Not a directory");
      } else {
        while(i < 256) {
          file = root.openNextFile();
          if(!file){
//            M5.Lcd.println("Total " + (String)i + " files");
            fileNum = i;
            i = 256;
          } else {
            fileName[i] = file.name();
//            M5.Lcd.println(fileName[i]);
          }
          i++;
        }
      }
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.println("*** File List ***");

      selectNum = 0;
      listFolder();

      while (!exitMenu){
        if(digitalRead(BUTTON_A_PIN) == 0) {
          if( selectNum > 0){
            selectNum--;
            listFolder();
          }
        }
        if(digitalRead(BUTTON_C_PIN) == 0) {
          if( selectNum < fileNum -1){
            selectNum++; 
            listFolder();
          }
        }
        if(digitalRead(BUTTON_B_PIN) == 0) {
          exitMenu = true;
        }
        delay(100);
      }

      PlayFileName = fileName[selectNum];
    }
  }
}

void listFolder() {
  M5.Lcd.setCursor(0, 30);
  for(int i = 0; i < fileNum;  i++){
    if(i == selectNum){
      M5.Lcd.println("> " + fileName[i]);
    } else {
      M5.Lcd.println("  " + fileName[i]);
    }
  }
}


