/*
 * RoboBrrdComm.c
 * For the RoboBrrd robot!
 *
 * May 2011
 * RobotGrrl.com 
 */


/*
 Basic Pin setup:
 ------------                                  ---u----
 ARDUINO   13|-> SCLK (pin 25)           OUT1 |1     28| OUT channel 0
 12|                           OUT2 |2     27|-> GND (VPRG)
 11|-> SIN (pin 26)            OUT3 |3     26|-> SIN (pin 11)
 10|-> BLANK (pin 23)          OUT4 |4     25|-> SCLK (pin 13)
 9|-> XLAT (pin 24)             .  |5     24|-> XLAT (pin 9)
 8|                             .  |6     23|-> BLANK (pin 10)
 7|                             .  |7     22|-> GND
 6|                             .  |8     21|-> VCC (+5V)
 5|                             .  |9     20|-> 2K Resistor -> GND
 4|                             .  |10    19|-> +5V (DCPRG)
 3|-> GSCLK (pin 18)            .  |11    18|-> GSCLK (pin 3)
 2|                             .  |12    17|-> SOUT
 1|                             .  |13    16|-> XERR
 0|                           OUT14|14    15| OUT channel 15
 ------------                                  --------
 
 -  Put the longer leg (anode) of the LEDs in the +5V and the shorter leg
 (cathode) in OUT(0-15).
 -  +5V from Arduino -> TLC pin 21 and 19     (VCC and DCPRG)
 -  GND from Arduino -> TLC pin 22 and 27     (GND and VPRG)
 -  digital 3        -> TLC pin 18            (GSCLK)
 -  digital 9        -> TLC pin 24            (XLAT)
 -  digital 10       -> TLC pin 23            (BLANK)
 -  digital 11       -> TLC pin 26            (SIN)
 -  digital 13       -> TLC pin 25            (SCLK)
 -  The 2K resistor between TLC pin 20 and GND will let ~20mA through each
 LED.  To be precise, it's I = 39.06 / R (in ohms).  This doesn't depend
 on the LED driving voltage.
 - (Optional): put a pull-up resistor (~10k) between +5V and BLANK so that
 all the LEDs will turn off when the Arduino is reset.
 
 If you are daisy-chaining more than one TLC, connect the SOUT of the first
 TLC to the SIN of the next.  All the other pins should just be connected
 together:
 BLANK on Arduino -> BLANK of TLC1 -> BLANK of TLC2 -> ...
 XLAT on Arduino  -> XLAT of TLC1  -> XLAT of TLC2  -> ...
 The one exception is that each TLC needs it's own resistor between pin 20
 and GND.
 
 This library uses the PWM output ability of digital pins 3, 9, 10, and 11.
 Do not use analogWrite(...) on these pins.
 
 This sketch does the Knight Rider strobe across a line of LEDs.
 
 Alex Leone <acleone ~AT~ gmail.com>, 2009-02-03 */

#include <Time.h>
#include <NewSoftSerial.h>
#include <Streaming.h>
//#include "Tlc5940.h"

boolean debug = false;

// All the pins
int interruptOutgoing = 12;
int interruptIncoming = 0;
int ROBOBRRDrx = 7;
int ROBOBRRDtx = 8;
int LED = 5;
int STATUS = 4;
int robobrrdNSSAttempts = 0;
int triggerAttemptsCount = 0;

// NewSoftSerial
NewSoftSerial nssROBOBRRD(ROBOBRRDrx, ROBOBRRDtx);

// Trigger flag
volatile boolean triggerFlag = false;

// Outstanding communication flag
boolean outstandingComm = false;

char msg[141];

// LED Pins
int redL(1), greenL(2), blueL(3);
int redR(4), greenR(5), blueR(6);

// LED Values
int preLR(0), preLG(0), preLB(0), preRR(0), preRG(0), preRB(0);
int LR, LG, LB, RR, RG, RB;

// Speaker
int spkr(6);
int length = 5; // the number of notes
char notes[] = "d d  "; // a space represents a rest
int tempo = 300;
char music[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C', 'D', 'E', 'F' };


// Initialize
void setup() {
	
	// Communication
	Serial.begin(9600);
	nssROBOBRRD.begin(9600);
	
	// Interrupts
	pinMode(interruptOutgoing, OUTPUT);
    digitalWrite(interruptOutgoing, LOW);
    
	attachInterrupt(interruptIncoming, trigger, RISING);
    digitalWrite(2, LOW);
	
	/* Call Tlc.init() to setup the tlc.
     You can optionally pass an initial PWM value (0 - 4095) for all channels.*/
    pinMode(spkr, OUTPUT);
    //Tlc.init(0);
    
    // Outputs
	pinMode(LED, OUTPUT);
	pinMode(STATUS, OUTPUT);
    
}

void loop() {
	
    // * * * * * * * * * * * * * * *
    // Check for messages from Xbee
    // * * * * * * * * * * * * * * *
    
	if(nextXB() == 'E') {
		
		digitalWrite(STATUS, LOW);
        delay(1000);
		
		if(debug) Serial << "Byte is E" << endl;
        
		// Send out the interrupt
		digitalWrite(interruptOutgoing, HIGH);
		outstandingComm = true;
		delay(500);
		digitalWrite(interruptOutgoing, LOW);
		
		while(!triggerFlag) {
            
            digitalWrite(LED, !digitalRead(LED));
            delay(50);
            
			// Waiting for trigger to send the data
			if(debug) Serial << "Waiting for the trigger" << endl;
			
			if(triggerAttemptsCount >= 100) {
				triggerAttemptsCount = 0;
				break;
			}
			
			triggerAttemptsCount++;
			
		}
		
		if(triggerFlag) {
            
            digitalWrite(LED, LOW);
            delay(50);
			
			// Sending the message now
			nssROBOBRRD.print("E");
			
			if(debug) Serial << "Sending the message now" << endl;
			
			outstandingComm = false;
			
			digitalWrite(LED, HIGH);
			delay(1000);
			digitalWrite(LED, LOW);
			
			triggerFlag = false;
			
		} else {
            digitalWrite(LED, LOW);
        }
		 
         
	}
	
}

void trigger() {
	triggerFlag = true;
}

byte nextXB() {
	
	if(debug) Serial << "Waiting for a byte from XBee" << endl;
    
	while(1) {
		
		if(Serial.available() > 0) {
			byte b = Serial.read();
			if(debug) Serial << b << endl;
			return b;
		}
		
		if(triggerFlag) {
			
			digitalWrite(STATUS, LOW);
			
			int letter = 0;
			
			// Empty the message buffer
			for(int i=0; i<141; i++) {
				msg[i] = ' ';
			}
			
			// We are ready to receive the message
			digitalWrite(interruptOutgoing, HIGH);
			delay(50);
			digitalWrite(interruptOutgoing, LOW);
			
			digitalWrite(LED, HIGH);
			
			// Reading the message
			byte b = nextROBOBRRD();
			while(b != '*') {
				msg[letter] = b;
				letter++;
				b = nextROBOBRRD();
			}
			
			// Parse the message
			if(msg[0] == 'E') {
				
                // * * * * * * * * * * * * * * * * * * * *
                // Do something when received from RoboBrrd
                // * * * * * * * * * * * * * * * * * * * *
                
				// Do something
				if(debug) Serial << "Received message OK" << endl;
				
                digitalWrite(LED, HIGH);
				delay(1000);
				digitalWrite(LED, LOW);
				delay(1000);
				digitalWrite(LED, HIGH);
				delay(1000);
				
			}
            
            // --- P
            if(msg[0] == 'P') {
                Serial << "P";
            }
            
            // --- L
            if(msg[0] == 'L') {
                Serial << "L";
            }
            
            // --- R
            if(msg[0] == 'R') {
                Serial << "R";
            }
			
			digitalWrite(LED, LOW);
			triggerFlag = false;
			
		} else {
		
		/*
		 For some reaon this doesn't work on the 3.3V Arduino Pro Mini 168
		if(millis()%50 == 0) {
			digitalWrite(STATUS, !digitalRead(STATUS));
		}
		 */
		
		if(millis()%50 == 0) {
			digitalWrite(STATUS, !digitalRead(STATUS));
			delay(1);
         }
            
            
        }
        
		
        // * * * * * * * * * * * * * * 
        // Do something in the meantime
        // * * * * * * * * * * * * * *

        /*
        //if(millis()%500 == 0) {

         LR = int(random(0, 256));
         LG = int(random(0, 256));
         LB = int(random(0, 256));
         RR = int(random(0, 256));
         RG = int(random(0, 256));
         RB = int(random(0, 256));
         
         fade ( preLR, preLG, preLB,
         LR, LG, LB,
         preLR, preLG, preLB,
         LR, LG, LB,
         1 );
         
         //Serial << "From: " << preLR << "," << preLG << "," << preLB << "!" << endl;
         //Serial << "To: " << LR << "," << LG << "," << LB << "!" << endl;
         
         preLR = LR;
         preLG = LG;
         preLB = LB;
         preRR = RR;
         preRG = RG;
         preRB = RB;
            
            delay(500);
            
        //}
         */
        
	}
}

byte nextROBOBRRD() {
	
	if(debug) Serial << "Waiting for a byte from MANOI" << endl;
	
	while(1) {
		
		if(nssROBOBRRD.available()) {
			byte b = nssROBOBRRD.read();
			if(debug) Serial << b << endl;
			return b;
		}
	
        if(millis()%50 == 0) {
			digitalWrite(LED, !digitalRead(LED));
			delay(1);
        }
        
		if(robobrrdNSSAttempts >= 100) {
			robobrrdNSSAttempts = 0;
			break;
		}
		
		robobrrdNSSAttempts++;
		
	}
	
}

/*
void fade ( int startL_R,  int startL_G,  int startL_B, 
           int finishL_R, int finishL_G, int finishL_B,
           int startR_R,  int startR_G,  int startR_B,
           int finishR_R, int finishR_G, int finishR_B,
           int stepTime ) {
    
    int skipEveryL_R = 256/abs(startL_R-finishL_R); 
    int skipEveryL_G = 256/abs(startL_G-finishL_G);
    int skipEveryL_B = 256/abs(startL_B-finishL_B); 
    int skipEveryR_R = 256/abs(startR_R-finishR_R); 
    int skipEveryR_G = 256/abs(startR_G-finishR_G);
    int skipEveryR_B = 256/abs(startR_B-finishR_B); 
    
    
    
    for(int i=0; i<256; i++) {
        
        // Left
        
        if(startL_R<finishL_R) {
            if(i<=finishL_R) {
                if(i%skipEveryL_R == 0) {
                    Tlc.set(redL, 16*i);
                } 
            }
        } else if(startL_R>finishL_R) {
            if(i>=(256-startL_R)) {
                if(i%skipEveryL_R == 0) {
                    Tlc.set(redL, 16*(256-i)); 
                }
            } 
        }
        
        if(startL_G<finishL_G) {
            if(i<=finishL_G) {
                if(i%skipEveryL_G == 0) {
                    Tlc.set(greenL, 16*i);
                } 
            }
        } else if(startL_G>finishL_G) {
            if(i>=(256-startL_G)) {
                if(i%skipEveryL_G == 0) {
                    Tlc.set(greenL, 16*(256-i)); 
                }
            } 
        }
        
        if(startL_B<finishL_B) {
            if(i<=finishL_B) {
                if(i%skipEveryL_B == 0) {
                    Tlc.set(blueL, 16*i);
                } 
            }
        } else if(startL_B>finishL_B) {
            if(i>=(256-startL_B)) {
                if(i%skipEveryL_B == 0) {
                    Tlc.set(blueL, 16*(256-i)); 
                }
            } 
        }
        
        // Right
        
        if(startR_R<finishR_R) {
            if(i<=finishR_R) {
                if(i%skipEveryR_R == 0) {
                    Tlc.set(redR, 16*i);
                } 
            }
        } else if(startR_R>finishR_R) {
            if(i>=(256-startR_R)) {
                if(i%skipEveryR_R == 0) {
                    Tlc.set(redR, 16*(256-i)); 
                }
            }
        }
        
        if(startR_G<finishR_G) {
            if(i<=finishR_G) {
                if(i%skipEveryR_G == 0) {
                    Tlc.set(greenR, 16*i);
                } 
            }
        } else if(startR_G>finishR_G) {
            if(i>=(256-startR_G)) {
                if(i%skipEveryR_G == 0) {
                    Tlc.set(greenR, 16*(256-i)); 
                }
            } 
        }
        
        if(startR_B<finishR_B) {
            if(i<=finishR_B) {
                if(i%skipEveryR_B == 0) {
                    Tlc.set(blueR, 16*i);
                } 
            }
        } else if(startR_B>finishR_B) {
            if(i>=(256-startR_B)) {
                if(i%skipEveryR_B == 0) {
                    Tlc.set(blueR, 16*(256-i)); 
                }
            } 
        }
        
        Tlc.update();
        delay(stepTime);
        
    }
    
}
 */

void playTone(int tone, int duration) {
	
	for (long i = 0; i < duration * 1000L; i += tone * 2) {
		digitalWrite(spkr, HIGH);
		delayMicroseconds(tone);
		digitalWrite(spkr, LOW);
		delayMicroseconds(tone);
	}
	
}