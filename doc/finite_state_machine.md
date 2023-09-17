### Finite State Machine

The finite state machine is defined in the `Op` class.

| State  | Action |
     --: | :--
 `Wait`  | Wait for information on machine type.
 `Init`  | Wait for carriage to be put in the correct location.
 `Ready` | Wait to start operation.
 `Knit`  | Operate in knitting mode.
 `Test`  | Test the hardware.
 `Error` | Report a failed operation.

A tabular representation of state transitions follows.

| Transition      | Function / condition |
              --: | :--
 `Wait  -> Test`  | `Tester::startTest()`
 `Init  -> Test`  | `Tester::startTest()`
 `Ready -> Test`  | `Tester::startTest()`
 `Test  -> Init`  | `Tester::quitCmd()`
 `Wait  -> Init`  | `Knitter::initMachine()`
 `Init  -> Ready` | `Knitter::isReady()`
 `Ready -> Knit`  | `Knitter::startKnitting()`
 `Knit  -> Ready` | `m_workedOnLine && m_lastLineFlag`
