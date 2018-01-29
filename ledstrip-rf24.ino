
/*
* Getting Started example sketch for nRF24L01+ radios
* This is a very basic example of how to send data from one node to another
* Updated: Dec 2014 by TMRh20
*/

#include <SPI.h>
#include "RF24.h"

#include <FastLED.h>

// LED sepecific items

#define NUM_STRIPS 3
#define NUM_LEDS_PER_STRIP 16
#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS
#define DATA_PIN 3
#define DATA_PIN2 5
#define DATA_PIN3 6
#define COLOR_ORDER BRG
#define LED_TYPE WS2811                                       // Using APA102, WS2812, WS2801. Don't forget to change LEDS.addLeds.
#define LED_BRIGHT 150

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

  delay( 2000 ); // power-up safety delay
  
  Serial.begin(115200);

  // Initialize LEDs - we want to put on a bit of a show to know things are working before we go to the main RF loop
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);

  FastLED.addLeds<LED_TYPE, DATA_PIN2, COLOR_ORDER>(leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, DATA_PIN3, COLOR_ORDER>(leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);


  // Blank out any lit LEDS
  fill_solid(leds,NUM_LEDS, CRGB::Black);
  FastLED.delay(1000);

  Serial.println("Running LED Test");
  //LEDtestShow();
  ledOutlineTest(LED_BRIGHT);
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
//
  while(1) { 
      // LEDtestShow();
      waveMaker();
      delay(5000);
    }


 




/****************** Pong Back Role ***************************/

  if ( role == 0 )
  {
    unsigned long got_time = 0;
    
//    if( radio.available()){
//                                                                    // Variable for the received timestamp
//      while (radio.available()) {                                   // While there is data ready
//        radio.read( &got_time, sizeof(unsigned long) );             // Get the payload
//      }
//     
//      radio.stopListening();                                        // First, stop listening so we can talk   
//      radio.write( &got_time, sizeof(unsigned long) );              // Send the final one back.      
//      radio.startListening();                                       // Now, resume listening so we catch the next packets.     
//      Serial.print(F("Sent response "));
//      Serial.println(got_time);  
//   }

//    if (got_time == 100) {
//      
//      Serial.println(F("LED Routine"));
//      /////////////// LED Stuff /////////////
//      for(int dot = NUM_LEDS; dot >= 0 ; dot--) { 
//        leds[dot] = CRGB::Green; 
//        FastLED.show(); 
//                
//        for(int fade = 0; fade < 12; fade++) {
//          leds[dot].fadeToBlackBy( 64 );
//          FastLED.show();
//        }
//        delay(50); 
//        leds[dot] = CRGB::Black;
//      }  
//
//    }
    got_time = 0;
  }




/****************** Change Roles via Serial Commands ***************************/

  if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    if ( c == 'T' && role == 0 ){      
      Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
      role = 1;                  // Become the primary transmitter (ping out)
    
    } else if ( c == 'R' && role == 1 ){
      Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));      
       role = 0;                // Become the primary receiver (pong back)
       radio.startListening();
    }
    
    if ( c == 'L' ) {
        LEDtestShow();
    }
    
  }


} // Loop



void LEDtestShow() {

  Serial.println("<<<<<<<<<<<<<< LED TEST SHOW START >>>>>>>>>>>>>>>>>>>>!");

  int i = 0; 
  int ledBrightness = LED_BRIGHT;
  int waitBlack = 1;
  int colorChangeFlag = 0;   
  int lastSinPos = 0;
  int latchMiddle = 0;
  
  while ( (i < COLOR_SIZE) || waitBlack ) {       // exit while after all color changes and all leds fade to black

      int endOfStrip = sinelon(ledBrightness, latchMiddle);     //when EoS=0 we want to change to the next color, but only after the first run

      // DEBUG
      if ( lastSinPos != endOfStrip ) {
        Serial.println(endOfStrip);
        lastSinPos = endOfStrip; 
      }

      // only light middle once we hit middle
      if (endOfStrip == (NUM_LEDS / 2) ) { 
        latchMiddle = 1; 
      }  

      
      // when at end of strip and change flag = 0, set change flag=1
      if ( endOfStrip == 0 && colorChangeFlag == 0 ) {        
          colorChangeFlag = 1;

      // set change flag after we've lit the first light
      } else if ( colorChangeFlag == 1 && endOfStrip > 0  ) {     
          colorChangeFlag = 0;
          gHue = colorValues[i]; 
          (i < COLOR_SIZE) ? i++ : i;
                    
      }
      
      // no more color changes, set led brightness to 0 for subsequent calls to sinelon for fade-to-black
      if ( i == COLOR_SIZE && (endOfStrip == 0 || ledBrightness == 0) ) {
          ledBrightness = 0;                        
          waitBlack = checkAllFaded();              // returns value of brightest led in strip
          if (! waitBlack ) { fadeToBlackBy( leds, NUM_LEDS, 255); }     // run fadeToBlack one more time for good measure
      }


      FastLED.delay(1000/120);      // this delay also runs show() for some reason
  
  }
  Serial.println("<<<<<<<<<<<<<< LED TEST SHOW END >>>>>>>>>>>>>>>>>>>>!");
  
}


int sinelon(int ledBrightness, int latchMiddle) {

    int ledCenter = (NUM_LEDS / 2);   
    
    // fades led by thisfade amount
    fadeToBlackBy( leds, NUM_LEDS, thisfade);

    // light up the "Markers" Top, bottom and middle
    leds[0] += CHSV( gHue, 255, ledBrightness);             // bottom
    if (latchMiddle) { leds[ledCenter]  += CHSV( gHue, 255, ledBrightness); }     // middle
    leds[NUM_LEDS-1] += CHSV( gHue, 255, ledBrightness);    // top
  
    int firstHalf  = beatsin8( thisbeat, 0, ledCenter, 0, 192 );    // rotating phase by 192 to force a zero start
    int secondHalf = (NUM_LEDS-1) - firstHalf;
  
    if ( ledBrightness > 0 ) {
      leds[firstHalf]  += CHSV( gHue, 255, ledBrightness);
      leds[secondHalf] += CHSV( gHue, 255, ledBrightness);
      
  
    }

    return (firstHalf);     // return numeric position in sine wave
  
}

int checkAllFaded() {

    int brightest = 0;
    
    for (int i = 0; i < NUM_LEDS-1; i++) {
      brightest = (leds[i].getAverageLight() > brightest) ? leds[i].getAverageLight() : brightest;
    }

    Serial.println(brightest);
    
    return brightest;
}


void ledOutlineTest(int ledBrightness) { 

    Serial.println("In ledOutlineTest");
    
    for (int runIt=0; runIt < 3; runIt++) {

      // Light bottom of each
      for (int a=0; a < NUM_LEDS; a += NUM_LEDS_PER_STRIP) {
        leds[a].setHue(HUE_RED);    // bottom
      }
      
      // Light top of each
      for (int a= NUM_LEDS_PER_STRIP-1; a < NUM_LEDS; a += NUM_LEDS_PER_STRIP ) {
        leds[a].setHue(HUE_GREEN);    // top
      }
      
      // Light middle of each
      for (int a=NUM_LEDS_PER_STRIP/2; a <= NUM_LEDS; a += NUM_LEDS_PER_STRIP ) {
        leds[a].setHue(HUE_BLUE);    // middle
      }

      FastLED.show();
      
      do {
        fadeToBlackBy( leds, NUM_LEDS, 25);
        FastLED.delay(50);
      } while ( (checkAllFaded() - 10) );

      Serial.println("runIt");
      Serial.println(runIt);
    }

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  Serial.println("Leaving ledOutlineTest");
    
}

void waveMaker() {

  Serial.println("Entering WAVE...");
  int myBPM = 40; 
  
  while (1) {
    
    int wavePos1 = beatsin8( myBPM, 0,  NUM_LEDS_PER_STRIP-1, 0, 0 );  
    int wavePos2 = beatsin8( myBPM, 0 + NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP*2-1, 0, 16);   // middle strip is shorter!!!
    int wavePos3 = beatsin8( myBPM, 0 + (NUM_LEDS_PER_STRIP * 2), NUM_LEDS -1, 0, 32);    

    leds[wavePos1].setHue(HUE_BLUE);
    leds[wavePos2].setHue(HUE_BLUE);
    leds[wavePos3].setHue(HUE_BLUE);
       
    //fadeToBlackBy( leds, NUM_LEDS, 1);
    nscale8_video( leds, NUM_LEDS, 10);
    FastLED.delay(1000/120);
    
  }
     
  
}
