// ayab_hw_test.ino


/*
 * INCLUDES
 */
#include "Arduino.h"
#include "SerialCommand.h"


/*
 * DEFINES / SETTINGS
 */
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

/*
 *	DECLARATIONS
 */ 
SerialCommand SCmd;   // The SerialCommand object


/*
 * SETUP
 */
void setup() {
	Serial.begin(9600);

	// Setup callbacks for SerialCommand commands
	SCmd.addCommand("HELLO",SayHello);     // Echos the string argument back
	SCmd.addDefaultHandler(unrecognized);  // Handler for command that isn't matched  (says "What?") 
	INFO("setup", "ready"); 
}


/*
 *	MAIN LOOP
 */ 
void loop() {
	SCmd.readSerial(); 
}


/*
 *
 */
void SayHello()
{
  char *arg;  
  arg = SCmd.next();    // Get the next argument from the SerialCommand object buffer
  if (arg != NULL)      // As long as it existed, take it
  {
    Serial.print("Hello "); 
    Serial.println(arg); 
  } 
  else {
    Serial.println("Hello, whoever you are"); 
  }
}

/*
 * This gets set as the default handler, and gets called when no other command matches. 
 */
void unrecognized()
{
  INFO("unrecognized", "What?"); 
}