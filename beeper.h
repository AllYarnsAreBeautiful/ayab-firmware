// beeper.h

#ifndef BEEPER_H
#define BEEPER_H

#include "Arduino.h"
#include "settings.h"
#include "debug.h"


class Beeper{
public:
	Beeper();

	void ready();
	void finishedLine();
	void endWork();

private:
	void beep(byte length);
};

#endif