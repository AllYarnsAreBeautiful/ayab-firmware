//debug.h

#ifndef DEBUG_H
#define DEBUG_H


//#define DEBUG

/*
 *
 */
#ifdef DEBUG
	#define DEBUG_PRINT(str) \
	    Serial.print("#"); \
	    Serial.print(millis()); \
	    Serial.print(": "); \
	    Serial.print(__FUNCTION__); \
	    Serial.print("() in "); \
	    Serial.print(__FILE__); \
	    Serial.print(':'); \ 
	    Serial.print(__LINE__); \
	    Serial.print(' '); \
	    Serial.println(str);
#else
	#define DEBUG_PRINT(str)
#endif

#endif