# Two independently running state machines 

This is just an example of two state machines (sm) running independently. Nothing special about it, but I had to do it.

One sm repeatedly counts up from 0 to 0xFFFFFFFF (actually, it counts down!), the other repeatedly counts down from 31. They both send their current values to the c-program via their Rx FIFOs.