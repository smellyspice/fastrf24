
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

CRGB leds[NUM_LEDS];

// prototying this function so it accepts the optional default
void clearStripWithDelay (int milSecPause, int OptionalPreDelay=0);


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

  //fill_solid( leds, NUM_LEDS, CRGB(50,0,200));
  clearStripWithDelay(1000);

  // Set head and tail RED
  leds[0] = CRGB(255,0,0);
  leds[NUM_LEDS-1] = CRGB(255,0,0);
  Serial.println("Red");
  FastLED.show();
  clearStripWithDelay(400, 800);

  // Set head and tail GREEN
  leds[0] = CRGB(0,255,0);
  leds[NUM_LEDS-1] = CRGB(0,255,0);
  Serial.println("Green");
  FastLED.show();
  clearStripWithDelay(400,800);

  // Set head and tail BLUE
  leds[0] = CRGB(0,0,255);
  leds[NUM_LEDS-1] = CRGB(0,0,255);
  Serial.println("Blue");
  FastLED.show();
  clearStripWithDelay(400,800);

  wipeFromCenter();
 
}

void clearStripWithDelay (int milSecPause, int OptionalPreDelay=0) {

  delay(OptionalPreDelay);
  FastLED.clear();
  FastLED.show();
  delay(milSecPause);
  
}

void clearLEDWithDelay (int LedNumber, int milSecPause) {

  leds[LedNumber] = CRGB::Black;
  FastLED.show();
  delay(milSecPause);
  
}

void wipeFromCenter () {

  // find middle of strip
  int ledCenter = NUM_LEDS / 2;
  ledCenter = abs(ledCenter);
  
  // start wipe
  for (int count = 0; count < (ledCenter); count ++) {

    Serial.println("Count / ledCenter / Before / After / Before -1 / After + 1");
    Serial.println(count);
    Serial.println(ledCenter);
    Serial.println(ledCenter - count);
    Serial.println(ledCenter + count);
    Serial.println(ledCenter - (count-1));
    Serial.println(ledCenter + (count-1));
    
    // light up next outer leds
    leds[ledCenter - count] = CRGB::Blue;
    leds[ledCenter + count] = CRGB::Blue;
    
    // blank prvious head and tail pixel  
    /// leds[ledCenter] = CRGB(1,1,1);    // blank center pixel before looping  
    leds[ledCenter - (count-1)] = CRGB(1,1,1);
    leds[ledCenter + (count-1)] = CRGB(1,1,1);
    FastLED.show();
    delay(30);
  }
  
}
