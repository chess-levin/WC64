# WC64
### Yet another wordclock project

![Image of wordclock](https://github.com/chess-levin/WC64/blob/master/docs/img/wc64_front.jpg)

This is my version of a wordclock. I was trying out the arduino platform and this made so much fun, so I decided to build my own wordclock. The dimensions of my wordclock are approx. 11x11x3 cm. The front panel is hold by four strong neodym magnets. You can unclip it and turn it by 90 degress. This allows you to change the posing variant: You can hang the clock on the wall or set it on a shelf. The clock detects the environment brightness and adapts the brightness of the display to it. Find two videos of the clock on youtube [1](https://www.youtube.com/watch?v=mQ0f72IIGkc), [2](https://www.youtube.com/watch?v=TP77dVIzgcs).

### Electronics

Techically it is based upon an [8x8 LED Matrix (WS2812B LEDs)](https://github.com/chess-levin/WC64/blob/master/docs/img/wc64_matrix.jpg), a real time clock (RTC DS3231 AT24C32 Memory Real Time Clock IIC Modul für Arduino PIC CP14003 B36) and an arduino nano board. In order to set time and date there are two push buttons inside the case.  The display is a multilayer stack of acralic glass, transparancy film, LED matrix and hard foam board. The power supply is a 2100mA USB power adapter, that connects to the arduino via mini USB cable.

### Structure

![Image of wordclock](https://github.com/chess-levin/WC64/blob/master/docs/img/wc64_parts.jpg)

The first challenge was to find a way to get [the mask](https://github.com/chess-levin/WC64/blob/master/docs/mask_final_dt.pdf) with the letters on the acrylic. For a transparency made by a cutting plotter the letters are to schmall. So I decided to have the mask printed on a transperency. The result was that the black parts were to translucent. To achieve a high contrast between the dark parts and the lighten letters I glued two transparencies together. One of the two transparencies has a self-adhesive side which it is glued to the acrylic.

The second challenge was to build a thin carrier board for the 68 LEDs and the light sensor. After some trial and error rountrips with wood I found the hard foam board. The hard foam is light, stable and most important it doesn't splinter when drilling the holes for the LEDs. The carrier board is fixed to the display with doubled sided tape.

The case is made of wood.

There are some more images, a pdf of the mask and a [circuit diagram](https://github.com/chess-levin/WC64/blob/master/docs/img/WC64_circuit.png) in the docs folder.

### Thanks

I found several ideas browsing the net for other wordclock projects. One was the [formatc1702](https://github.com/formatc1702) project of Daniel Rojas. He gave me the idea how to put all words into an 8x8 matrix. Thank you guys - keep on  wordclocking!

Have fun!

Any comments are welcome.
