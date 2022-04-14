#ifndef EDTMF_h
#define EDTMF_h
#include "Goertzel.h"

class EDTMF{
   public:
      void init();
      String getNextChar();
      
      uint8_t repeats = 0 ;
      int8_t x_last = -1 ;         // -1 marker indicates not set
      int8_t y_last = -1 ;         // -1 marker indicates not set
      uint8_t failedMatches = 0 ;  // Quality statistic. Both x and y are > threshold, but have been rejected.
      // At least validationRepeat rejects will be counted.

      //uint32_t ledOnAtMs = 0 ;
      //bool ledOn = false ;
   private:
   
    static const uint16_t N = 64;                   // sample size. Dependency to sample frequency. Say 64 for 9600 samples per second. 128 for 19200 sps.

    const float threshold = 500;             // minimum tone amplitude. Say 500 for 8bit resolution, 2000 for 10 bit.
    
    const uint16_t sampling_freq = 9600;     // Sample frequency must be > twice signal frequency. (Timer driven ADC limit 76.9kSPS - see data sheet)
    //                                            higher frequency needs more samples. Best is 9600
    
    const uint8_t validationRepeats = 1 ;    // number of consecutive repeats of a code required to confirm it is valid. Can be 0 or more.
    //                                            e.g. 2 repeats = 3 consecutive identical codes. Higher is more accurate but slower.
    //                                            use 1 (or even 0 ) for 50mS spacing between tones. 2 for 100mS. 2 or 3 for longer spacing
   
    uint8_t sampleStore[ N ] ;               // sample store. Passed as pointer to library by static init() method

    //row objects
    Goertzel x_goertzel[4] = {
      Goertzel( 1209.0 ),
      Goertzel( 1336.0 ),
      Goertzel( 1477.0 ),
      Goertzel( 1633.0 )
    } ;

    //column objects
    Goertzel y_goertzel[4] = {
      Goertzel( 697.0 ),
      Goertzel( 770.0 ),
      Goertzel( 852.0 ),
      Goertzel( 941.0 )
    } ;

    const char decoder[4][4] = {
      { '1' , '4', '7', '*' } ,
      { '2' , '5', '8', '0' } ,
      { '3' , '6', '9', '#' } ,
      { 'A' , 'B', 'C', 'D' }
    } ;
    
    
};

#endif
