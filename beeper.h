// beeper.h

#ifndef BEEPER_H
#define BEEPER_H

#include "Arduino.h"
#include "settings.h"
#include "debug.h"


class Beeper{
public:
	Beeper();

	void start();
	void endline();
	void endWork();

private:
	void beep(byte length);
};

#endif