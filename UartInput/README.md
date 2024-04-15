# UartInput #

Test of UART input

Send `.A` + LF to set all DMX channels to 255
Send `.X` + LF to set all DMX channels to 0

Responds on both USB virtual serial port, and a serial port connected to the TX/RX pads. Uses DMX Port B for output. 9600 baud rate 8N1
For hardware rev 1.2 the pinout is:
4 - TX
5 - RX
