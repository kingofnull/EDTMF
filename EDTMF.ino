#include  "EDTMF.h"

EDTMF decoder;
void setup() {
  Serial.begin(115200);
  Serial.println(F("DTMF Decoder V0_08P")) ;
  //  pinMode(led, OUTPUT) ;

  decoder.init();
}




void loop() {
  String dialStr = "";
  uint32_t lastCatchTs = millis();

  while ((millis() - lastCatchTs) < 250) {
    String lastChar = decoder.getNextChar();
    if (lastChar != "") {

      //Serial.println(srt(millis()-lastCatchTs));
      lastCatchTs = millis() ;
      dialStr += lastChar;
      if (dialStr.length() == 1) {
        Serial.print("Reading=> ") ;
      } else {
        Serial.print(" . ") ;
      }


    }
  }

  if (dialStr != "") {
    Serial.println("");
    Serial.println(dialStr);
  }



}
