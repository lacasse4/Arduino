#include <FastLED.h>
#define NUM_LEDS 60
#define DATA_PIN 6

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5,400);
}

void loop1() { 
  // a single red led
  leds[0] = CRGB(255, 0, 0); 
  FastLED.show(); 
  delay(500); 
}

void loop3() { 
  // 3 first led as RGB
  leds[0] = CRGB::Red; 
  leds[1] = CRGB::Green;
  leds[2] = CRGB::Blue;
  FastLED.show(); 
  delay(30); 
}

void loop() {
  // scan all leds one by one
  for(int dot = 0; dot < NUM_LEDS; dot++) { 
    leds[dot] = CRGB::Blue;
    FastLED.show(); 
    // clear this led for the next time around the loop
    leds[dot] = CRGB::Black;
    delay(30);
  }
    for(int dot = NUM_LEDS-1; dot >= 0; dot--) { 
    leds[dot] = CRGB::Blue;
    FastLED.show(); 
    // clear this led for the next time around the loop
    leds[dot] = CRGB::Black;
    delay(30);
  }
}

void loop4() {
  // max power
  for(int dot = 0; dot < NUM_LEDS; dot++) { 
    leds[dot] = CRGB::White;
    FastLED.show(); 
    delay(100);
  }
}