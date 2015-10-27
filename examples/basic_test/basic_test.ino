
#include <EEPROM.h>
#include <EEWrap.h>

//Use the xxx_e types rather than the standard types like uint8_t
struct Foo{
  uint8_e a;
  int16_e b;
  float_e c;
};

Foo foo EEMEM; //EEMEM tells the compiler that the object resides in the EEPROM

void setup(){

  //Write to values stored in EEPROM
  foo.a = 45;
  foo.b = 12345;
  foo.c = 3.1415;

  //Print values from EEPROM
  Serial.begin(9600);
  Serial.println(foo.a);
  Serial.println(foo.b);
  Serial.println(foo.c);
}

void loop() {}
