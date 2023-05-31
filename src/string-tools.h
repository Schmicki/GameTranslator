#pragma once

/*************************************************************************************************/

int IsHex(char c);

int IsDigit(char c);

int IsAlnum(char c);

int IsSpace(char c);

int GetHex(char c);

char GetHexChar(char c);

/*
* The destination must be 11+ bytes long.
* Converts integer to string representation.
* Returns string length.
*/
int IntegerToString(int value, char* dst);

/*
* The destination must be 9+ bytes long.
* Converts integer to hexadecimal string representation.
* Returns string length.
*/
int IntegerToStringHex(int value, char* dst);

/*
* Converts string to integer.
*/
int StringToInteger(const char* string);

/*
* Converts hexadecimal string to integer.
*/
int StringToIntegerHex(const char* string);

/*
* *					Wildcard
* \n				new line
* \r				carriage return
* \t				horizontal tab
* \000 - \255		decimal byte value
* \x00 - \xFF		hexadecimal byte value
*/
const char* StringMatch(const char* text, int text_length, const char* pattern, int* match_length);

void StringReplaceChar(char* string, char find, char replace);