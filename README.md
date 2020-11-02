# IRSignalDecoder
This code uses a TI-Launchpad connected to an IR receiver to decode an IR signal sent to it from a remote.
Does not use any libraries. The signal is decoded live using Interrupts on Rising and Falling Edges to measure the time of each bit, which is then used to perform some action.

This was a homework for an Embedded Systems course at Carnegie Mellon University, and posted on Github with permission from the Professor.
