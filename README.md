# IRSignalDecoder
This code uses a TI-Launchpad connected to an IR receiver to decode an IR signal sent to it from a remote.
Does not use any libraries. The signal is decoded live using Interrupts on Rising and Falling Edges to measure the time of each bit, which is then used to perform some action.


This was a homework for an Embedded Systems course at Carnegie Mellon University, and posted on Github with permission from the Professor.
Do not use for your own homework.


**Following setup settings were used with Code Composer:**

Target: Tiva C Series - Tiva TM4C123GH6PM

Connection: Stellaris In-Circuit Debug Interface

Compiler Version: TI v16.9.6.LTS  


**Included Options:**

C:\ti\TivaWare_C_Series-2.1.4.178\inc

C:\ti\TivaWare_C_Series-2.1.4.178\driverlib  




**File Search Paths:**

C:\ti\TivaWare_C_Series-2.1.4.178\driverlib\ccs\Debug\driverlib.lib  
