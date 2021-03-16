# Two independently running state machines, one gets disabled temporarily

This is an example of two state machines (sm) running independently, but one, sm1, gets disabled temporarily.

The sm0 repeatedly counts up from 0 to 0xFFFFFFFF (actually, it counts down!), the other repeatedly counts down from 31. They both send their current values to the c-program via their Rx FIFOs.

In the c-program, sm1 gets disabled for a number of iterations. The output is somewhat interesting:
```
0 = 293088      1 = 31          ; 
0 = 293089      1 = 30          ; 
0 = 293090      1 = 29          ; 
0 = 293091      1 = 28  <---    ; Here the sm1 is disabled, but it still has 4 words in the Rx FIFO
0 = 293092      1 = 27  <---    ; 
0 = 293093      1 = 26  <---    ; 
0 = 293094      1 = 25  <---    ; Last valid Rx FIFO data
0 = 293095      1 = 28  <---    ; Here the Rx FIFO of sm1 is empty. Oddly, we get 28, not 25 
0 = 293096      1 = 28  <---    ; 
0 = 293097      1 = 28  <---    ; 
0 = 293098      1 = 28  <---    ; 
0 = 293099      1 = 28  <---    ; 
0 = 293100      1 = 28  <---    ; 
0 = 293101      1 = 24          ; The sm is started again, and as expected continues where it was stopped
0 = 293102      1 = 23          ; 
0 = 293103      1 = 22          ; 
0 = 293104      1 = 21          ; 
```
While sm0 keeps counting up, sm1 is disabled. At that moment there are still 4 data items in the FIFO. After these 4 have been read and printed, reading the FIFO keeps giving data (28, the entry at the time of disabling sm1). I know you should always check whether there is still valid data in the FIFO before reading it, but hey now we know what happens.
