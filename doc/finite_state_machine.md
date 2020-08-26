### Finite State Machine

The finite state machine is defined in the `Fsm` class.

| State  | Action |
     --: | :--
 `Init`  | Wait for carriage to be put in the correct location.
 `Ready` | Wait to start operation.
 `Knit`  | Operate in knitting mode.
 `Test`  | Test the hardware.
 `Error` | Report a failed operation.

A tabular representation of state transitions follows.

| Transition      | Function / condition |
              --: | :--
 `Init  -> Test`  | `Tester::startTest()`  
 `Ready -> Test`  | `Tester::startTest()`  
 `Test  -> Init`  | `Tester::quitCmd()`
 `Init  -> Ready` | `Knitter::isReady()`
 `Ready -> Knit`  | `Knitter::startKnitting()`
 `Knit  -> Ready` | `m_workedOnLine && m_lastLineFlag`
