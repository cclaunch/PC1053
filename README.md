# PC1053
Emulates an IBM 1053 (print-only I/O Selectric) as used on IBM 1130 and other mainframes of the 1960s

The 1053 attaches to a mainframe with three SMS paddle cards - two carry signals and the third delivers power. This emulator is built around an Arduino Mega 2560 R3, a relay module and some simple circuitry, hooked to three SMS paddle cards. It provides a serial port connection over a USB cable for communicating with a terminal emulator program on a personal computer to display the output that is being 'typed' by the mainframe. 

This emulates the console typeball that is installed on an IBM 1130 system, but can be easily modified to support a different (or additional) typeballs. The typeball has characters that are not in standard ASCII, e.g. ¢ and ¬ which can be displayed in UTF8.

Using a terminal emulator that supports UTF8 encoding and ANSI colors provides the most realistic output, since the text will display in either black or red on white background - the text color is selectable by ribbon shift commands from the mainframe to the 1053. The terminal emulator provides a few commands that can be issued to the PC1053 - mainly to set the left and right margins, but also to set, clear and display tab stops that will be in effect. 

PC1053 operates at approximately the same timing as a 1053, which supported a maximum of 15.5 characters per second and with the high speed carrier return feature would move from right to left edge in just a bit over 1 second. This provides realistic timing on the feedback signals that tell the mainframe when the Selectric mechanism is busy in a print cycle or performing an operation such as shifting case or moving to a new line. 

The device is constructed in a case with a three digit display to show the current column that the typeball/carrier has reached. It has buttons on the front to match the controls on a real 1053 - buttons to Tab, Space and Return, plus tab Set and Clear. This project has a Fritzing diagram of the connections and other circuits, while the physical layout of the case is left as an exercise for the reader. 
