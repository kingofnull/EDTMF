/*
  Original text created by Jacob Rosenthal:
  
  The Goertzel algorithm is long standing so see
  http://en.wikipedia.org/wiki/Goertzel_algorithm for a full description.
  It is often used in DTMF tone detection as an alternative to the Fast
  Fourier Transform because it is quick with low overheard because it
  is only searching for a single frequency rather than showing the
  occurrence of all frequencies.

  This work is entirely based on the Kevin Banks code found at
  http://www.embedded.com/design/configurable-systems/4024443/The-Goertzel-Algorithm
  so full credit to him for his generic implementation and breakdown. I've
  simply massaged it into an Arduino library. I recommend reading his article
  for a full description of whats going on behind the scenes.

  Created by Jacob Rosenthal, June 20, 2012.
  Released into the public domain.

  Modifications 6v6gt 09.09.2019
  1. Restructure instance variables to permit multiple instances of class.
  2. Make sample array static to share it between instances
  3. Pass in sample array as pointer so it can be reused outside class.
  4. Drive ADC by timer1 instead of polling ADC in loop()
      and reduce resolution to 8 bits.
  5. Separate coeeficient calculation from constructor because it
       now relies on data unknown at invocation (sample rate)
  5. Some consolidation of methods.     
  6. Software "as is". No claims made about its suitability for any use.
       Use at your own risk. Special care required if you use other
       analog inputs or an AVR chip other than ATmega328P (Uno etc.).    
*/

#ifndef Goertzel_h
#define Goertzel_h

#include "Arduino.h"

// library interface description
class Goertzel
{
  public:
    Goertzel( float ); // target frequency 
    
    void getCoefficient() ;   // calculates coefffiecient for its instance
    float detect();  // returns magnitude of detection.

    static void init( uint8_t *, uint16_t, uint16_t ) ;  // initialise with sampleStore array and size, sample frequency

    static uint8_t * testData ;     // defined in .cpp
    static uint16_t N;              // sample array size ditto
    static uint16_t SAMPLING_FREQUENCY ;
    static volatile bool testDataReady  ;    // flag indicates sample collection complete, or not.


  private:
    float _TARGET_FREQUENCY;
    float coeff;
    float Q1;
    float Q2;

};

#endif

