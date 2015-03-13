# Oscilloscope
Authors: 	Heric Flores-Huerta & Alexander Dymek
Date:	    12/18/2014
ECE Class:Real-Time Embedded Systems

Project Description:

Using the C programming Language we built a functional oscilloscope on an Texas Instruments (TI) Embedded System running on a 50 MHz ARM Cortex M3 CPU. The board used was a TI LM3S8962 along side an LM3S2110 connected through a CAN (Controller Area Network).
Functions of the oscilloscope include waveform display, adjustable trigger slope and voltage scale, and an FFT spectrum analyzer. Frequency calculation was provided via the CAN from the LM3S2110 controller.
The project started as self contained program before being ported over to Texas Instruments' Real-Time Operating System, SYS/BIOS.

Files:

lab0/workspace/lab0:  This is an simple project to introduce board functionality

lab1/workspace:       Implementation of oscilloscope written in C

lab0/workspace/lab2:  This project takes lab1 and ports it over to an RTOS (SYS/BIOS)

lab0/workspace/lab3:  Continuation of the above project and adds a CAN interface to the LM3S2110

lab0/workspace/lab3_2:Program files for LM3S2110 from above project

