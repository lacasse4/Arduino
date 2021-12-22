/*****************************************************
This is the demo sketch for the command line interface
by FreakLabs. It's a simple command line interface
where you can define your own commands and pass arguments
to them. 
*****************************************************/
#include <Cmd.h>

int led_pin = 13;
bool led_blink_enb = false;
int led_blink_delay_time = 1000;

void setup()
{  
  // init the command line and set it for a speed of 57600
  Serial.begin(9600);
  cmdInit(&Serial);
  cmdAdd("b", b);
  cmdAdd("a", a);
}

void loop()
{
  cmdPoll();
}

void b(int arg_cnt, char **args)
{
  Serial.println("in B");
}

void a(int arg_cnt, char **args)
{
  Serial.println("in A");
}
