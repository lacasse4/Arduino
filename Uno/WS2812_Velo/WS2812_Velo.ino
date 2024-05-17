#include <FastLED.h>
#define DATA_PIN 14
#define SCALE_DOWN_SHIFT 4
#define MAX_INTENSITY (SCALE_DOWN_SHIFT==0?255:256>>SCALE_DOWN_SHIFT)

#define MS_DELAY 50
#define MS_FADE_OUT_DELAY 25
#define MS_START_SEQUENCE_DELAY 250
#define MS_ANIMATION_TIME 10000

#define THEATER_CHASE_COUNT 31
#define THEATER_CHASE_DELAY 50
#define THEATER_CHASE_INCR   3

#define KITT_EYE_SIZE 8
#define KITT_SPEED_DELAY 10
#define KITT_RETURN_DELAY 50

#define CYLON_EYE_SIZE 4
#define CYLON_SPEED_DELAY 10
#define CYLON_RETURN_DELAY 50

#define TWINKLE_DELAY 50
#define TWINKLE_LEDS  10

#define RUNNING_LIGHTS_DELAY 50 

#define NUM_LEDS 69
#define NUM_LEDS_MIN 21
#define NUM_LEDS_MID 40
#define NUM_LEDS_BUFF_SIZE (NUM_LEDS+20)  // just to be safe
CRGB leds[NUM_LEDS_BUFF_SIZE];

#define NUM_PRIMARY_COLORS 3
CRGB primary_colors[NUM_PRIMARY_COLORS] = {
  CRGB::Red,
  CRGB::Yellow,
  CRGB::Blue
};

#define NUM_SECONDARY_COLORS 6
CRGB secondary_colors[NUM_SECONDARY_COLORS] = {
  CRGB::Red,
  CRGB::Yellow,
  CRGB::Blue,
  CRGB::Green,
  CRGB::Orange,
  CRGB::Purple
};

#define NUM_SIMPLE_COLORS 8
CRGB simple_colors[NUM_SIMPLE_COLORS] = {
  CRGB::Red,
  0x808000, // Yellow
  CRGB::Blue,
  CRGB::Green,
  CRGB::DarkOrange,
  CRGB::Violet,
  CRGB::Purple,
  CRGB::Grey
};

#define RED     simple_colors[0]
#define YELLOW  simple_colors[1]
#define BLUE    simple_colors[2]
#define GREEN   simple_colors[3]
#define ORANGE  simple_colors[4]
#define VIOLET  simple_colors[5]
#define PURPLE  simple_colors[6]
#define WHITE   simple_colors[7]
#define BLACK   CRGB::Black

#define NUM_VISIBLE_COLORS 51
CRGB visible_colors[NUM_VISIBLE_COLORS] = {
  CRGB::AliceBlue, 
  CRGB::Aqua,
  CRGB::Aquamarine,
  CRGB::Azure,
  CRGB::Bisque,
  CRGB::BlanchedAlmond,
  CRGB::Blue,
  CRGB::Cyan,
  CRGB::Chartreuse,
  CRGB::Coral,
  CRGB::Cornsilk,
  CRGB::Cyan,
  CRGB::DarkOrange,
  CRGB::DeepPink,
  CRGB::DeepSkyBlue,
  CRGB::DodgerBlue,
  CRGB::Green,
  CRGB::FairyLight,
  CRGB::FloralWhite,
  CRGB::Fuchsia,
  CRGB::GhostWhite,
  CRGB::Gold,
  CRGB::GreenYellow,
  CRGB::Honeydew,
  CRGB::HotPink,
  CRGB::Ivory,
  CRGB::LavenderBlush,
  CRGB::LemonChiffon,
  CRGB::LightCyan,
  CRGB::LightPink,
  CRGB::LightSalmon,
  CRGB::LightSlateGray,
  CRGB::LightYellow,
  CRGB::Lime,
  CRGB::Magenta,
  CRGB::MintCream,
  CRGB::MistyRose,
  CRGB::Moccasin,
  CRGB::NavajoWhite,
  CRGB::Orange,
  CRGB::OrangeRed,
  CRGB::PapayaWhip,
  CRGB::PeachPuff,
  CRGB::Pink,
  CRGB::Red,
  CRGB::Seashell,
  CRGB::Snow,
  CRGB::SpringGreen,
  CRGB::Tomato,
  CRGB::White,
  CRGB::Yellow,
};

unsigned long start_time;
#define set_timer()   start_time = millis()
#define check_timer() if (millis() - start_time > MS_ANIMATION_TIME) break


// ***************************************************
//                      SETUP
// ***************************************************

void setup() {
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  // FastLED.setMaxPowerInVoltsAndMilliamps(5,400);

  // scale max led luminosity to reduce power consumption
  // All colors in color arrays have their Max luminosity scaled down 
  // with a bitwise right shift of SCALE_DOWN_SHIFT.
  
  scale_down_color_array(primary_colors,   NUM_PRIMARY_COLORS);
  scale_down_color_array(secondary_colors, NUM_SECONDARY_COLORS);
  scale_down_color_array(simple_colors,    NUM_SIMPLE_COLORS);
  scale_down_color_array(visible_colors,   NUM_VISIBLE_COLORS);

  // set pin D19 (A5) in high impedance mode so we can hook 
  // the signal resistor onto this terminal on the break out board
  // without interfering with the Ardiuno logic.
  
  pinMode(19, INPUT);  
  delay(10);

//  Serial.begin(9600);
//  delay(10);

//  Serial.println(sin16(32768.0));    // -> 0                           === sin(pi   ou 180 deg)
//  Serial.println(sin16(32768.0/2));  // -> 32645 (32645/32767 = 0,996) === sin(pi/2 ou  90 deg)
//  Serial.println(sin16(32768.0/6));  // -> 16279 (16279/32767 = 0,497) === sin(pi/6 ou  30 deg) 
//  Serial.println(sin16(32768.0/4));  // -> 23170 (23170/32767 = 0,707) === sin(pi/4 ou  45 deg)

  show_start_sequence();
}

void show_start_sequence() {

  // flash strip 3 times to show we are restarting
  for (int i = 0; i < 3; i++) {
    fill_solid(leds, NUM_LEDS, WHITE);
    showStrip();
    delay(MS_START_SEQUENCE_DELAY);
    
    fill_solid(leds, NUM_LEDS, BLACK);
    showStrip();
    delay(MS_START_SEQUENCE_DELAY);
  }
  
  fill_solid(leds, NUM_LEDS, WHITE);
  showStrip();
  delay(MS_START_SEQUENCE_DELAY);
  auto_fade_out();
}


// Scale down color by bitwise shifting each color component by SCALE_DOWN_SHIFT.
// This is suitable when leds are used at night.
// Also, using SCALE_DOWN_SHIFT of 3 (1/8th of original intensity) on a system
// comprising 314 leds (using WS2812B) significantly reduces current consumption.
// ex: theatre_chase() will consume approx. 0.4 Amp with with CRGB::Grey scaled down
// rather and 2.6 Amp with the original CRGB::Grey.

CRGB scale_down_color(CRGB color) {
  color.r = color.r >> SCALE_DOWN_SHIFT;
  color.g = color.g >> SCALE_DOWN_SHIFT;
  color.b = color.b >> SCALE_DOWN_SHIFT;
  return color;
}

void scale_down_color_array(CRGB color_array[], int n) {
  for (int i = 0; i < n; i++) {
    color_array[i] = scale_down_color(color_array[i]);
  }
} 

// ***************************************************
//                  MAIN LOOP
// ***************************************************

// array of "auto" functions (callable without parameter) 
#define NUM_FUNCTIONS 11
void (*display_functions[NUM_FUNCTIONS])() = {
  auto_changing_colors,
  auto_fire,
  auto_meteor_rain,
  auto_twinkle,
  auto_two_colors_modulation,
  auto_cylon_bounce,
  auto_theater_chase_forward,
  auto_theater_chase_backward,
  auto_KITT_in_and_out,
  auto_running_lights,
  auto_nope
 };

void auto_nope() {}

void loop() {
    int index = random(NUM_FUNCTIONS);
    display_functions[index]();
}

// ***************************************************

CRGB get_random_color(CRGB color_array[], int n) {
  int index = random(n);
  return color_array[index];
}

#define get_random_primary_color()    get_random_color(primary_colors,NUM_PRIMARY_COLORS)
#define get_random_seconddary_color() get_random_color(secondary_colors,NUM_SECONDARY_COLORS)
#define get_random_simple_color()     get_random_color(simple_colors,NUM_SIMPLE_COLORS)
#define get_random_visible_color()    get_random_color(visible_colors,NUM_VISIBLE_COLORS)

int get_random_pixel_index() {
  return random(NUM_LEDS);
}

int get_random_pixel_index_unbalanced() {
  int range = random(3);
  switch (range) {
    case 0: return random(NUM_LEDS_MIN);
    case 1: return random(NUM_LEDS_MID);
    case 2: return random(NUM_LEDS);
  }
}

// ***************************************************
//                    FADE OUT
// ***************************************************

void auto_fade_out() {
  fade_out(MS_FADE_OUT_DELAY);
}

void fade_out(int ms_delay) {
  CRGB buff[NUM_LEDS_BUFF_SIZE];
  float scale;

  memcpy(buff, leds, sizeof(leds));

  for (int i = MAX_INTENSITY; i >= 0 ; i=i-2) {
    scale = (float)i / MAX_INTENSITY;
    for (int j = 0; j < NUM_LEDS; j++) {
      leds[j].r = scale * buff[j].r;
      leds[j].g = scale * buff[j].g;
      leds[j].b = scale * buff[j].b;
    }    
    showStrip();
    delay(ms_delay);
  }
}

// ***************************************************
//                TWO COLORS MODULATION
// ***************************************************

#define MIN_PERIOD      1000  // in ms
#define MAX_PERIOD      4000  // in ms
#define TIME_INCREMENT  10    // in ms
#define NO_COLOR        -1
#define SIN16_2_PI      65536
#define SIN16_MOINS_PI_SUR_2  (65536>>2)

void auto_two_colors_modulation() {
  CRGB color;
  bool enabled[3];
  int ms_period[3];
  int ms_elapsed[3];
  int color_disabled;
  int color_to_disable;
  unsigned long start_time;
  bool even_half;

  enabled[0] = true;
  enabled[1] = true;
  enabled[2] = false;
  color_disabled = 2;
  color = BLACK;
  even_half = random(2) == 0;
  
  for (int i = 0; i < 3; i++) {
    if (enabled[i]) {
      ms_period[i]  = MIN_PERIOD + random(MAX_PERIOD - MIN_PERIOD);
      ms_elapsed[i] = 0;
    }
    else {
      ms_period[i]  = 0;
      ms_elapsed[i] = 0;
    }
  }

  set_timer();

  while (1) {

    // Check if a timer has elapsed. 
    // This will be the next color to disable.
    color_to_disable = NO_COLOR;
    for (int i = 0; i < 3; i++) {
      if (enabled[i]) ms_elapsed[i] += TIME_INCREMENT;
      if (ms_elapsed[i] > ms_period[i]) {
        color_to_disable = i;
        break;
      }
    }

    // if a timer has elapsed for a color, disable this color
    // and enable the color that was disabled before.
    if (color_to_disable != NO_COLOR) {
      enabled[color_disabled]       = true;
      ms_period[color_disabled]     = MIN_PERIOD + random(MAX_PERIOD - MIN_PERIOD);
      ms_elapsed[color_disabled]    = 0;

      enabled[color_to_disable]     = false;
      ms_period[color_to_disable]   = 0;
      ms_elapsed[color_to_disable]  = 0;
      // intensity[color_to_disable]   = 0;
      
      color_disabled = color_to_disable;
    }

    for (int i = 0; i < 3; i++) {
      if (enabled[i]) {
        float angle = SIN16_2_PI*(float)ms_elapsed[i]/ms_period[i] + SIN16_MOINS_PI_SUR_2;
        uint16_t sin16_arg = (uint16_t)((long)angle % SIN16_2_PI);
        color.raw[i] = (byte)((MAX_INTENSITY>>1)*(1.0+(float)sin16(sin16_arg)/32767.0));
      }
      set_half_strip(color, even_half);
      showStrip();
    }

    delay(TIME_INCREMENT);
    check_timer();
  }
  
  fade_out(MS_DELAY);
}

void set_half_strip(CRGB color, bool even) {
  int start_index = even ? 0 : 1;
  for (int i = start_index; i < NUM_LEDS; i=i+2) {
    leds[i] = color;
  }
}

void set_strip_odd_half(CRGB color) {
  for (int i = 1; i < NUM_LEDS; i=i+2) leds[i] = color;
}



// ***************************************************
//                        WIPE
// ***************************************************

void auto_wipe_forward() {
  wipe_forward(get_random_simple_color(), MS_DELAY);
  auto_fade_out();
}

void auto_wipe_forward_and_erase() {
  wipe_forward(get_random_simple_color(), MS_DELAY);
  wipe_forward(BLACK, MS_DELAY);
}
 
void auto_wipe_backward_and_erase() {
  wipe_backward(get_random_simple_color(), MS_DELAY);
  wipe_backward(BLACK, MS_DELAY);
}

void wipe_forward(const CRGB color, int ms_delay) {
  for(int i=0; i<NUM_LEDS; i++) {
      leds[i] = color;
      showStrip();
      delay(ms_delay);
  }
}

void wipe_backward(const CRGB color, int ms_delay) {
  int j = NUM_LEDS-1;
  for(int i=0; i<NUM_LEDS; i++) {
      leds[j--] = color;
      showStrip();
      delay(ms_delay);
  }
}

// ***************************************************
//                    THEATER CHASE
// ***************************************************

void auto_theater_chase_forward() {
  CRGB color = get_random_simple_color();
  set_timer();
  while(1) {
    theater_chase_forward (color, THEATER_CHASE_DELAY, THEATER_CHASE_COUNT, true);
    check_timer();
  }
  theater_chase_forward(color, THEATER_CHASE_DELAY, THEATER_CHASE_DELAY, false);
  auto_fade_out();
}

void auto_theater_chase_backward() {
  CRGB color = get_random_simple_color();
  set_timer();
  while(1) {
    theater_chase_backward(color, THEATER_CHASE_DELAY, THEATER_CHASE_DELAY, true);
    check_timer();
  }
  theater_chase_backward(color, THEATER_CHASE_DELAY, THEATER_CHASE_DELAY, false);
  auto_fade_out();
}

void theater_chase_forward(CRGB color, int ms_delay, int n, bool erase_last) {
  int q = 0;
  for (int j = 0; j < n; j++, q++) {  
    for (int i = 0; i < NUM_LEDS; i++) {
      if ((i+q) % THEATER_CHASE_INCR == 0) {
        set_pixel_color(i, color);  
      }
    }
    
    showStrip();
    delay(ms_delay);
    
    if (j == n-1 || !erase_last) break;
    
    fill_solid(leds, NUM_LEDS, BLACK);
  }
}

void theater_chase_backward(CRGB color, int ms_delay, int n, bool erase_last) {
  int q = n + THEATER_CHASE_INCR - 1;
  for (int j = 0; j < n; j++, q++) {  
    for (int i = 0; i < NUM_LEDS; i++) {
      if ((i+q) % THEATER_CHASE_INCR == 0) {
        set_pixel_color(i, color);
      }
    }
    showStrip();
    delay(ms_delay);
    
    if (j == n-1 || !erase_last) break;
    
    fill_solid(leds, NUM_LEDS, BLACK);
  }
}


// ***************************************************
//                FADE IN AND OUT
// ***************************************************

void auto_fade_in_out_color() {
  fade_in_out_color(get_random_simple_color(), 0);
}

void fade_in_out_color(const CRGB color, int ms_delay) {
  byte r, g, b;
  r = color.r;
  g = color.g;
  b = color.b;
  fade_in_out_components(r, g, b, ms_delay);
}

void fade_in_out_components(byte red, byte green, byte blue, int ms_delay) {
  float r, g, b, scale;
     
  for(int k = 0; k < MAX_INTENSITY; k=k+1) {
    scale = (float)k/MAX_INTENSITY;
    r = scale*red;
    g = scale*green;
    b = scale*blue;
    setAll(r,g,b);
    showStrip();
    delay(ms_delay);
  }
     
  for(int k = MAX_INTENSITY; k >= 0; k=k-2) {
    scale = (float)k/MAX_INTENSITY;
    r = scale*red;
    g = scale*green;
    b = scale*blue;
    setAll(r,g,b);
    showStrip();
    delay(ms_delay);
  }
}

// ***************************************************
//                    CYLON BOUNCE
// ***************************************************

void auto_cylon_bounce() {
  CRGB color;
  set_timer();
  while(1) {
    color = get_random_simple_color();
    CylonBounce(color.r, color.g, color.b, CYLON_EYE_SIZE, CYLON_SPEED_DELAY, CYLON_RETURN_DELAY);
    check_timer();
  }
}

void CylonBounce(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay){

  for(int i = 0; i < NUM_LEDS-EyeSize-2; i++) {
    setAll(0,0,0);
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
    showStrip();
    delay(SpeedDelay);
  }

  delay(ReturnDelay);

  for(int i = NUM_LEDS-EyeSize-2; i > 0; i--) {
    setAll(0,0,0);
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
    showStrip();
    delay(SpeedDelay);
  }
 
  delay(ReturnDelay);
}

// ***************************************************
//                        KITT
// ***************************************************

void auto_KITT_in_and_out() {
  set_timer();
  while(1) {
    CRGB color = get_random_simple_color();
    OutsideToCenter(color.r, color.g, color.b, KITT_EYE_SIZE, KITT_SPEED_DELAY, KITT_RETURN_DELAY);    
    CenterToOutside(color.r, color.g, color.b, KITT_EYE_SIZE, KITT_SPEED_DELAY, KITT_RETURN_DELAY);
    check_timer();
  }
}

void CenterToOutside(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) {
  for(int i =((NUM_LEDS-EyeSize)/2); i>=0; i--) {
    setAll(0,0,0);
   
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
   
    setPixel(NUM_LEDS-i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(NUM_LEDS-i-j, red, green, blue);
    }
    setPixel(NUM_LEDS-i-EyeSize-1, red/10, green/10, blue/10);
   
    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
}

// ***************************************************

void OutsideToCenter(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) {
  for(int i = 0; i<=((NUM_LEDS-EyeSize)/2); i++) {
    setAll(0,0,0);
   
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
   
    setPixel(NUM_LEDS-i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(NUM_LEDS-i-j, red, green, blue);
    }
    setPixel(NUM_LEDS-i-EyeSize-1, red/10, green/10, blue/10);
   
    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
}


// ***************************************************

void auto_twinkle() {
  CRGB color = get_random_simple_color();
  int lit_leds[TWINKLE_LEDS];
  int to_turn_on  = 0;
  int to_turn_off = 1;
  
  setAll(0, 0, 0);
  for (int i = 0; i < TWINKLE_LEDS; i++) lit_leds[i] = 0;
  
  set_timer();
  while(1) {
    lit_leds[to_turn_on] = get_random_pixel_index();
    set_pixel_color(lit_leds[to_turn_on], color);
    showStrip();
    
    set_pixel_color(lit_leds[to_turn_off], BLACK);
    to_turn_on  = (to_turn_on+1)  % TWINKLE_LEDS;
    to_turn_off = (to_turn_off+1) % TWINKLE_LEDS;

    delay(TWINKLE_DELAY);    
    check_timer();
  }
  auto_fade_out();
}

// ***************************************************

void TwinkleRandom(int Count, int SpeedDelay, boolean OnlyOne) {
  setAll(0,0,0);
 
  for (int i=0; i<Count; i++) {
     setPixel(random(NUM_LEDS),random(0,255),random(0,255),random(0,255));
     showStrip();
     delay(SpeedDelay);
     if(OnlyOne) {
       setAll(0,0,0);
     }
   }
 
  delay(SpeedDelay);
}

// ***************************************************

void SparkleLoop() {
  for (int i = 0; i < 1000; i++) Sparkle(0xff, 0xff, 0xff, 0);
}

void Sparkle(byte red, byte green, byte blue, int SpeedDelay) {
  int Pixel = random(NUM_LEDS);
  setPixel(Pixel,red,green,blue);
  showStrip();
  delay(SpeedDelay);
  setPixel(Pixel,0,0,0);
}

// ***************************************************

void auto_running_lights() {
  int position = 0;
  CRGB color = get_random_simple_color();

  set_timer();
  while(1) {
    position++; // = 0; //Position + Rate;
    for(int i = 0; i < NUM_LEDS; i++) {
      setPixel(i,((sin(i+position+1) * 127 + 128)/255)*color.r,
                 ((sin(i+position)   * 127 + 128)/255)*color.g,
                 ((sin(i+position-1) * 127 + 128)/255)*color.b);
    }
    showStrip();
    delay(RUNNING_LIGHTS_DELAY);
    check_timer();
  }
  auto_fade_out();
}


void auto_changing_colors() {
  CRGB color = get_random_simple_color();
  float phase[3] = {0.0};
  set_timer();
  while(1) {
    for(int i = 0; i < NUM_LEDS; i++) {
      setPixel(i,((sin(i+phase[0]) * 127 + 128)/255)*color.r,
                 ((sin(i+phase[1])   * 127 + 128)/255)*color.g,
                 ((sin(i+phase[2]) * 127 + 128)/255)*color.b);
    }
    showStrip();
    delay(RUNNING_LIGHTS_DELAY);
    phase[0] += 0.1;
    phase[1] += 0.2;
    phase[2] += 0.3;
    check_timer();
  }
  auto_fade_out();
}

// ***************************************************

void colorWipe(byte red, byte green, byte blue, int SpeedDelay) {
  for(uint16_t i=0; i<NUM_LEDS; i++) {
      setPixel(i, red, green, blue);
      showStrip();
      delay(SpeedDelay);
  }
}

// ***************************************************

void rainbowCycle(int SpeedDelay) {
  byte *c;
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< NUM_LEDS; i++) {
      c=Wheel(((i * 256 / NUM_LEDS) + j) & 255);
      setPixel(i, *c, *(c+1), *(c+2));
    }
    showStrip();
    delay(SpeedDelay);
  }
}

// ***************************************************

byte * Wheel(byte WheelPos) {
  static byte c[3];
 
  if(WheelPos < 85) {
   c[0]=WheelPos * 3;
   c[1]=255 - WheelPos * 3;
   c[2]=0;
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   c[0]=255 - WheelPos * 3;
   c[1]=0;
   c[2]=WheelPos * 3;
  } else {
   WheelPos -= 170;
   c[0]=0;
   c[1]=WheelPos * 3;
   c[2]=255 - WheelPos * 3;
  }

  return c;
}

// ***************************************************

void theaterChaseRainbow(int SpeedDelay) {
  byte *c;
 
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < NUM_LEDS; i=i+3) {
          c = Wheel( (i+j) % 255);
          setPixel(i+q, *c, *(c+1), *(c+2));    //turn every third pixel on
        }
        showStrip();
       
        delay(SpeedDelay);
       
        for (int i=0; i < NUM_LEDS; i=i+3) {
          setPixel(i+q, 0,0,0);        //turn every third pixel off
        }
    }
  }
}


// ***************************************************

void auto_fire() {
  set_timer();
  while(1) {
    Fire(55,120,15);
    check_timer();
  }
  auto_fade_out();
}

// ***************************************************

void Fire(int Cooling, int Sparking, int SpeedDelay) {
  static byte heat[NUM_LEDS_BUFF_SIZE];
  int cooldown;
 
  // Step 1.  Cool down every cell a little
  for( int i = 0; i < NUM_LEDS; i++) {
    cooldown = random(0, ((Cooling * 10) / NUM_LEDS) + 2);
   
    if(cooldown>heat[i]) {
      heat[i]=0;
    } else {
      heat[i]=heat[i]-cooldown;
    }
  }
 
  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
   
  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if( random(255) < Sparking ) {
    int y = random(7);
    heat[y] = heat[y] + random(0x20,0x40);
    //heat[y] = heat[y] + random(160,255);
    //heat[y] = random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for( int j = 0; j < NUM_LEDS; j++) {
    setPixelHeatColor(j, heat[j] );
  }

  showStrip();
  delay(SpeedDelay);
}

// ***************************************************

void setPixelHeatColor (int Pixel, byte temperature) {
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature/255.0)*191);
 
  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252
 
  // figure out which third of the spectrum we're in:
  if( t192 > 0x80) {                     // hottest
    setPixel(Pixel, 255, 255, heatramp);
  } else if( t192 > 0x40 ) {             // middle
    setPixel(Pixel, 255, heatramp, 0);
  } else {                               // coolest
    setPixel(Pixel, heatramp, 0, 0);
  }
}

// ***************************************************

void auto_meteor_rain() {
  set_timer();
  while(1) {
    meteorRain(MAX_INTENSITY, MAX_INTENSITY, MAX_INTENSITY, 5, 64, true, 30);
    check_timer();
  }
}

void meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay) {  
  setAll(0,0,0);
 
  for(int i = 0; i < NUM_LEDS+NUM_LEDS; i++) {
   
   
    // fade brightness all LEDs one step
    for(int j=0; j<NUM_LEDS; j++) {
      if( (!meteorRandomDecay) || (random(10)>5) ) {
        fadeToBlack(j, meteorTrailDecay );        
      }
    }
   
    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j <NUM_LEDS) && (i-j>=0) ) {
        setPixel(i-j, red, green, blue);
      }
    }
   
    showStrip();
    delay(SpeedDelay);
  }
}

// ***************************************************

void fadeToBlack(int ledNo, byte fadeValue) {
 #ifdef ADAFRUIT_NEOPIXEL_H
    // NeoPixel
    uint32_t oldColor;
    uint8_t r, g, b;
    int value;
   
    oldColor = strip.getPixelColor(ledNo);
    r = (oldColor & 0x00ff0000UL) >> 16;
    g = (oldColor & 0x0000ff00UL) >> 8;
    b = (oldColor & 0x000000ffUL);

    r=(r<=10)? 0 : (int) r-(r*fadeValue/256);
    g=(g<=10)? 0 : (int) g-(g*fadeValue/256);
    b=(b<=10)? 0 : (int) b-(b*fadeValue/256);
   
    strip.setPixelColor(ledNo, r,g,b);
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   leds[ledNo].fadeToBlackBy( fadeValue );
 #endif  
}

// ***************************************************
// ***************************************************
// ***************************************************

// Functions required for https://www.tweaking4all.com/

void showStrip() {
 #ifdef ADAFRUIT_NEOPIXEL_H
   // NeoPixel
   strip.show();
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   FastLED.show();
 #endif
}

void setPixel(int pixel_index, byte red, byte green, byte blue) {
  if (pixel_index >= NUM_LEDS || pixel_index < 0) return;
 #ifdef ADAFRUIT_NEOPIXEL_H
   // NeoPixel
   strip.setPixelColor(pixel_index, strip.Color(red, green, blue));
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   leds[pixel_index].r = red;
   leds[pixel_index].g = green;
   leds[pixel_index].b = blue;
 #endif
}

void set_pixel_color(int pixel_index, CRGB color) {
  if (pixel_index >= NUM_LEDS || pixel_index < 0) return;
  leds[pixel_index] = color;
}

void setAll(byte red, byte green, byte blue) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue);
  }
  showStrip();
}

/*
CRGB colors[NUM_COLORS] = {
  CRGB::AliceBlue,
  CRGB::Amethyst,
  CRGB::AntiqueWhite,
  CRGB::Aqua,
  CRGB::Aquamarine,
  CRGB::Azure,
  CRGB::Beige,
  CRGB::Bisque,
  CRGB::Black,
  CRGB::BlanchedAlmond,
  CRGB::Blue,
  CRGB::BlueViolet,
  CRGB::Brown,
  CRGB::BurlyWood,
  CRGB::CadetBlue,
  CRGB::Chartreuse,
  CRGB::Chocolate,
  CRGB::Coral,
  CRGB::CornflowerBlue,
  CRGB::Cornsilk,
  CRGB::Crimson,
  CRGB::Cyan,
  CRGB::DarkBlue,
  CRGB::DarkCyan,
  CRGB::DarkGoldenrod,
  CRGB::DarkGray,
  CRGB::DarkGrey,
  CRGB::DarkGreen,
  CRGB::DarkKhaki,
  CRGB::DarkMagenta,
  CRGB::DarkOliveGreen,
  CRGB::DarkOrange,
  CRGB::DarkOrchid,
  CRGB::DarkRed,
  CRGB::DarkSalmon,
  CRGB::DarkSeaGreen,
  CRGB::DarkSlateBlue,
  CRGB::DarkSlateGray,
  CRGB::DarkSlateGrey,
  CRGB::DarkTurquoise,
  CRGB::DarkViolet,
  CRGB::DeepPink,
  CRGB::DeepSkyBlue,
  CRGB::DimGray,
  CRGB::DimGrey,
  CRGB::DodgerBlue,
  CRGB::FairyLight,
  CRGB::FireBrick,
  CRGB::FloralWhite,
  CRGB::ForestGreen,
  CRGB::Fuchsia,
  CRGB::Gainsboro,
  CRGB::GhostWhite,
  CRGB::Gold,
  CRGB::Goldenrod,
  CRGB::Gray,
  CRGB::Grey,
  CRGB::Green,
  CRGB::GreenYellow,
  CRGB::Honeydew,
  CRGB::HotPink,
  CRGB::IndianRed,
  CRGB::Indigo,
  CRGB::Ivory,
  CRGB::Khaki,
  CRGB::Lavender,
  CRGB::LavenderBlush,
  CRGB::LawnGreen,
  CRGB::LemonChiffon,
  CRGB::LightBlue,
  CRGB::LightCoral,
  CRGB::LightCyan,
  CRGB::LightGoldenrodYellow,
  CRGB::LightGreen,
  CRGB::LightGrey,
  CRGB::LightPink,
  CRGB::LightSalmon,
  CRGB::LightSeaGreen,
  CRGB::LightSkyBlue,
  CRGB::LightSlateGray,
  CRGB::LightSlateGrey,
  CRGB::LightSteelBlue,
  CRGB::LightYellow,
  CRGB::Lime,
  CRGB::LimeGreen,
  CRGB::Linen,
  CRGB::Magenta,
  CRGB::Maroon,
  CRGB::MediumAquamarine,
  CRGB::MediumBlue,
  CRGB::MediumOrchid,
  CRGB::MediumPurple,
  CRGB::MediumSeaGreen,
  CRGB::MediumSlateBlue,
  CRGB::MediumSpringGreen,
  CRGB::MediumTurquoise,
  CRGB::MediumVioletRed,
  CRGB::MidnightBlue,
  CRGB::MintCream,
  CRGB::MistyRose,
  CRGB::Moccasin,
  CRGB::NavajoWhite,
  CRGB::Navy,
  CRGB::OldLace,
  CRGB::Olive,
  CRGB::OliveDrab,
  CRGB::Orange,
  CRGB::OrangeRed,
  CRGB::Orchid,
  CRGB::PaleGoldenrod,
  CRGB::PaleGreen,
  CRGB::PaleTurquoise,
  CRGB::PaleVioletRed,
  CRGB::PapayaWhip,
  CRGB::PeachPuff,
  CRGB::Peru,
  CRGB::Pink,
  CRGB::Plaid,
  CRGB::Plum,
  CRGB::PowderBlue,
  CRGB::Purple,
  CRGB::Red,
  CRGB::RosyBrown,
  CRGB::RoyalBlue,
  CRGB::SaddleBrown,
  CRGB::Salmon,
  CRGB::SandyBrown,
  CRGB::SeaGreen,
  CRGB::Seashell,
  CRGB::Sienna,
  CRGB::Silver,
  CRGB::SkyBlue,
  CRGB::SlateBlue,
  CRGB::SlateGray,
  CRGB::SlateGrey,
  CRGB::Snow,
  CRGB::SpringGreen,
  CRGB::SteelBlue,
  CRGB::Tan,
  CRGB::Teal,
  CRGB::Thistle,
  CRGB::Tomato,
  CRGB::Turquoise,
  CRGB::Violet,
  CRGB::Wheat,
  CRGB::White,
  CRGB::WhiteSmoke,
  CRGB::Yellow,
  CRGB::YellowGreen
};



    typedef enum {
        AliceBlue=0xF0F8FF,             ///< @htmlcolorblock{F0F8FF}
        Amethyst=0x9966CC,              ///< @htmlcolorblock{9966CC}
        AntiqueWhite=0xFAEBD7,          ///< @htmlcolorblock{FAEBD7}
        Aqua=0x00FFFF,                  ///< @htmlcolorblock{00FFFF}
        Aquamarine=0x7FFFD4,            ///< @htmlcolorblock{7FFFD4}
        Azure=0xF0FFFF,                 ///< @htmlcolorblock{F0FFFF}
        Beige=0xF5F5DC,                 ///< @htmlcolorblock{F5F5DC}
        Bisque=0xFFE4C4,                ///< @htmlcolorblock{FFE4C4}
        Black=0x000000,                 ///< @htmlcolorblock{000000} ..
        BlanchedAlmond=0xFFEBCD,        ///< @htmlcolorblock{FFEBCD}
        Blue=0x0000FF,                  ///< @htmlcolorblock{0000FF} ..
        BlueViolet=0x8A2BE2,            ///< @htmlcolorblock{8A2BE2}
        Brown=0xA52A2A,                 ///< @htmlcolorblock{A52A2A}
        BurlyWood=0xDEB887,             ///< @htmlcolorblock{DEB887}
        CadetBlue=0x5F9EA0,             ///< @htmlcolorblock{5F9EA0}
        Chartreuse=0x7FFF00,            ///< @htmlcolorblock{7FFF00}
        Chocolate=0xD2691E,             ///< @htmlcolorblock{D2691E}
        Coral=0xFF7F50,                 ///< @htmlcolorblock{FF7F50}
        CornflowerBlue=0x6495ED,        ///< @htmlcolorblock{6495ED}
        Cornsilk=0xFFF8DC,              ///< @htmlcolorblock{FFF8DC}
        Crimson=0xDC143C,               ///< @htmlcolorblock{DC143C}
        Cyan=0x00FFFF,                  ///< @htmlcolorblock{00FFFF} ..
        DarkBlue=0x00008B,              ///< @htmlcolorblock{00008B}
        DarkCyan=0x008B8B,              ///< @htmlcolorblock{008B8B}
        DarkGoldenrod=0xB8860B,         ///< @htmlcolorblock{B8860B}
        DarkGray=0xA9A9A9,              ///< @htmlcolorblock{A9A9A9}
        DarkGrey=0xA9A9A9,              ///< @htmlcolorblock{A9A9A9}
        DarkGreen=0x006400,             ///< @htmlcolorblock{006400}
        DarkKhaki=0xBDB76B,             ///< @htmlcolorblock{BDB76B}
        DarkMagenta=0x8B008B,           ///< @htmlcolorblock{8B008B}
        DarkOliveGreen=0x556B2F,        ///< @htmlcolorblock{556B2F}
        DarkOrange=0xFF8C00,            ///< @htmlcolorblock{FF8C00}
        DarkOrchid=0x9932CC,            ///< @htmlcolorblock{9932CC}
        DarkRed=0x8B0000,               ///< @htmlcolorblock{8B0000}
        DarkSalmon=0xE9967A,            ///< @htmlcolorblock{E9967A}
        DarkSeaGreen=0x8FBC8F,          ///< @htmlcolorblock{8FBC8F}
        DarkSlateBlue=0x483D8B,         ///< @htmlcolorblock{483D8B}
        DarkSlateGray=0x2F4F4F,         ///< @htmlcolorblock{2F4F4F}
        DarkSlateGrey=0x2F4F4F,         ///< @htmlcolorblock{2F4F4F}
        DarkTurquoise=0x00CED1,         ///< @htmlcolorblock{00CED1}
        DarkViolet=0x9400D3,            ///< @htmlcolorblock{9400D3}
        DeepPink=0xFF1493,              ///< @htmlcolorblock{FF1493}
        DeepSkyBlue=0x00BFFF,           ///< @htmlcolorblock{00BFFF}
        DimGray=0x696969,               ///< @htmlcolorblock{696969}
        DimGrey=0x696969,               ///< @htmlcolorblock{696969}
        DodgerBlue=0x1E90FF,            ///< @htmlcolorblock{1E90FF}
        FireBrick=0xB22222,             ///< @htmlcolorblock{B22222}
        FloralWhite=0xFFFAF0,           ///< @htmlcolorblock{FFFAF0}
        ForestGreen=0x228B22,           ///< @htmlcolorblock{228B22}
        Fuchsia=0xFF00FF,               ///< @htmlcolorblock{FF00FF} 
        Gainsboro=0xDCDCDC,             ///< @htmlcolorblock{DCDCDC}
        GhostWhite=0xF8F8FF,            ///< @htmlcolorblock{F8F8FF}
        Gold=0xFFD700,                  ///< @htmlcolorblock{FFD700}
        Goldenrod=0xDAA520,             ///< @htmlcolorblock{DAA520}
        Gray=0x808080,                  ///< @htmlcolorblock{808080}
        Grey=0x808080,                  ///< @htmlcolorblock{808080}
        Green=0x008000,                 ///< @htmlcolorblock{008000}
        GreenYellow=0xADFF2F,           ///< @htmlcolorblock{ADFF2F}
        Honeydew=0xF0FFF0,              ///< @htmlcolorblock{F0FFF0}
        HotPink=0xFF69B4,               ///< @htmlcolorblock{FF69B4}
        IndianRed=0xCD5C5C,             ///< @htmlcolorblock{CD5C5C}
        Indigo=0x4B0082,                ///< @htmlcolorblock{4B0082}
        Ivory=0xFFFFF0,                 ///< @htmlcolorblock{FFFFF0}
        Khaki=0xF0E68C,                 ///< @htmlcolorblock{F0E68C}
        Lavender=0xE6E6FA,              ///< @htmlcolorblock{E6E6FA}
        LavenderBlush=0xFFF0F5,         ///< @htmlcolorblock{FFF0F5}
        LawnGreen=0x7CFC00,             ///< @htmlcolorblock{7CFC00}
        LemonChiffon=0xFFFACD,          ///< @htmlcolorblock{FFFACD}
        LightBlue=0xADD8E6,             ///< @htmlcolorblock{ADD8E6}
        LightCoral=0xF08080,            ///< @htmlcolorblock{F08080}
        LightCyan=0xE0FFFF,             ///< @htmlcolorblock{E0FFFF}
        LightGoldenrodYellow=0xFAFAD2,  ///< @htmlcolorblock{FAFAD2}
        LightGreen=0x90EE90,            ///< @htmlcolorblock{90EE90}
        LightGrey=0xD3D3D3,             ///< @htmlcolorblock{D3D3D3}
        LightPink=0xFFB6C1,             ///< @htmlcolorblock{FFB6C1}
        LightSalmon=0xFFA07A,           ///< @htmlcolorblock{FFA07A}
        LightSeaGreen=0x20B2AA,         ///< @htmlcolorblock{20B2AA}
        LightSkyBlue=0x87CEFA,          ///< @htmlcolorblock{87CEFA}
        LightSlateGray=0x778899,        ///< @htmlcolorblock{778899}
        LightSlateGrey=0x778899,        ///< @htmlcolorblock{778899}
        LightSteelBlue=0xB0C4DE,        ///< @htmlcolorblock{B0C4DE}
        LightYellow=0xFFFFE0,           ///< @htmlcolorblock{FFFFE0}
        Lime=0x00FF00,                  ///< @htmlcolorblock{00FF00}
        LimeGreen=0x32CD32,             ///< @htmlcolorblock{32CD32}
        Linen=0xFAF0E6,                 ///< @htmlcolorblock{FAF0E6}
        Magenta=0xFF00FF,               ///< @htmlcolorblock{FF00FF} ..
        Maroon=0x800000,                ///< @htmlcolorblock{800000}
        MediumAquamarine=0x66CDAA,      ///< @htmlcolorblock{66CDAA}
        MediumBlue=0x0000CD,            ///< @htmlcolorblock{0000CD}
        MediumOrchid=0xBA55D3,          ///< @htmlcolorblock{BA55D3}
        MediumPurple=0x9370DB,          ///< @htmlcolorblock{9370DB}
        MediumSeaGreen=0x3CB371,        ///< @htmlcolorblock{3CB371}
        MediumSlateBlue=0x7B68EE,       ///< @htmlcolorblock{7B68EE}
        MediumSpringGreen=0x00FA9A,     ///< @htmlcolorblock{00FA9A}
        MediumTurquoise=0x48D1CC,       ///< @htmlcolorblock{48D1CC}
        MediumVioletRed=0xC71585,       ///< @htmlcolorblock{C71585}
        MidnightBlue=0x191970,          ///< @htmlcolorblock{191970}
        MintCream=0xF5FFFA,             ///< @htmlcolorblock{F5FFFA}
        MistyRose=0xFFE4E1,             ///< @htmlcolorblock{FFE4E1}
        Moccasin=0xFFE4B5,              ///< @htmlcolorblock{FFE4B5}
        NavajoWhite=0xFFDEAD,           ///< @htmlcolorblock{FFDEAD}
        Navy=0x000080,                  ///< @htmlcolorblock{000080}
        OldLace=0xFDF5E6,               ///< @htmlcolorblock{FDF5E6}
        Olive=0x808000,                 ///< @htmlcolorblock{808000}
        OliveDrab=0x6B8E23,             ///< @htmlcolorblock{6B8E23}
        Orange=0xFFA500,                ///< @htmlcolorblock{FFA500}
        OrangeRed=0xFF4500,             ///< @htmlcolorblock{FF4500}
        Orchid=0xDA70D6,                ///< @htmlcolorblock{DA70D6}
        PaleGoldenrod=0xEEE8AA,         ///< @htmlcolorblock{EEE8AA}
        PaleGreen=0x98FB98,             ///< @htmlcolorblock{98FB98}
        PaleTurquoise=0xAFEEEE,         ///< @htmlcolorblock{AFEEEE}
        PaleVioletRed=0xDB7093,         ///< @htmlcolorblock{DB7093}
        PapayaWhip=0xFFEFD5,            ///< @htmlcolorblock{FFEFD5}
        PeachPuff=0xFFDAB9,             ///< @htmlcolorblock{FFDAB9}
        Peru=0xCD853F,                  ///< @htmlcolorblock{CD853F}
        Pink=0xFFC0CB,                  ///< @htmlcolorblock{FFC0CB}
        Plaid=0xCC5533,                 ///< @htmlcolorblock{CC5533}
        Plum=0xDDA0DD,                  ///< @htmlcolorblock{DDA0DD}
        PowderBlue=0xB0E0E6,            ///< @htmlcolorblock{B0E0E6}
        Purple=0x800080,                ///< @htmlcolorblock{800080}
        Red=0xFF0000,                   ///< @htmlcolorblock{FF0000} ..
        RosyBrown=0xBC8F8F,             ///< @htmlcolorblock{BC8F8F}
        RoyalBlue=0x4169E1,             ///< @htmlcolorblock{4169E1}
        SaddleBrown=0x8B4513,           ///< @htmlcolorblock{8B4513}
        Salmon=0xFA8072,                ///< @htmlcolorblock{FA8072}
        SandyBrown=0xF4A460,            ///< @htmlcolorblock{F4A460}
        SeaGreen=0x2E8B57,              ///< @htmlcolorblock{2E8B57}
        Seashell=0xFFF5EE,              ///< @htmlcolorblock{FFF5EE}
        Sienna=0xA0522D,                ///< @htmlcolorblock{A0522D}
        Silver=0xC0C0C0,                ///< @htmlcolorblock{C0C0C0}
        SkyBlue=0x87CEEB,               ///< @htmlcolorblock{87CEEB}
        SlateBlue=0x6A5ACD,             ///< @htmlcolorblock{6A5ACD}
        SlateGray=0x708090,             ///< @htmlcolorblock{708090}
        SlateGrey=0x708090,             ///< @htmlcolorblock{708090}
        Snow=0xFFFAFA,                  ///< @htmlcolorblock{FFFAFA}
        SpringGreen=0x00FF7F,           ///< @htmlcolorblock{00FF7F}
        SteelBlue=0x4682B4,             ///< @htmlcolorblock{4682B4}
        Tan=0xD2B48C,                   ///< @htmlcolorblock{D2B48C}
        Teal=0x008080,                  ///< @htmlcolorblock{008080}
        Thistle=0xD8BFD8,               ///< @htmlcolorblock{D8BFD8}
        Tomato=0xFF6347,                ///< @htmlcolorblock{FF6347}
        Turquoise=0x40E0D0,             ///< @htmlcolorblock{40E0D0}
        Violet=0xEE82EE,                ///< @htmlcolorblock{EE82EE}
        Wheat=0xF5DEB3,                 ///< @htmlcolorblock{F5DEB3}
        White=0xFFFFFF,                 ///< @htmlcolorblock{FFFFFF} ..
        WhiteSmoke=0xF5F5F5,            ///< @htmlcolorblock{F5F5F5}
        Yellow=0xFFFF00,                ///< @htmlcolorblock{FFFF00} ..
        YellowGreen=0x9ACD32,           ///< @htmlcolorblock{9ACD32}

        // LED RGB color that roughly approximates
        // the color of incandescent fairy lights,
        // assuming that you're using FastLED
        // color correction on your LEDs (recommended).
        FairyLight=0xFFE42D,           ///< @htmlcolorblock{FFE42D}

        // If you are using no color correction, use this
        FairyLightNCC=0xFF9D2A         ///< @htmlcolorblock{FFE42D}

    } HTMLColorCode;
};

*/


/*
void loop() {
//  theaterChase(0xff, 0, 0, 50);
//  theaterChase(0, 0xff, 0, 50);
//  theaterChase(0, 0, 0xff, 50);
//  CylonBounce(0xff, 0, 0, 4, 10, 50);
//  CylonBounce(0, 0xff, 0, 4, 10, 50);
//  CylonBounce(0, 0, 0xff, 4, 10, 50);
//  FadeInOut(0xff, 0x00, 0x00); // red
//  FadeInOut(0xff, 0xff, 0xff); // white
//  FadeInOut(0x00, 0x00, 0xff); // blue
//  NewKITT(0xff, 0, 0, 8, 10, 50);
//  NewKITT(0, 0xff, 0, 8, 10, 50);
//  NewKITT(0, 0, 0xff, 8, 10, 50);
//  Twinkle(0xff, 0, 0, NUM_LEDS/2, 100, false);
//  Twinkle(0, 0xff, 0, NUM_LEDS/2, 100, false);
//  Twinkle(0, 0, 0xff, NUM_LEDS/2, 100, false);
//  TwinkleRandom(40, 100, false);
//  SparkleLoop();
//  RunningLights(0xff,0,0, 50);        // red
//  RunningLights(0xff,0xff,0xff, 50);  // white
//  RunningLights(0,0,0xff, 50);        // blue
//  colorWipe(0x00,0xff,0x00, 50);
//  colorWipe(0x00,0x00,0x00, 50);
//  rainbowCycle(20);
//  theaterChaseRainbow(50);
//  rainbowCycle(20);
//  FireLoop();
//  meteorRain(0xff,0xff,0xff,5, 64, true, 30);


// Slow changing

  // FadeInOut(0xff, 0x00, 0x00); // red
  // FadeInOut(0xff, 0xff, 0xff); // white
  // FadeInOut(0x00, 0x00, 0xff); // blue
  // Twinkle(0xff, 0, 0, NUM_LEDS/2, 100, false);
  // Twinkle(0, 0xff, 0, NUM_LEDS/2, 100, false);
  // Twinkle(0, 0, 0xff, NUM_LEDS/2, 100, false);
  // TwinkleRandom(40, 100, false);
  // colorWipe(0x00,0xff,0x00, 50);
  // colorWipe(0x00,0x00,0x00, 50);
  // rainbowCycle(20);
  // rainbowCycle(20);
  // meteorRain(0xff,0xff,0xff,5, 64, true, 30);

  // wipeLeft(getRandomColor(), 50);
  // wipeRight(getRandomColor(), 50);
  // wipeFromMiddle(getRandomColor(), 50);
  // wipeFromBorders(getRandomColor(), 50);
  // wipeRight(CRGB::Black, 50);
  // FadeInOut2(getRandomColor());
  
  // fill_solid(leds, NUM_LEDS, CRGB::White);
  // for (int i = 0; i < NUM_LEDS; i++) {
  //   byte c = i * 32 / 60;
  //   setPixel(i, c, c, c);
  // }

  // for (int i = 1; i < NUM_COLORS; i++) {
  //   FadeInOut2(getRandomColor(), 50);
  // }


//  for (int i = 1; i < NUM_COLORS; i++) {
//    fill_solid(leds, NUM_LEDS, getRandomColor());
//    FadeIn(50);
//    FadeOut(50);
//  }

//  fill_solid(leds, NUM_LEDS, CRGB::Black);
//  showStrip();

 
//  for (int i = 0; i < NUM_LEDS; i++) {
//    fill_solid(leds, NUM_LEDS, CRGB::Black);
//    leds[i] = getRandomColor();
//    showStrip();
//    delay(50);
//  }

  // for (int i = 0; i < NUM_SIMPLE_COLORS; i++) {
  //   theater_chase(simple_colors[i], 50, 10);
  // }

  // for (int i = 0; i < NUM_SIMPLE_COLORS; i++) {
  //   fade_in_out_color(simple_colors[i], 0);
  // }

  // for (int i = 0; i < NUM_SIMPLE_COLORS; i++) {
  //   chirp(simple_colors[i]);
  // }

  // for (int i = 0; i < NUM_SIMPLE_COLORS; i++) {
  //   CRGB c = get_random_visible_color();
  //    theater_chase_forward (c, 50, 20);
  //    theater_chase_backward(c, 50, 20);
  // }

//  for (int i = 0; i < NUM_FUCTIONS; i++) {
//    display_functions[i]();
//  }
  RGBLoop();
}
*/

// ***************************************************
// Unused functions
// ***************************************************

/*

// ***************************************************

void auto_chirp() {
  chirp(get_random_simple_color());
}

void auto_chirp2() {
  chirp(get_random_simple_color());
}

void chirp(CRGB color) {
  int ms_delay = 1024;

  fill_solid(leds, NUM_LEDS, BLACK);
  showStrip();
  delay(ms_delay);

  for (int i = 0; i < 100; i ++) {
    fill_solid(leds, NUM_LEDS, color);
    showStrip();
    delay(ms_delay);
    fill_solid(leds, NUM_LEDS, BLACK);
    showStrip();
    delay(ms_delay);
    ms_delay -= ms_delay * 0.10;
    if (ms_delay <= 1) break;
  }
}

void chirp2(CRGB color) {
  int ms_delay = 1024;

  fill_solid(leds, NUM_LEDS, BLACK);
  showStrip();
  delay(ms_delay);

  for (int i = 0; i < 100; i ++) {
    fill_solid(leds, NUM_LEDS, color);
    showStrip();
    if (ms_delay <= 1 || i == 99) break;
    delay(ms_delay);
    fill_solid(leds, NUM_LEDS, BLACK);
    showStrip();
    delay(ms_delay);
    ms_delay -= ms_delay * 0.10;
  }
}

// ***************************************************

void auto_wipe_out_and_erase() {
  wipe_from_middle(get_random_simple_color(), MS_DELAY);
  wipe_from_middle(BLACK, MS_DELAY);
}

void auto_wipe_in_and_erase() {
  wipe_from_borders(get_random_simple_color(), MS_DELAY);
  wipe_from_borders(BLACK, MS_DELAY);
}

void wipe_from_middle(const CRGB color, int ms_delay) {
  int j = (NUM_LEDS-1)/2;
  int k =  NUM_LEDS/2;
  for(int i=0; i<(NUM_LEDS+1)/2; i++) {
      leds[j--] = color;
      leds[k++] = color;
      showStrip();
      delay(ms_delay);
  }
}

void wipe_from_borders(const CRGB color, int ms_delay) {
  int j = NUM_LEDS-1;
  for(int i=0; i<NUM_LEDS/2+1; i++, j--) {
      leds[i] = color;
      leds[j] = color;
      showStrip();
      delay(ms_delay);
  }
}

// ***************************************************

void RGBLoop(){
  for(int j = 0; j < 3; j++ ) {
    // Fade IN
    for(int k = 0; k < 256>>SCALE_DOWN_SHIFT; k++) {
      switch(j) {
        case 0: setAll(k,0,0); break;
        case 1: setAll(0,k,0); break;
        case 2: setAll(0,0,k); break;
      }
      showStrip();
      delay(3*(2<<SCALE_DOWN_SHIFT)); 
    }
    // Fade OUT
    for(int k = 256>>SCALE_DOWN_SHIFT-1; k >= 0; k--) {
      switch(j) {
        case 0: setAll(k,0,0); break;
        case 1: setAll(0,k,0); break;
        case 2: setAll(0,0,k); break;
      }
      showStrip();
      delay(3*(2<<SCALE_DOWN_SHIFT));
    }
  }
}

// ***************************************************

void auto_NewKITT() {
  CRGB color = get_random_simple_color();
  set_timer();
  while(1) {
    NewKITT(color.r, color.g, color.b, KITT_EYE_SIZE, KITT_SPEED_DELAY, KITT_RETURN_DELAY);
    check_timer();
  }
}

void auto_KITT_forth_and_back() {
  set_timer();
  while(1) {
    CRGB color = get_random_simple_color();
    LeftToRight(color.r, color.g, color.b, KITT_EYE_SIZE, KITT_SPEED_DELAY, KITT_RETURN_DELAY);    
    RightToLeft(color.r, color.g, color.b, KITT_EYE_SIZE, KITT_SPEED_DELAY, KITT_RETURN_DELAY);
    check_timer();
  }
}

void NewKITT(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay){
  RightToLeft(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  LeftToRight(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  OutsideToCenter(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  CenterToOutside(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  LeftToRight(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  RightToLeft(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  OutsideToCenter(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  CenterToOutside(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
}

void LeftToRight(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) {
  for(int i = 0; i < NUM_LEDS-EyeSize-2; i++) {
    setAll(0,0,0);
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
}

void RightToLeft(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) {
  for(int i = NUM_LEDS-EyeSize-2; i > 0; i--) {
    setAll(0,0,0);
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
}

// ***************************************************


void auto_theater_chase_forth_and_back() {
  theater_chase_forth_and_back(get_random_simple_color(), THEATER_CHASE_DELAY);
  auto_fade_out();
}

void theater_chase_forth_and_back(CRGB color, int ms_delay) {
  set_timer();
  while(1) {
    theater_chase_forward (color, ms_delay, THEATER_CHASE_COUNT);
    theater_chase_backward_no_erase(color, ms_delay, THEATER_CHASE_COUNT);
    check_timer();
  }
}

void theater_chase_back_and_forth(CRGB color, int ms_delay, int num_cycles) {
  set_timer();
  while(1) {
    theater_chase_backward(color, ms_delay, THEATER_CHASE_COUNT);
    theater_chase_forward_no_erase (color, ms_delay, THEATER_CHASE_COUNT);
    check_timer();
  }
}


*/
