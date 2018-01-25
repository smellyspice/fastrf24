
/*
* Getting Started example sketch for nRF24L01+ radios
* This is a very basic example of how to send data from one node to another
* Updated: Dec 2014 by TMRh20
*/

#include <SPI.h>
#include "RF24.h"

#include <FastLED.h>

// LED sepecific items
#define NUM_LEDS 17
#define DATA_PIN 5
#define COLOR_ORDER BRG
#define LED_TYPE WS2811                                       // Using APA102, WS2812, WS2801. Don't forget to change LEDS.addLeds.
#define LED_BRIGHT 100

CRGB leds[NUM_LEDS];
uint8_t gHue=0;
uint8_t thisbeat =  25;    // was 25                                   // Beats per minute for first part of dot.
uint8_t thisfade =  8;                                       // How quickly does it fade? Lower = slower fade rate.

#define COLOR_SIZE 3
int colorValues[COLOR_SIZE] = { 0, 96, 160 };           // 0 = RED  96 = GREEN  160 = BLUE

// END ------ LED sepecific items


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

  Serial.println("<<<<<<<<<<<<<< LED TEST SHOW START >>>>>>>>>>>>>>>>>>>>!");

  int i = 0; 
  int ledBrightness = LED_BRIGHT;
  int waitBlack = 1;
  int colorChangeFlag = 0;   

  
  while ( (i < COLOR_SIZE) || waitBlack ) {       // exit while after all color changes and all leds fade to black

      int endOfStrip = sinelon(ledBrightness);     //when EoS=0 we want to change to the next color, but only after the first run

      
      // when at end of strip and change flag = 0, set change flag=1
      if ( endOfStrip == 0 && colorChangeFlag == 0 ) {        
          colorChangeFlag = 1;
          Serial.println("ColorChangeFlag = SET");


      // set change flag after we've lit the first light
      } else if ( colorChangeFlag == 1 && endOfStrip > 0  ) {     
          colorChangeFlag = 0;
          gHue = colorValues[i]; 
          (i < COLOR_SIZE) ? i++ : i;
          Serial.print("Color Changed to #: ");
          Serial.println(i);
                    
      }
      
      // no more color changes, set led brightness to 0 for subsequent calls to sinelon for fade-to-black
      if ( i == COLOR_SIZE && endOfStrip == 0 ) {
          ledBrightness = 0;                        
          waitBlack = checkAllFaded();              // returns value of brightest led in strip

      }


      FastLED.delay(1000/120);      // this delay also runs show() for some reason
  
  }
       
}


int sinelon(int ledBrightness) {

  int ledCenter = NUM_LEDS / 2;   
  
  // fades led by thisfade amount
  fadeToBlackBy( leds, NUM_LEDS, thisfade);
  //nscale8_video(leds, NUM_LEDS, 192);
  
  int firstHalf  = beatsin8( thisbeat, 0, ledCenter, 0, 192 );    // rotating phase by 192 to force a zero start
  int secondHalf = (NUM_LEDS-1) - firstHalf;

  if ( ledBrightness > 0 ) {
    Serial.print("ledBrightness: ");
    Serial.println(ledBrightness);
    leds[firstHalf]  += CHSV( gHue, 255, ledBrightness);
    leds[secondHalf] += CHSV( gHue, 255, ledBrightness);
  }

  Serial.println("=========================");
  Serial.println("First / Second");
  Serial.println(firstHalf);
  Serial.println(secondHalf);
  Serial.println("=========================");

  return (firstHalf);
  
}

int checkAllFaded() {

    int brightest = 0;
    
    for (int i = 0; i < NUM_LEDS; i++) {
      brightest = (leds[i].getLuma() > brightest) ? leds[i].getLuma() : brightest;
        Serial.println("i / Luma:");
        Serial.println(i);
        Serial.println(leds[i].getLuma());
        
    }
    Serial.print("Returning ");
    Serial.println(brightest);
    return brightest;
}

