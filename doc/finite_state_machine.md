# Finite State Machine

The knitting machine finite state machine is defined in the Knitter class.

A graphical representation follows.

\startuml
skinparam shadowing false
skinparam ArrowColor DimGray
skinparam state {
  backgroundColor LightSteelBlue
  BorderColor Gray
}
hide empty description

Init : Wait for carriage to be put in the correct location.
Ready: Wait to start operation.
Operate: Knit.
Test: Calculate but don't exercise solenoids.

Reset --> Init
Init --> Test : startTest()
Ready --> Test : startTest()
Init -> Ready : Direction == Right\n&& HallActive == Left
Ready -> Operate : startOperation()
Operate -> Ready : lastLine()\n&& finished
\enduml
