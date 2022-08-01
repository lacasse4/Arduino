int i = 0;
unsigned long time = 0; 
bool flag = HIGH; 

void setup() { 
  // put your setup code here, to run once: 
  Serial.begin(9600); 
  pinMode(10, OUTPUT); //direction control PIN 10 with direction wire 
  pinMode(11, OUTPUT); //PWM PIN 11 with PWM wire 

  /* 
   *  Pin 10 (yellow cable) down not seem to work for backward operation.
   *  But it has to be set to 1 to work anyway.
   */
  digitalWrite(10, 1);

  /* 
   *  Couldn't make sense out of Pin 9 output from motor 
   */
  
} 

void loop() { 

  // put your main code here, to run repeatedly: 
/*
  if (millis() - time > 5000) { 
    flag = !flag; 
    digitalWrite(10, flag); 
    time = millis(); 
  }
*/ 

//  flag = !flag; 
//  digitalWrite(10, flag); 
//  digitalWrite(10, 1); 
//  analogWrite(11, 100);
//  delay(2000);
//
//  digitalWrite(10, 0); 
//  analogWrite(11, 50);
//  delay(2000);


/*
 * increase motor speed from 0 to approx max. in 2 sec increments.
 */
  char s[100];
  Serial.println("---------------------------");
  for (i = 0; i < 16; i++) {
    int speed = i*16;
    int command = 255 - i*16;
    analogWrite(11, command);
    sprintf(s, "speed = %3d, command = %3d", speed, command);
    Serial.println(s);
    delay(2000);
  }


/*  
  if (Serial.available()) { 
    analogWrite(11, Serial.parseInt()); //input speed (must be int) 
    delay(200); 
  }
 */
/*  
  for(int j = 0;j<8;j++) { 
    i += pulseIn(9, HIGH, 500000); // SIGNAL OUTPUT PIN 9 with white line,
                                   // cycle = 2*i, 1s = 1000000us，Signal cycle pulse number：27*2 
  } 
  i = i >> 3; 
  Serial.print(111111 / i); //speed r/min (60*1000000/(45*6*2*i)) 
  Serial.println(" r/min"); 
  i = 0; 
*/
}
