void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.flush();
  delay(1000);

  char s[100];
  printUnsigned("F_CPU           = ", F_CPU);
  printUnsigned("SystemCoreClock = ", SystemCoreClock);
  printSigned(  "SysTick_IRQn    = ", SysTick_IRQn);

  printUnsigned("GetTickCount() = ", GetTickCount());
  printUnsigned("GetTickCount() = ", GetTickCount());
  printUnsigned("GetTickCount() = ", GetTickCount());
  printUnsigned("GetTickCount() = ", GetTickCount());
  
  printBinary(  "SysTick->CTRL  = ", SysTick->CTRL, 32);
  printUnsigned("SysTick->LOAD  = ", SysTick->LOAD);
  printUnsigned("SysTick->VAL   = ", SysTick->VAL);
  printHexa(    "SysTick->CALIB = ", SysTick->CALIB);

  printBinary(  "PMC_PCSR0      = ", PMC->PMC_PCSR0, 32);
  printBinary(  "PMC_PCSR1      = ", PMC->PMC_PCSR1, 32);
  printBinary(  "PMC_WPSR       = ", PMC->PMC_WPSR, 32);
    

//  printBinary("ISER0 = ", ISER0, 32);
//  printBinary("ISER1 = ", ISER1, 32);
//  printBinary("TCCR0A = ", TCCR0A);
//  printBinary("TCCR0B = ", TCCR0B);
//  printBinary("TCNT0 =  ",  TCNT0);
//  printBinary("OCR0A =  ",  OCR0A);
//  printBinary("OCR0B =  ",  OCR0B);
//  printBinary("TIMSK0 = ", TIMSK0);
//  printBinary("TIFR0 =  ",  TIFR0);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void printBinary(char *label, unsigned int data, int num_bits) {
  char t[100];
  char s[4][9] = {0};
  char c;
  int char_counter = 7;
  int byte_counter = 3;
  
  if (num_bits != 32 && num_bits != 24 && num_bits != 16 && num_bits != 8) return;
  
  for (int i = 0; i < num_bits; i++) {
    
    c = data & 1 ? '1' : '0';
    s[byte_counter][char_counter] = c;
    data >>= 1;
    
    char_counter--;
    if (char_counter < 0) {
      char_counter = 7;
      byte_counter--;
    }
  }
  
  strcpy(t, label);
  for (int i = 0; i < 4; i++) {
    strcat(t, s[i]);
    strcat(t, " ");
  }
  Serial.println(t);
}

void printSigned(char *label, int data) {
  char t[100];

  sprintf(t, "%s%ld", label, data);
  Serial.println(t);
}

void printUnsigned(char *label, unsigned int data) {
  char t[100];

  sprintf(t, "%s%lu", label, data);
  Serial.println(t);
}

void printHexa(char *label, unsigned int data) {
  char t[100];

  sprintf(t, "%s%lx", label, data);
  Serial.println(t);
}
