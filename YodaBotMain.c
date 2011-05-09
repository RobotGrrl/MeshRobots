/*
 *  YodaBotMain.c
 *  
 *
 *  Created by Erin Kennedy on 10-11-18.
 *  Copyright 2010 robotgrrl.com. All rights reserved.
 *
 */

#include <NewSoftSerial.h>
#include <Streaming.h>
#include <Encoder.h>
#include <Motor.h>

boolean debug = true;

// Rampage Robot
Motor left(5, 6);
Motor right(9, 10);

// All the pins
int interruptIncoming = 2;
int interruptOutgoing = 11;
int rbbbRX = 3;
int rbbbTX = 4;
int LED = 13; // green, on yoda's hand

// NewSoftSerial
NewSoftSerial nssRBBB(rbbbRX, rbbbTX);

// Trigger flag
volatile boolean triggerFlag = false;


// Initialize
void setup() {
	
	if(debug) Serial << "Initializing main Yoda Bot board" << endl;
	
	// Communication
	Serial.begin(9600);
	nssRBBB.begin(9600);
	
	// Outputs
	pinMode(LED, OUTPUT);
	
	// Interrupts
	pinMode(interruptOutgoing, OUTPUT);
	attachInterrupt(0, trigger, RISING);
		
}

void loop() {

	while(!triggerFlag) {
		if(debug) Serial << "Trigger flag is false, driving" << endl;
		drive();
	}
	
	if(triggerFlag) {
		
		if(debug) Serial << "Trigger flag is set, sending outgoing interrupt" << endl;
		
		// Send the flag to receive the message
		digitalWrite(interruptOutgoing, HIGH);
		
		// Check if it's getting the message
		if(nextByte() == 'E') {
			// Show that we received the message
			
			if(debug) Serial << "Received the message" << endl;
			
			digitalWrite(LED, HIGH);
			delay(2000);
			digitalWrite(LED, LOW);
			triggerFlag = false;
		}
		
		delay(50);
		digitalWrite(interruptOutgoing, LOW);
		
	}
	
}

void trigger() {
	triggerFlag = true;
}

byte nextByte() {
	while(1) {
		if(nssRBBB.available()) {
			byte b = nssRBBB.read();
			if(debug) Serial << "Received byte: " << b << endl;
			return b;
		}
		if(debug) Serial << "Waiting for next byte" << endl;
	}
	
}

// This is an old function for controlling the wheels
void drive() {
	
	for(int i=0; i<256; i++) {
		
		left.rate(i);
		right.rate(i);
		
		left.drive(true);
		right.drive(true);
		
		delay(10);
		
	}
	
	for(int i=255; i>0; i--) {
		
		left.rate(i);
		right.rate(i);
		
		left.drive(true);
		right.drive(true);
		
		delay(10);
		
	}
	
	left.stop();
	right.stop();
	
	delay(500);
	
	for(int i=0; i<256; i++) {
		
		left.rate(i);
		right.rate(i);
		
		left.drive(false);
		right.drive(false);
		
		delay(10);
		
	}
	
	for(int i=255; i>0; i--) {
		
		left.rate(i);
		right.rate(i);
		
		left.drive(false);
		right.drive(false);
		
		delay(10);
		
	}
	
	left.stop();
	right.stop();
	
	delay(500);
	
}
