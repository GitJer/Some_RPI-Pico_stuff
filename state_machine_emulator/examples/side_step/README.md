# The sidestep also works with two GPIO as output:

## pio program:
```
.program tester
 
.side_set 2 opt

.wrap_target
    nop side 0b01
    nop
    nop side 0b10
.wrap
```
## c program:
```
0, sideset_base, 4
```

