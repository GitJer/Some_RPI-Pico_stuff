# Two independently running state machines, synchronized via irq, one is temporarily disabled

This is an example of two state machines (sm) synchronizing their execution via setting and clearing an irq. At some point sm1 gets disabled. Because they are synchronized, sm0 also stops.

One sm repeatedly counts up from 0 to 0xFFFFFFFF (actually, it counts down!), the other repeatedly counts down from 31. They both send their current values to the c-program via their Rx FIFOs.

```
0 = 198         1 = 24          ;
0 = 199         1 = 23          ;
0 = 200         1 = 22          ; 
0 = 201         1 = 21  <---    ; The sm1 is disabled
0 = 202         1 = 20  <---    ; The sm0 stops producing output (irq wait)
0 = 199         1 = 19  <---    ;
0 = 199         1 = 18  <---    ; Last valid sm1 Rx FIFO data
0 = 199         1 = 21  <---    ;
0 = 199         1 = 21  <---    ;
0 = 199         1 = 21  <---    ;
0 = 199         1 = 21  <---    ;
0 = 199         1 = 21  <---    ;
0 = 199         1 = 21  <---    ;
0 = 203         1 = 17          ; The sm1 started again, sm0 follows
0 = 204         1 = 16          ;
0 = 205         1 = 15          ;
```