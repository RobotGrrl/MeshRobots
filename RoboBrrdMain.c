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

// LED pins
int redR(3), greenR(9), blueR(10);
int redL(11), greenL(12), blueL(13);

// LED Values
int L1R = 255;
int L1G = 255;
int L1B = 255;
int R1R = 255;
int R1G = 255;
int R1B = 255;
int L2R = 255;
int L2G = 255;
int L2B = 255;
int R2R = 255;
int R2G = 255;
int R2B = 255;
int preL1R = 0;
int preL1G = 0;
int preL1B = 255;
int preR1R = 0;
int preR1G = 0;
int preR1B = 255;
int preL2R = 0;
int preL2G = 0;
int preL2B = 255;
int preR2R = 0;
int preR2G = 0;
int preR2B = 255;

// Misc.
boolean alternate = true;
int pirCount = 1;
int thresh = 200;
int ldrStable = 0;
int currentDir = 1;

// All the pins
int interruptIncomming = 0; // on pin 2
int interruptOutgoing = 22;

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
    updown.attach(updownPin);
    
    // Home positions
    leftwing.write(0);
    rightwing.write(30);
    //leftright.write(90);
    updown.write(100);
    
	// Communication
	//Serial.begin(9600);
    Serial1.begin(9600);
	
	// Interrupts
	pinMode(interruptOutgoing, OUTPUT);
    digitalWrite(interruptOutgoing, LOW);
    
	attachInterrupt(interruptIncomming, trigger, RISING);
	digitalWrite(2, LOW);
    
    // Sensors
//    pinMode(pir, INPUT);
//    pinMode(tiltFB, INPUT);
//    pinMode(tiltLR, INPUT);
//    pinMode(ldrL, INPUT);
//    pinMode(ldrR, INPUT);
    
    for(int i=0; i<5; i++) {
        moveRightWing(!alternate);
        moveLeftWing(!alternate);
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
    
    //periodicSend();
    
    //pirBehaviour(analogRead(pir));
    ldrBehaviour(analogRead(ldrL), analogRead(ldrR));
    
    if(millis()%500 == 0) {
    updateLights(true);
	}
        
}

void trigger() {
	triggerFlag = true;
}

byte nextByte() {
	while(1) {
		if(Serial.available() > 0) {
			byte b = Serial1.read();
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
            
            Serial1 << "E*";
            
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
        sendToComm('R');
        goRight();
        for(int i=0; i<6; i++) {
            moveRightWing(!alternate);
            alternate = !alternate;
            delay(80);
        }
        ldrStable = 0;
    } else if(ldrL < (ldrR-thresh)) {
        // right
        if(currentDir != 1) goMiddle(); 
        sendToComm('L');
        goLeft();
        for(int i=0; i<6; i++) {
            moveLeftWing(!alternate);
            alternate = !alternate;
            delay(80);
        }
        ldrStable = 0;
    }
    
}

void pirBehaviour(int pirR) {
    
    if(pirR >= 500) {
        
        sendToComm('P');
        
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
            delay(150);
            }
            
        }
    
        pirCount++;
        
    }/* else {
        pirCount = 1; 
    }
    */
    //Serial << "PIR Count: " << pirCount << endl;
    
}

void sendToComm(char c) {
    
    digitalWrite(interruptOutgoing, HIGH);
    
    while(!triggerFlag) {
        // Waiting for trigger to send the data
        if(debug) Serial << "Waiting for the trigger" << endl;
        //digitalWrite(LED, HIGH);
        //delay(50);
        //digitalWrite(LED, LOW);
        //delay(50);
        
        if(triggerAttemptCount >= 100) {
            triggerAttemptCount = 0;
            break;
        }
        
        triggerAttemptCount++;
        
    }
    
    // Ready to send data
    if(triggerFlag) {
        
        if(debug) Serial << "Going to send the message now" << endl;
        
        Serial1 << c << "*";
        
        //digitalWrite(LED, HIGH);
        //delay(1000);
        //digitalWrite(LED, LOW);
        
    }
    
    digitalWrite(interruptOutgoing, LOW);
    triggerFlag = false;
    
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
        
        Serial1 << "P*";
        
        //digitalWrite(LED, HIGH);
        delay(1000);
        //digitalWrite(LED, LOW);
        
    }
    
    digitalWrite(interruptOutgoing, LOW);
    triggerFlag = false;
    
}

void updateLights(boolean independent) {
    
//    fade( preL1R,    preL1G,      preL1B,  // L1 Start
//         256,       256,         256,     // L1 Finish
//         preR1R,    preR1G,      preR1B,  // R1 Start
//         256,       256,         256,     // R1 Finish
//         preL2R,    preL2G,      preL2B,  // L2 Start
//         L2R,       L2G,         L2B,     // L2 Finish
//         preR2R,    preR2G,      preR2B,  // R2 Start
//         R2R,       R2G,         R2B,     // R2 Finish
//         1);
    
//    analogWrite(redR, HIGH);
//    analogWrite(greenR, HIGH);
//    analogWrite(blueR, HIGH);
//    analogWrite(redL, LOW);
//    analogWrite(greenL, LOW);
//    analogWrite(blueL, LOW);
//    
//    delay(500);
//    
//    preL1R = 0;
//    preL1G = 0;
//    preL1B = 0;
//    preR1R = 0;
//    preR1G = 0;
//    preR1B = 0;
    if(independent) {
        
        L1R = int(random(50, 255));
        L1G = int(random(50, 255));
        L1B = int(random(50, 255));
        R1R = int(random(50, 255));
        R1G = int(random(50, 255));
        R1B = int(random(50, 255));
        L2R = int(random(50, 255));
        L2G = int(random(50, 255));
        L2B = int(random(50, 255));
        R2R = int(random(50, 255));
        R2G = int(random(50, 255));
        R2B = int(random(50, 255));
        
    } else {
    
    
        L1R = int(random(50, 255));
        L1G = int(random(50, 255));
        L1B = int(random(50, 255));
    R1R = L1R;//int(random(50, 255));
    R1G = L1G;//int(random(50, 255));
    R1B = L1B;//int(random(50, 255));
        L2R = int(random(50, 255));
        L2G = int(random(50, 255));
        L2B = int(random(50, 255));
        R2R = int(random(50, 255));
        R2G = int(random(50, 255));
        R2B = int(random(50, 255));
        
    }
        
        fade( preL1R,    preL1G,      preL1B,  // L1 Start
              L1R,       L1G,         L1B,     // L1 Finish
              preR1R,    preR1G,      preR1B,  // R1 Start
              R1R,       R1G,         R1B,     // R1 Finish
              preL2R,    preL2G,      preL2B,  // L2 Start
              L2R,       L2G,         L2B,     // L2 Finish
              preR2R,    preR2G,      preR2B,  // R2 Start
              R2R,       R2G,         R2B,     // R2 Finish
              1);
        
        preL1R = L1R;
        preL1G = L1G;
        preL1B = L1B;
        preR1R = R1R;
        preR1G = R1G;
        preR1B = R1B;
        preL2R = L2R;
        preL2G = L2G;
        preL2B = L2B;
        preR2R = R2R;
        preR2G = R2G;
        preR2B = R2B;
    
}


void fade ( int startL1_R,  int startL1_G,  int startL1_B, 
            int finishL1_R, int finishL1_G, int finishL1_B,
            int startR1_R,  int startR1_G,  int startR1_B,
            int finishR1_R, int finishR1_G, int finishR1_B,
            int startL2_R,  int startL2_G,  int startL2_B, 
            int finishL2_R, int finishL2_G, int finishL2_B,
            int startR2_R,  int startR2_G,  int startR2_B,
            int finishR2_R, int finishR2_G, int finishR2_B,
            int stepTime ) {
    
    int skipEveryL1_R = 256/abs(startL1_R-finishL1_R); 
    int skipEveryL1_G = 256/abs(startL1_G-finishL1_G);
    int skipEveryL1_B = 256/abs(startL1_B-finishL1_B); 
    int skipEveryR1_R = 256/abs(startR1_R-finishR1_R); 
    int skipEveryR1_G = 256/abs(startR1_G-finishR1_G);
    int skipEveryR1_B = 256/abs(startR1_B-finishR1_B); 
    int skipEveryL2_R = 256/abs(startL2_R-finishL2_R); 
    int skipEveryL2_G = 256/abs(startL2_G-finishL2_G);
    int skipEveryL2_B = 256/abs(startL2_B-finishL2_B); 
    int skipEveryR2_R = 256/abs(startR2_R-finishR2_R); 
    int skipEveryR2_G = 256/abs(startR2_G-finishR2_G);
    int skipEveryR2_B = 256/abs(startR2_B-finishR2_B); 
    
    for(int i=0; i<256; i++) {
        
        if(startL1_R<finishL1_R) {
            if(i<=finishL1_R) {
                if(i%skipEveryL1_R == 0) {
                    analogWrite(redL, i);
                } 
            }
        } else if(startL1_R>finishL1_R) {
            if(i>=(256-startL1_R)) {
                if(i%skipEveryL1_R == 0) {
                    analogWrite(redL, 256-i); 
                }
            } 
        }
        
        if(startL1_G<finishL1_G) {
            if(i<=finishL1_G) {
                if(i%skipEveryL1_G == 0) {
                    analogWrite(greenL, i);
                } 
            }
        } else if(startL1_G>finishL1_G) {
            if(i>=(256-startL1_G)) {
                if(i%skipEveryL1_G == 0) {
                    analogWrite(greenL, 256-i); 
                }
            } 
        }
        
        if(startL1_B<finishL1_B) {
            if(i<=finishL1_B) {
                if(i%skipEveryL1_B == 0) {
                    analogWrite(blueL, i);
                } 
            }
        } else if(startL1_B>finishL1_B) {
            if(i>=(256-startL1_B)) {
                if(i%skipEveryL1_B == 0) {
                    analogWrite(blueL, 256-i); 
                }
            } 
        }
        
        if(startR1_R<finishR1_R) {
            if(i<=finishR1_R) {
                if(i%skipEveryR1_R == 0) {
                    analogWrite(redR, i);
                } 
            }
        } else if(startR1_R>finishR1_R) {
            if(i>=(256-startR1_R)) {
                if(i%skipEveryR1_R == 0) {
                    analogWrite(redR, 256-i); 
                }
            } 
        }
        
        if(startR1_G<finishR1_G) {
            if(i<=finishR1_G) {
                if(i%skipEveryR1_G == 0) {
                    analogWrite(greenR, i);
                } 
            }
        } else if(startR1_G>finishR1_G) {
            if(i>=(256-startR1_G)) {
                if(i%skipEveryR1_G == 0) {
                    analogWrite(greenR, 256-i); 
                }
            } 
        }
        
        if(startR1_B<finishR1_B) {
            if(i<=finishR1_B) {
                if(i%skipEveryR1_B == 0) {
                    analogWrite(blueR, i);
                } 
            }
        } else if(startR1_B>finishR1_B) {
            if(i>=(256-startR1_B)) {
                if(i%skipEveryR1_B == 0) {
                    analogWrite(blueR, 256-i); 
                }
            } 
        }
        
        delay(stepTime);
        
    }
    
}

