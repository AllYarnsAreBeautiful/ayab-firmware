//debug.h

#ifndef DEBUG_H
#define DEBUG_H


#define DEBUG

/*
 *
 */
#define NL Serial.print("\n\r")
#ifdef DEBUG
	#define ERROR(module, out) \
		Serial.print(#module); \
		Serial.print(":ERROR:"); \
		Serial.print(#out); \
		NL
	#define WARNING(module, out) \
		Serial.print(#module); \
		Serial.print(":WARNING:"); \
		Serial.print(#out); \
		NL
	#define INFO(module, out) \
		Serial.print(#module); \
		Serial.print(":INFO:"); \
		Serial.print(#out); \
		NL
#else
	#define ERROR(module, out)
	#define WARNING(module, out )
	#define INFO(module, out)
#endif


#endif