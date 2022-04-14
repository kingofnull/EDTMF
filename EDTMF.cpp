#include "Arduino.h"
#include "EDTMF.h"
#include "Goertzel.h"


/*
  DTMF Decoding

  11.09.2019 V0_08P  Author 6V6GT https://forum.arduino.cc/index.php?topic=121540.msg4305445#msg4305445

   
  This code is a basic implementation of a DTMF decoder for detecting the 16 character
  DTMF code from the analog pin A0 and gives the decoded output by checking for all the
  Upper and lower tones in the DTMF matrix and gives us the corresponding number by turning
  on the corresponding digital bit for the numbers 0-9 and by Serially printing the rest of
  the characters.

  Wire as follows:
  vcc -->10k-->100nf(lead1)--> 10k --> gnd ;  100nf(lead2) --> A0 ;

  Good results  8bit /  9600 / 64 samples / ADC prescaler  8
                8bit / 19200 / 128 samples  / ADC prescaler  8 (but needs validationRepeats = 0 for 50mS burst length)

  ToDo
    1. better stop and start of data collection suspending adc trigger
    2. replace floats in calculations.

*/

// bits and ideas from and many thanks to:
// https://create.arduino.cc/projecthub/MM_Shoaib/dtmf-decoder-using-only-arduino-872502 (Mian Mohammad Shoaib)
// https://github.com/jacobrosenthal/Goertzel (Jacob Rosenthal)
// http://www.embedded.com/design/embedded/4024443/The-Goertzel-Algorithm  (Kevin Banks)
// https://forum.arduino.cc/index.php?topic=121540.0 (el_supremo and others)


// uint8_t led = 13 ;






void EDTMF::init() {
  // initialise class static storage
  Goertzel::init( sampleStore, N , sampling_freq ) ;

  // calculate coefficients for Goertzel objects
  //   must be done after init()
  for ( int i = 0 ; i < 4 ; i++ ) {
    x_goertzel[ i ].getCoefficient(  ) ;
    y_goertzel[ i ].getCoefficient(  ) ;
  }

  Serial.println( "EDTMF initialised!" );
}

String EDTMF::getNextChar() {
  String result = "";



  /*   if ( ledOn ) {
      if ( millis() - ledOnAtMs > 250 ) {
        digitalWrite( led, LOW ) ;  //expire led
        ledOn = false ;
      }
    }
  */

  if ( Goertzel::testDataReady ) {
    // if sample buffer full

    float x_magnitude_best = 0 ;
    int8_t x = -1 ;  // x is row index , -1 marker indicates not set

    float y_magnitude_best = 0 ;
    int8_t y = -1 ;  // y is column index , -1 marker indicates not set


    for ( int i = 0 ; i < 4 ; i++ ) {

      // pick the best match (x : rows)
      float x_magnitude = x_goertzel[ i ].detect();
      if ( x_magnitude > x_magnitude_best ) {
        x_magnitude_best = x_magnitude ;
        x = i ;
      }

      // pick the best match (y : columns )
      float y_magnitude = y_goertzel[ i ].detect();
      if ( y_magnitude > y_magnitude_best ) {
        y_magnitude_best = y_magnitude ;
        y = i ;
      }

      /*
            // uncomment for detailed debug
            Serial.print( "i= " ) ;
            Serial.print( i ) ;
            Serial.print( "; Magnitude x= " ) ;
            Serial.print( x_magnitude ) ;
            Serial.print( "; Magnitude y= " ) ;
            Serial.print( y_magnitude ) ;

            Serial.print( "; x= " ) ;
            Serial.print( x ) ;
            Serial.print( "; y= " ) ;
            Serial.print( y ) ;

            Serial.print( "; Repeats= " ) ;
            Serial.println( repeats ) ;
            delay(100) ;
      */

    }  // for i

    // test if the sample was identified
    if ( x_magnitude_best > threshold && y_magnitude_best > threshold ) {

      // test if we have i consecutive matches to filter spurious results
      if ( x_last == x && y_last == y ) {
        if ( repeats < 255 ) repeats ++ ;
      }
      else {
        repeats = 0 ;
      }

      x_last = x ;
      y_last = y ;

      if ( repeats == validationRepeats  ) {
        // print out once if we have exactly the required number of consecutive matches
        // irrespecive of the length of the tone burst, we want only one.
        // ledOnAtMs = millis() ;
        // digitalWrite( led, HIGH ) ;
        // ledOn = true ;
        // Serial.print( decoder[x][y] ) ;
        // Serial.print( "   (" ) ;
        // Serial.print( x ) ;
        // Serial.print( "," ) ;
        // Serial.print( y ) ;
        // Serial.print( ")   failed/rejected matches=" ) ;
        // Serial.println( failedMatches ) ;
        result = decoder[x][y];
        failedMatches = 0 ;
      }
      else if ( repeats < validationRepeats ) {
        if ( failedMatches < 255 ) failedMatches ++ ;
      }

    } // if ( x_magnitude_best . . .
    else {
      x_last = -1 ;  //clean
      y_last = -1 ;
      repeats = 0 ;
    }

    // allow ISR to refill the sample buffer.
    Goertzel::testDataReady = false ;

    return result;
  }

}
