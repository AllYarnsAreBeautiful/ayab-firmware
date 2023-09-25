### Finite State Machine

The finite state machine is defined in the `Controller` class.

| State  | Action |
     --: | :--
 `Idle`  | Wait for information on machine type.
 `Init`  | Wait for carriage to be put in the correct location.
 `Ready` | Wait to start operation.
 `Knit`  | Operate in knitting mode.
 `Test`  | Test the hardware.
 `Error` | Report a failed operation.

A tabular representation of state transitions follows.

| Transition      | Function / condition |
              --: | :--
 `Idle  -> Init`  | `Com::h_reqInit()`
 `Init  -> Ready` | `OpKnit::isReady()`
 `Ready -> Knit`  | `OpKnit::startKnitting()`
 `Ready -> Test`  | `OpTest::startTest()`
 `Knit  -> Ready` | `m_workedOnLine && m_lastLineFlag`
 `Test  -> Init`  | `OpTest::end()`
