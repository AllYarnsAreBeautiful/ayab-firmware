//debug.h

#ifndef DEBUG_H
#define DEBUG_H


#define DEBUG

/*
 *
 */
#ifdef DEBUG
	#define ERROR(module, out) \
		Serial.print(#module); \
		Serial.print(":ERROR:"); \
		Serial.println(#out);
	#define WARNING(module, out) \
		Serial.print(#module); \
		Serial.print(":WARNING:"); \
		Serial.println(#out);
	#define INFO(module, out) \
		Serial.print(#module); \
		Serial.print(":INFO:"); \
		Serial.println(#out);
#else
	#define ERROR(module, out)
	#define WARNING(module, out )
	#define INFO(module, out)
#endif


#endif