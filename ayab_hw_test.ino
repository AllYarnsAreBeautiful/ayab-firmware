// ayab_hw_test.ino


/*
 * INCLUDES
 */
#include "Arduino.h"
#include "SerialCommand.h"

#include "Solenoids.h"
#include "debug.h"
#include "settings.h"


/*
 * DEFINES
 */

/*
 *	DECLARATIONS
 */ 
SerialCommand SCmd;   // The SerialCommand object
Solenoids solenoids;
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