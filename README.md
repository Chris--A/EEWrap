# `EEWrap` library for AVR and Arduino boards.

This library makes the EEPROM simple. In fact you do not even need to think about the EEPROM.

## Setup:

All you have to do is include the EEPROM functionality and the `EEWrap` library.

```C++
#include <avr/eeprom.h>
#include <EEWrap.h>
```

## Basic Usage.

The library wraps ordinary types and provides a layer between your code and the EEPROM.
What it means is, you do not have to directly touch the EEPROM at all.

The library provides some predefined types for all the basic standard C++ types.

C/C++ type | EEWrap type
------------ | -------------
`char` | `int8_e`
`unsigned char` | `uint8_e`
`int` | `int16_e`
`unsigned int` | `uint16_e`
`long` | `int32_e`
`unsigned long` | `uint32_e`
`long long` | `int64_e`
`unsigned long long` | `uint64_e`
`bool` | `bool_e`
`float` | `float_e`
`float` | `float_e`

You can also wrap any custom types by using the EEWrap class directly to create your own types.
For example, `int16_e` is equivalent to `EEWrap< int >`.

You can declare an instance of these types in the standard way, you just add the EEPROM specifier `EEMEM` to the end of it.
This specifier causes the compiler to layout space for the variable. Multiple variables will be spaced after each other (their address).

If you want to locate your variables manually you simply declare a pointer and set it to the desired location. When you want to use the data, just dereference the pointer.

And that is basically it, you use the variable as normal: When you write to it, you are writing to the EEPROM, when you read the variable, you are reading the EEPROM.

Simple huh?

## Examples

```C++

//Declare an unsigned long as an eeprom variable.
uint32_e timeRunning EEMEM;

//A function that is called periodically.
void updateTime(){

  timeRunning += millis();
}
```

Or the equivalent, with a custom EEPROM location.
```C++

//Declare an unsigned long as an eeprom variable.
uint32_e *timeRunning = (uint32_e*) 14; //Locate the variable at byte 15.

void updateTime(){
  *timeRunning += millis();
}
```

You can declare arrays as usual.
```C++

uint8_e str[ 6 ];

str[0] = 'H';
str[1] = 'e';
str[2] = 'l';
str[2] = 'l';
str[4] = 'o';
str[5] = 0;
```



