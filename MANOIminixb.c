/*
 *  MANOIminixb.c
 *  
 *
 *  Created by Erin Kennedy on 10-11-24.
 *  Copyright 2010 robotgrrl.com. All rights reserved.
 *
 */

#include <NewSoftSerial.h>
#include <Streaming.h>

boolean debug = false;

// All the pins
int interruptOutgoing = 7;
int interruptIncoming = 2;
int MANOIrx = 5;
int MANOItx = 4;
int LED = 3;
int STATUS = 6;
int manoiNSSAttempts = 0;
int triggerAttemptsCount = 0;

// NewSoftSerial
NewSoftSerial nssMANOI(MANOIrx, MANOItx);

// Trigger flag
volatile boolean triggerFlag = false;

// Outstanding communication flag
boolean outstandingComm = false;

char msg[141];

// Initialize
void setup() {
    
	if(debug) Serial << "MANOI Comm. beginning initialization" << endl;
	
	// Communication
	Serial.begin(9600);
	nssMANOI.begin(9600);
	
	// Outputs
	pinMode(LED, OUTPUT);
	pinMode(STATUS, OUTPUT);
	
	// Interrupts
	pinMode(interruptOutgoing, OUTPUT);
	attachInterrupt(0, trigger, RISING);
	
	if(debug) Serial << "Done init" << endl;
	
}

void loop() {
	
    byte msgXB = nextXB();
    
    // --- E
	if(msgXB == 'E') {
		
		digitalWrite(STATUS, LOW);
		
		if(debug) Serial << "Byte is E" << endl;
		
		// Send out the interrupt
		digitalWrite(interruptOutgoing, HIGH);
		outstandingComm = true;
		delay(50);
		digitalWrite(interruptOutgoing, LOW);
		
		while(!triggerFlag) {
			
			// Waiting for trigger to send the data
			if(debug) Serial << "Waiting for the trigger" << endl;
			digitalWrite(LED, HIGH);
			delay(50);
			digitalWrite(LED, LOW);
			delay(50);
			
			if(triggerAttemptsCount >= 100) {
				triggerAttemptsCount = 0;
				break;
			}
			
			triggerAttemptsCount++;
			
		}
		
		if(triggerFlag) {
			
			// Sending the message now
			nssMANOI.print("E");
			
			if(debug) Serial << "Sending the message now" << endl;
			
			outstandingComm = false;
			
			digitalWrite(LED, HIGH);
			delay(1000);
			digitalWrite(LED, LOW);
			
			triggerFlag = false;
			
		}
		
	}
    
    if(msgXB == 'P') {
        relayMessage('P');
    }
    
    if(msgXB == 'L') {
        relayMessage('L');
    }
    
    if(msgXB == 'R') {
        relayMessage('R');
    }
    
    /*
    byte c = 'P';
	// --- P
    if(msgXB == 'P' || msgXB == 'L' || msgXB == 'R') {
        relayMessage(c);
	}
     */
     
    
}

void relayMessage(byte c) {
    
    digitalWrite(STATUS, LOW);
    
    if(debug) Serial << "Byte is P" << endl;
    
    // Send out the interrupt
    digitalWrite(interruptOutgoing, HIGH);
    outstandingComm = true;
    delay(50);
    digitalWrite(interruptOutgoing, LOW);
    
    while(!triggerFlag) {
        
        // Waiting for trigger to send the data
        if(debug) Serial << "Waiting for the trigger" << endl;
        digitalWrite(LED, HIGH);
        delay(50);
        digitalWrite(LED, LOW);
        delay(50);
        
        if(triggerAttemptsCount >= 100) {
            triggerAttemptsCount = 0;
            break;
        }
        
        triggerAttemptsCount++;
        
    }
    
    if(triggerFlag) {
        
        // Sending the message now
        nssMANOI.print(c);
        
        if(debug) Serial << "Sending the message now" << endl;
        
        outstandingComm = false;
        
        digitalWrite(LED, HIGH);
        delay(1000);
        digitalWrite(LED, LOW);
        
        triggerFlag = false;
        
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
			byte b = nextMANOI();
			while(b != '*') {
				msg[letter] = b;
				letter++;
				b = nextMANOI();
			}
			
			// Parse the message
			if(msg[0] == 'E') {
				
				// Do something
				if(debug) Serial << "Received message OK" << endl;
				
				digitalWrite(LED, LOW);
				delay(1000);
				digitalWrite(LED, HIGH);
				delay(1000);
				
			}
			
			digitalWrite(LED, LOW);
			triggerFlag = false;
			
		}
		
		/*
		 For some reaon this doesn't work on the 3.3V Arduino Pro Mini 168
         if(millis()%50 == 0) {
         digitalWrite(STATUS, !digitalRead(STATUS));
         }
		 */
		
		if(millis()%50 == 0) {
			digitalWrite(STATUS, !digitalRead(STATUS));
			delay(5);
		}
        
	}
}

byte nextMANOI() {
	
	if(debug) Serial << "Waiting for a byte from MANOI" << endl;
	
	while(1) {
		
		if(nssMANOI.available()) {
			byte b = nssMANOI.read();
			if(debug) Serial << b << endl;
			return b;
		}
		
		if(manoiNSSAttempts >= 100) {
			manoiNSSAttempts = 0;
			break;
		}
		
		manoiNSSAttempts++;
		
	}
	
}

