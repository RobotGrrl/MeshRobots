/*
 * RoboBrrdMain.c
 * For the RoboBrrd robot!
 *
 * May 2011
 * RobotGrrl.com 
 */

#include <Time.h>
#include <Servo.h> 
#include <Streaming.h>

boolean debug = false;

// Servos
Servo mouth, leftwing, rightwing, leftright, updown;

// Sensors
int pir(A0), tiltFB(1), tiltLR(2), ldrL(3), ldrR(4);

// Servo pins
int mouthPin(8), leftwingPin(7), rightwingPin(6);
int leftrightPin(5), updownPin(4);

// Misc.
boolean alternate = true;
int pirCount = 1;
int thresh = 200;
int ldrStable = 0;
int currentDir = 1;

// All the pins
int interruptIncomming = 0; // on pin 2
int interruptOutgoing = 12;

// Counters
int triggerAttemptCount = 0;
int commAttemptCount = 0;

// Trigger flag
volatile boolean triggerFlag = false;

int pos = 0;

// Initialize
void setup() {

    // Servos
    leftwing.attach(leftwingPin);
    rightwing.attach(rightwingPin);
    leftright.attach(leftrightPin);
    //updown.attach(updownPin);
    
    // Home positions
    leftwing.write(0);
    rightwing.write(30);
    //leftright.write(90);
    //updown.write(90);
    
	// Communication
	Serial.begin(9600);
	
	// Interrupts
	pinMode(interruptOutgoing, OUTPUT);
    digitalWrite(interruptOutgoing, LOW);
    
	attachInterrupt(interruptIncomming, trigger, RISING);
	digitalWrite(2, LOW);
    
    // Sensors
    pinMode(pir, INPUT);
    pinMode(tiltFB, INPUT);
    pinMode(tiltLR, INPUT);
    pinMode(ldrL, INPUT);
    pinMode(ldrR, INPUT);
    
    for(int i=0; i<5; i++) {
        moveRightWing(!alternate);
        //moveLeftWing(!alternate);
        alternate = !alternate;
        delay(500);
    }
    
    rightwing.write(30);

	
}

void loop() {
    
    /*
	while(!triggerFlag) {
		if(debug) Serial << "Trigger flag is false..." << endl;
		
        
        // * * * * * * * * * * * * *
        // Do whatever activity here
        // * * * * * * * * * * * * *
        
        //sensorReadings();
        
		//ldrBehaviour(ldrLReading, ldrRReading);
        //pirBehaviour(pirReading);
        //exerciseBehaviour(100);
        
//        leftwing.write(60);
//        delay(2000);
//        
//        leftwing.write(90);
//        delay(2000);
//        
//        leftwing.write(120);
//        delay(2000);
        
        //moveLeftWing(alternate);
        
        //moveRightWing(!alternate);
        //moveLeftWing(!alternate);
        //alternate = !alternate;
        //delay(1000);
		
	}
     */
     
    
	if(triggerFlag) {
		
        boolean alt = false;
        for(int i=0; i<5; i++) {
            moveRightWing(alt);
            alt = !alt;
            delay(50);
        }

        
		if(debug) Serial << "Trigger flag is set, sending outgoing interrupt" << endl;
		//digitalWrite(STATUS, LOW);
		
		//digitalWrite(LED, HIGH);
		
		// Send the flag to receive the message
		digitalWrite(interruptOutgoing, HIGH);
		delay(10);
		
        
        // * * * * * * * * * * * * * *
        // Check for the right messages
        // * * * * * * * * * * * * * *
        
		// Check if it's getting the message
		if(nextByte() == 'E') {
			// Show that we received the message
			
			if(debug) Serial << "Received the message" << endl;
			
            boolean alt = false;
            for(int i=0; i<5; i++) {
                moveRightWing(alt);
                alt = !alt;
                delay(50);
            }
			
		}
        
		
		delay(50);
		//digitalWrite(LED, LOW);
		digitalWrite(interruptOutgoing, LOW);
		triggerFlag = false;

		
	}
    
    periodicSend();
    
    pirBehaviour(analogRead(pir));
     
     
	
}

void trigger() {
	triggerFlag = true;
}

byte nextByte() {
	while(1) {
		if(Serial.available() > 0) {
			byte b = Serial.read();
			//if(debug) Serial << "Received byte: " << b << endl;
			return b;
		}
        
        
        if(commAttemptCount >= 100) {
			commAttemptCount = 0;
			break;
		}
		
		commAttemptCount++;
        
		//if(debug) Serial << "Waiting for next byte" << endl;
	}
	
}


// Example of sending data to the comm. board
// Originally used in MANOI
void periodicSend() {
    
    // Send some data to the communication board
    if(second()%30 == 0 || second()%60 == 0) {
        
        //digitalWrite(STATUS, LOW);
        
        //digitalWrite(LED, HIGH);
        delay(500);
        //digitalWrite(LED, LOW);
        
        // Signal that we want to send data
        digitalWrite(interruptOutgoing, HIGH);
        
        while(!triggerFlag) {
            // Waiting for trigger to send the data
            if(debug) Serial << "Waiting for the trigger" << endl;
            //digitalWrite(LED, HIGH);
            delay(50);
            //digitalWrite(LED, LOW);
            delay(50);
            // TODO: Make it give up at some point
            
            if(triggerAttemptCount >= 100) {
                triggerAttemptCount = 0;
                break;
            }
            
            triggerAttemptCount++;
            
        }
        
        // Ready to send data
        if(triggerFlag) {
            
            if(debug) Serial << "Going to send the message now" << endl;
            
            Serial << "E*";
            
            //digitalWrite(LED, HIGH);
            delay(1000);
            //digitalWrite(LED, LOW);
            
        }
        
        digitalWrite(interruptOutgoing, LOW);
        triggerFlag = false;
        
    }
    
}

void sensorReadings() {
    int pirReading = analogRead(pir);
    int tiltFBReading = analogRead(tiltFB);
    int tiltLRReading = analogRead(tiltLR);
    int ldrLReading = analogRead(ldrL);
    int ldrRReading = analogRead(ldrR);
    
    if(false) {
        Serial << " PIR: " << pirReading;
        Serial << " TiltFB: " << tiltFBReading;
        Serial << " TiltLR: " << tiltLRReading;
        Serial << " LDR L: " << ldrLReading;
        Serial << " LDR R: " << ldrRReading;
        Serial << endl;
    }
}


void ldrBehaviour(int ldrL, int ldrR) {
    
    if(ldrL < (ldrR+thresh) && ldrL > (ldrR-thresh)) {
        // neutral
        
        if(ldrStable >= 5000) {
            goMiddle();
            ldrStable = 0;
        }
        
        ldrStable++;
        
    } else if(ldrL > (ldrR+thresh)) {
        // left (but go to the right)
        if(currentDir != 1) goMiddle(); 
        goRight();
        ldrStable = 0;
    } else if(ldrL < (ldrR-thresh)) {
        // right
        if(currentDir != 1) goMiddle(); 
        goLeft();
        ldrStable = 0;
    }
    
}

void pirBehaviour(int pirR) {
    
    if(pirR >= 500) {
        
        sendPToComm();
        
        if(pirCount % 5 == 0) {
            openMouth();
            delay(1500);
            closeMouth();
            delay(100);
        } else {
            
            for(int i=0; i<6; i++) {
            moveLeftWing(alternate);
            moveRightWing(!alternate);
            alternate = !alternate;
            delay(80);
            }
            
        }
    
        pirCount++;
        
    }/* else {
        pirCount = 1; 
    }
    */
    //Serial << "PIR Count: " << pirCount << endl;
    
}

void sendPToComm() {
    
    digitalWrite(interruptOutgoing, HIGH);
    
    while(!triggerFlag) {
        // Waiting for trigger to send the data
        if(debug) Serial << "Waiting for the trigger" << endl;
        //digitalWrite(LED, HIGH);
        delay(50);
        //digitalWrite(LED, LOW);
        delay(50);
        // TODO: Make it give up at some point
        
        if(triggerAttemptCount >= 100) {
            triggerAttemptCount = 0;
            break;
        }
        
        triggerAttemptCount++;
        
    }
    
    // Ready to send data
    if(triggerFlag) {
        
        if(debug) Serial << "Going to send the message now" << endl;
        
        Serial << "P*";
        
        //digitalWrite(LED, HIGH);
        delay(1000);
        //digitalWrite(LED, LOW);
        
    }
    
    digitalWrite(interruptOutgoing, LOW);
    triggerFlag = false;
    
}

