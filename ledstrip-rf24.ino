
/*
* Getting Started example sketch for nRF24L01+ radios
* This is a very basic example of how to send data from one node to another
* Updated: Dec 2014 by TMRh20
*/

#include <SPI.h>
#include "RF24.h"

#include <FastLED.h>
#define NUM_LEDS 17
#define DATA_PIN 5
#define COLOR_ORDER BRG
#define LED_TYPE WS2811                                       // Using APA102, WS2812, WS2801. Don't forget to change LEDS.addLeds.


#define LED_BRIGHT 100
#define LED_DIM 5


// LED sepecific items
CRGB leds[NUM_LEDS];
uint8_t gHue=0;
uint8_t thisbeat =  25;                                       // Beats per minute for first part of dot.
uint8_t thisfade =  8;                                       // How quickly does it fade? Lower = slower fade rate.

int colorChangeFlag = 0;
#define COLOR_SIZE 3
int colorValues[COLOR_SIZE] = { 0, 96, 160 };


/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 1;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7,8);
/**********************************************************/

byte addresses[][6] = {"1Node","2Node"};

// Used to control whether this node is sending or receiving
bool role = 0;






void setup() {
  Serial.begin(115200);

  // Initialize LEDs - we want to put on a bit of a show to know things are working before we go to the main RF loop
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  
  Serial.println("Running LED Test");
  LEDtestShow();
  Serial.println("LED Test done.");

  
  Serial.println(F("RF24/examples/GettingStarted"));
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));


  
  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
 // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);
  
  // Open a writing and reading pipe on each radio, with opposite addresses
  if(radioNumber){
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1,addresses[0]);
  }else{
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }
  
  // Start the radio listening for data
  radio.startListening();


}


void loop() {
  
  
  // REMOVE THIS WHEN READY TO TEST LOOP!!!!!
  ///////////////////////////////////////////////////////

  while(1) { 
      // LEDtestShow();
    }


/****************** Pong Back Role ***************************/

  if ( role == 0 )
  {
    unsigned long got_time = 0;
    
    if( radio.available()){
                                                                    // Variable for the received timestamp
      while (radio.available()) {                                   // While there is data ready
        radio.read( &got_time, sizeof(unsigned long) );             // Get the payload
      }
     
      radio.stopListening();                                        // First, stop listening so we can talk   
      radio.write( &got_time, sizeof(unsigned long) );              // Send the final one back.      
      radio.startListening();                                       // Now, resume listening so we catch the next packets.     
      Serial.print(F("Sent response "));
      Serial.println(got_time);  
   }

    if (got_time == 100) {
      Serial.println(F("LED Routine"));
      /////////////// LED Stuff /////////////
      for(int dot = NUM_LEDS; dot >= 0 ; dot--) { 
        leds[dot] = CRGB::Green; 
        FastLED.show(); 
                
        for(int fade = 0; fade < 12; fade++) {
          leds[dot].fadeToBlackBy( 64 );
          FastLED.show();
        }
        delay(50); 
        leds[dot] = CRGB::Black;
      }  

    }
    got_time = 0;
  }




/****************** Change Roles via Serial Commands ***************************/

  if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    if ( c == 'T' && role == 0 ){      
      Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
      role = 1;                  // Become the primary transmitter (ping out)
    
   }else
    if ( c == 'R' && role == 1 ){
      Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));      
       role = 0;                // Become the primary receiver (pong back)
       radio.startListening();
       
    }
  }


} // Loop



void LEDtestShow() {

  Serial.println("HI!");

  int i = 0; 
  int ledBrightness = LED_BRIGHT;
  int waitBlack = 1;
  
  while ( (i < COLOR_SIZE) || waitBlack ) {

      int endOfStrip = sinelon(ledBrightness);     //when EoS=0 we want to change to the next color

      
      if ( endOfStrip == 0 && colorChangeFlag == 0 ) {        // when at end of strip and change flag = 0, set pre-change flag = 2
          colorChangeFlag = 2;
      
        
      } else if ( colorChangeFlag == 1 && endOfStrip > 0 && i < COLOR_SIZE ) {     //  
          colorChangeFlag = 0;
          (i < COLOR_SIZE) ? i++ : i;
          
      } else if ( colorChangeFlag == 2 && endOfStrip > 0  ) {
          colorChangeFlag = 1;
          gHue = colorValues[i]; 
                    
      }

      Serial.print("i: ");
      Serial.println(i);
      Serial.print("endOfStrip: ");
      Serial.println(endOfStrip);
      Serial.print("colorChangeFlag: ");
      Serial.println(colorChangeFlag);
      
      if ( i == COLOR_SIZE && endOfStrip == 0 ) {
          ledBrightness = 0;
          waitBlack = checkAllFaded();
          //Serial.print("waitBlack: ");
          //Serial.println(waitBlack);
          //Serial.print("endOfStrip: ");
          //Serial.println(endOfStrip);
      }
      
      FastLED.show();
      FastLED.delay(1000/120);
  
  }
       
}


int sinelon(int ledBrightness) {

  //Serial.println(gHue);
  int ledCenter = NUM_LEDS / 2;
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, thisfade);
  int pos = beatsin16( thisbeat, ledCenter, (NUM_LEDS-1) );
  int pos2 = ledCenter - (pos-ledCenter);
  leds[pos] += CHSV( gHue, 255, ledBrightness);
  leds[pos2] += CHSV( gHue, 255, ledBrightness);

  return (pos2);
  
}

int checkAllFaded() {

    int brightest = 0;
    
    for (int i = 0; i < NUM_LEDS; i++) {
      brightest = (leds[i].getLuma() > brightest) ? leds[i].getLuma() : brightest;
    }

    return brightest;
}


////////// NO LONG USING THIS FUNCTION
/////////////////////////////////////////////////////////////////////////////
void wipeFromCenter (int r, int g, int b, int milSecPause, int preDelay) {

  // find middle of strip
  int ledCenter = NUM_LEDS / 2;
  //ledCenter = abs(ledCenter);
  ledCenter = abs8(ledCenter);

  delay(preDelay);

  // determine prominent color used and set tailing dim value
  int rDim = (r > 0) ? LED_DIM : 0;
  int gDim = (g > 0) ? LED_DIM : 0;
  int bDim = (b > 0) ? LED_DIM : 0;
  
  // start wipe
  for (int count = 0; count < ledCenter; count++) {

    // light up next outer leds
    leds[ledCenter - count] = CRGB(r,g,b);
    leds[ledCenter + count] = CRGB(r,g,b);
    
    // blank prvious head and tail pixel  
    fadeToBlackBy( leds, NUM_LEDS, 127);
//    leds[ledCenter - (count-1)] = CRGB(rDim, gDim, bDim);
//    leds[ledCenter + (count-1)] = CRGB(rDim, gDim, bDim);
    FastLED.show();
    delay(milSecPause);
  }

  // need a final blank for the outer markers
  fadeToBlackBy( leds, NUM_LEDS, 127);
  leds[1] = CRGB(rDim, gDim, bDim);
  leds[NUM_LEDS-2] = CRGB(rDim, gDim, bDim);
  FastLED.show();
    
  // Reverse the wipe
  for (int count = ledCenter; 0 <= count; count--) {

    // light up next outer leds
    leds[ledCenter - count] = CRGB(r,g,b);
    leds[ledCenter + count] = CRGB(r,g,b);
    
    // blank prvious head and tail pixel  
    fadeToBlackBy( leds, NUM_LEDS, 127);
//    leds[ledCenter - (count+1)] = CRGB(rDim, gDim, bDim);
//    leds[ledCenter + (count+1)] = CRGB(rDim, gDim, bDim);
    FastLED.show();
    delay(milSecPause);
  }

  // need a final blank for the outer markers
  fadeToBlackBy( leds, NUM_LEDS, 127);
  leds[1] = CRGB(rDim, gDim, bDim);
  leds[NUM_LEDS-2] = CRGB(rDim, gDim, bDim);
  FastLED.show(); 

}

void flashCenterAndEnds () {

  

}
