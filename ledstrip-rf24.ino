
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

#define LED_BRIGHT 100
#define LED_DIM 5


// LED sepecific items
CRGB leds[NUM_LEDS];
uint8_t gHue=0;
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



  // Initialize LEDs - we want to put on a bit of a show to know things are working before we go to the main RF loop
  FastLED.addLeds<WS2811, DATA_PIN, BRG>(leds, NUM_LEDS);
  LEDtestShow();

 

}

void loop() {
  
  
  // REMOVE THIS WHEN READY TO TEST LOOP!!!!!
  ///////////////////////////////////////////////////////

  while(1) { LEDtestShow(); }


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

  int i; 
  
  while ( i < COLOR_SIZE ) {
    
      int endOfStrip = sinelon();
      
      if ( endOfStrip == 0 && colorChangeFlag == 0 ) { 
        gHue = colorValues[i]; 
        colorChangeFlag = 1;
        i++;
        Serial.println(endOfStrip);
        Serial.println(gHue);
      } else if (colorChangeFlag ==1 && endOfStrip > 0) {
        colorChangeFlag = 0;
        Serial.println(endOfStrip);
      }
        
      FastLED.show();
      FastLED.delay(1000/120);
  
  }
 
}


int sinelon() {

  //Serial.println(gHue);
  int ledCenter = NUM_LEDS / 2;
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 20, ledCenter, (NUM_LEDS-1) );
  leds[pos] += CHSV( gHue, 255, LED_BRIGHT);
  int pos2 = ledCenter - (pos-ledCenter);
  leds[pos2] += CHSV( gHue, 255, LED_BRIGHT);

  return (pos2);
}


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
