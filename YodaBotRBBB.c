/*
 *  YodaBotRBBB.c
 *  
 *
 *  Created by Erin Kennedy on 10-11-17.
 *  Copyright 2010 robotgrrl.com. All rights reserved.
 *
 */

#include <NewSoftSerial.h>
#include <Streaming.h>

boolean debug = false;

// All the pins
int interruptOutgoing = 8;
int interruptIncoming = 2;
int yodaRX = 7;
int yodaTX = 8;
int xbRX = 10;
int xbTX = 11;
int LED = 9; // green, near servo

// NewSoftSerial
NewSoftSerial nssXB(xbTX, xbRX);
//NewSoftSerial nssArduino(yodaRX, yodaTX);

// Trigger flag
volatile boolean triggerFlag = false;


// Initialize
void setup() {
	
	if(debug) Serial << "RBBB Beginning initialization" << endl;
	
	// Communication
	Serial.begin(9600);
	nssXB.begin(9600);
	//nssArduino.begin(9600);
	
	// Outputs
	pinMode(LED, OUTPUT);
	
	// Interrupts
	pinMode(interruptOutgoing, OUTPUT);
	attachInterrupt(0, trigger, RISING);
	
	if(debug) Serial << "Done init" << endl;
	
}

void loop() {

	if(nextXB() == 'E') {
		
		if(debug) Serial << "Byte is E" << endl;
		
		// Send out the interrupt
		digitalWrite(interruptOutgoing, HIGH);
		delay(50);
		digitalWrite(interruptOutgoing, LOW);
		
		while(!triggerFlag) {
			
			// Waiting for trigger to send the data
			if(debug) Serial << "Waiting for trigger" << endl;
			digitalWrite(LED, !digitalRead(LED));
			delay(50);
		}
		
		if(triggerFlag) {
			
			// Sending the message now
			Serial << "E";
			//nssArduino.print("EEEEEEEEEEEEEEEEEEEEEEEEE");
			
			if(debug) Serial << "Sending the message now" << endl;
			
			digitalWrite(LED, HIGH);
			delay(1000);
			digitalWrite(LED, LOW);
			
			triggerFlag = false;
		}
		
	}
	
}

void trigger() {
	triggerFlag = true;
}

byte nextXB() {
	
	if(debug) Serial << "Waiting for a byte" << endl;
	
	while(1) {
		
		if(Serial.available() > 0) {
			byte b = Serial.read();
			if(debug) Serial << b << endl;
			return b;
		}
		
		if(nssXB.available()) {
			byte b = nssXB.read();
			digitalWrite(LED, HIGH);
			delay(100);
			digitalWrite(LED, LOW);
			delay(100);
			if(debug) Serial << b << endl;
			return b;
		}
	}
}
