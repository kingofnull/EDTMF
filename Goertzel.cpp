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


#include "Arduino.h"
#include "Goertzel.h"

// set by Goertzel::init()
uint8_t * Goertzel::testData;             // static declaration in .h
uint16_t Goertzel::N ;                    // ditto
uint16_t Goertzel::SAMPLING_FREQUENCY ;   // ditto

volatile bool Goertzel::testDataReady = false ; // static declaration in .h


//ADC interrupt service routine
ISR(ADC_vect) {
  // load sample buffer on sample conversion.
  static uint16_t sampleIndex = 0 ;
  if ( ! Goertzel::testDataReady   ) {
    if ( sampleIndex < Goertzel::N  ) {
      *(Goertzel::testData + sampleIndex++ )  = ADCH ; // 8 bits. Direct adressing into byte buffer
    }
    else  {
      Goertzel::testDataReady = true ;    // make buffer available to consumer
      sampleIndex = 0 ;
    }
  }
  TIFR1 = _BV(ICF1); // reset interrupt flag
}


// constructor
Goertzel::Goertzel(float TARGET_FREQUENCY  )
{
  _TARGET_FREQUENCY = TARGET_FREQUENCY; //should be integer of SAMPLING_RATE/N
}


//static method
void Goertzel::init( uint8_t *sampleArray , uint16_t sampleArraySize, uint16_t sampleFrequency ) {
  // set up sample array, number of samples and sample frequency.

  // load class static variables
  testData = sampleArray ;
  N = sampleArraySize ;
  SAMPLING_FREQUENCY = sampleFrequency ;


  // initialise ADC and Timer1. Timer1 triggers ADC at frequency SAMPLING_FREQUENCY.
  // ISR(ADC_vect) called when conversion complete.
  cli() ;

  // Setup Timer1 for chosen sampling frequency.
  TCCR1A = 0;
  TCCR1B = _BV(CS10)  |    // Bit 2:0 – CS12:0: Clock Select =  no prescaler
           _BV(WGM13) |    // WGM 12 = CTC ICR1 Immediate MAX
           _BV(WGM12);     // WGM 12 ditto
  ICR1 = ( (F_CPU ) / SAMPLING_FREQUENCY ) - 1;


  // Setup ADC for triggering by timer1; 8bit resolution; Analog Port PC0 (pin A0 ) ADMUX
  ADMUX =  _BV(REFS0) ;    // Fixed AVcc reference voltage for ATMega328P
  ADMUX |= _BV(ADLAR) ;    // left adjust conversion result in ADCH (8bit)
  DIDR0 |= _BV(ADC0D);     // DIDR0  Digital Input Disable Register 0
  ADCSRB = _BV(ADTS2) |    // Bit 2:0  ADTS[2:0]: ADC Auto Trigger Source
           _BV(ADTS1) |    // Timer/Counter1 Capture Event
           _BV(ADTS0);     // ditto


  ADCSRA = _BV(ADEN) |      // Bit 7   ADEN: ADC Enable
           _BV(ADSC) |      // Bit 6   ADSC: ADC Start Conversion
           _BV(ADATE) |     // Bit 5   ADATE: ADC Auto Trigger Enable
           _BV(ADIE) |      //
           _BV(ADPS0) |     // Bits 2:0  ADPS[2:0]: ADC Prescaler Select Bits  (div 8 )
           _BV(ADPS1);      // ditto
  sei() ;

}



// instance method
void Goertzel::getCoefficient( void ) {
  // previously in constructor. Now SAMPLING_FREQUENCY unknown at invocation time.
  float omega = (2.0 * PI * _TARGET_FREQUENCY) / SAMPLING_FREQUENCY;
  coeff = 2.0 * cos(omega);
}



// instance method
float Goertzel::detect()
{
  Q2 = 0;
  Q1 = 0;
  for ( uint16_t index = 0; index < N; index++)
  {
    // byte sample is ( *( testData + index ) );
    float Q0;
    Q0 = coeff * Q1 - Q2 + (float) ( *( testData + index ) - 128 ) ; //  128 for 8bit; 512 for 10bit resolution.
    Q2 = Q1;
    Q1 = Q0;
  }

  /* standard Goertzel processing. */
  float magnitude = sqrt(Q1 * Q1 + Q2 * Q2 - coeff * Q1 * Q2);
  return magnitude  ;
}

