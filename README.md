# Oscilloscope
Authors: 	Heric Flores-Huerta & Alexander Dymek
Date:	    12/18/2014
ECE Class:Real-Time Embedded Systems

Project Description:

  Using the C programming Language we built a functional oscilloscope on an Texas Instruments (TI) Embedded System              running on a 50 MHz ARM Cortex M3 CPU. The board used was a TI LM3S8962 along side an LM3S2110 connected through                a CAN (Controller Area Network).
  Functions of the oscilloscope include waveform display, adjustable trigger slope and voltage scale, and an FFT                spectrum analyzer. Frequency calculation was provided via the CAN from the LM3S2110 controller.
  The project started as self contained program before being ported over to Texas Instruments' Real-Time Operating              System, SYS/BIOS.
