# WC64
## Yet another wordclock project

![Image of wordclock](https://github.com/chess-levin/WC64/blob/master/docs/img/wc64_front.jpg)

This is my version of a wordclock. I was trying out the arduino platform and this made so much fun, so I decided to build my own wordclock. The dimensions of my wordclock are approx. 11x11x3 cm. You can clip of the front panel and turn it by 90 degress 
in order to change the posing variant: You can hang the clock on the wall or set it on a shelf. The clock detects
the environment brightness and adapts the brightness of the display to it.

Techically it is based upon an 8x8 LED Matrix (WS2812B LEDs e.g. www.diamex.de), a real time clock (RTC DS3231 AT24C32 Memory Real Time Clock IIC Modul für Arduino PIC CP14003 B36) and an arduino nano board. In order to set time and date there are two push buttons inside the case. The case is made of wood. The display is a multilayer stack of acralic glass, transparancy film, LED matrix and hard foam board. There are some more images, a video and a circuit diagram in the docs folder.

![Image of wordclock](https://github.com/chess-levin/WC64/blob/master/docs/img/wc64_parts.jpg)

The challenge was to find a way to put the mask with the letters on the acrylic. For a transparency made by a cutting plotter the letters are to schmall. So I decided to have the mask printed on the transperency. The result was that the black parts were to translucent. To achieve a high contrast between the dark parts and the lighten letters I glued two transparencies together. One of the two transparencies has a self-adhesive side which this is glued to the acrylic.

I found several ideas browsing the net for other wordclock projects. One was the [formatc1702](https://github.com/formatc1702) project of Daniel Rojas. He gave me the idea how to put all words into an 8x8 matrix. Thank you guys - keep on  wordclocking!

Have fun!

Any comments are welcome.
