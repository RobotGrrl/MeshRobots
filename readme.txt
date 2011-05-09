This is the code for all of my robots on the mesh network! It is basically just MANOI and RoboBrrd so far, because YodaBot is in need of repairs. You can see more details about the robot mesh network at robotgrrl.com.

Each robot has two Arduinos, the main board and the communication board. The main board is for important aspects of the robot, such as the motors or sensors. The communication board is for secondary aspects of the robot, such as LEDs, and external communication (XBee). The two boards share a serial connection and a pair of interrupts for handshakes. It's a good idea to have switches on the pins that use the usb serial connection (so that you can just flip the switch so then you are able to download the code without having to unplug everything).

Diagram:

---------------              ---------------
| Main Board  |              | Comm Board  |
|      (RX) 0 | <<<<~~<~~<~~ | 8 (NSS TX)  |
|      (TX) 1 | ~~>~~>~~>>>> | 7 (NSS RX)  |
|  (Int RX) 2 | <<<<~~<~~<~~ | 12 (Int TX) |
| (Int TX) 12 | ~~>~~>~~>>>> | 2 (Int RX)  |
---------------              | ----------- |
                             | 0 (XB TX)   |
                             | 1 (XB RX)   |
                             ---------------

Eventually I'll make a template cookie cutter version that you all can easily adapt to your robots.

Enjoy! :)