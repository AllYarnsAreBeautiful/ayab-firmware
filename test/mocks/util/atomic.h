/* suggested by @jpcornil-git based on https://arduino.stackexchange.com/questions/77494/which-arduinos-support-atomic-block */

#include <avr/io.h>
#include <avr/interrupt.h>

#define ATOMIC_BLOCK(type) for(type; type##_OBJECT_NAME.run(); type##_OBJECT_NAME.stop())
#define ATOMIC_RESTORESTATE_OBJECT_NAME atomicBlockRestoreState_
#define ATOMIC_RESTORESTATE AtomicBlockRestoreState ATOMIC_RESTORESTATE_OBJECT_NAME

class AtomicBlockRestoreState
{
public:
    // Constructor: called when the object is created
    inline AtomicBlockRestoreState()
    {
        sreg_save = SREG; // save status register
        cli(); // turn interrupts OFF
    }

    // Destructor: called when the object is destroyed (ex: goes out-of-scope)
    inline ~AtomicBlockRestoreState()
    {
        SREG = sreg_save; // restore status register
    }

    // Can we run? Returns true to run the `for` loop or
    // `false` to stop it.
    inline bool run()
    {
        return run_now;
    }

    // Tell the `for` loop to stop
    inline void stop()
    {
        run_now = false;
    }

private:
    bool run_now = true;
    uint8_t sreg_save;
};
