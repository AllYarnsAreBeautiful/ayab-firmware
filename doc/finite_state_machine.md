### Finite State Machine

The finite state machine is defined in the `Fsm` class.

|State||
--:|:--
`Init` | Wait for carriage to be put in the correct location.
`Ready` | Wait to start operation.
`Knit` | Knitting mode.
`Test` | Hardware testing mode.
|||

A tabular representation of state transitions follows.

|Transition||
--:|:--
`Init  -> Test`  |`Tester::startTest()`  
`Ready -> Test`  |`Tester::startTest()`  
`Test -> Init` | `Tester::quitCmd()`
`Init  -> Ready` | `Knitter::isReady()`
`Ready -> Knit` | `Knitter::startKnitting()`
`Knit  -> Ready` | `m_workedOnLine && m_lastLineFlag`

