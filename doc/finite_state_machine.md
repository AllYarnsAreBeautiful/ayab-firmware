### Finite State Machine

The finite state machine is defined in the `Fsm` class.

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
 `Wait  -> Test`  | `Test::startTest()`
 `Init  -> Test`  | `Test::startTest()`
 `Ready -> Test`  | `Test::startTest()`
 `Test  -> Init`  | `Test::quitCmd()`
 `Wait  -> Init`  | `Knit::initMachine()`
 `Init  -> Ready` | `Knit::isReady()`
 `Ready -> Knit`  | `Knit::startKnitting()`
 `Knit  -> Ready` | `m_workedOnLine && m_lastLineFlag`
