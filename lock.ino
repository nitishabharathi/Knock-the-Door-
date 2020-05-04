#include <Servo.h>
Servo myservo;  


const int knockSensor = 0;         
const int programSwitch = 2;       
const int lockMotor = 3;          


const int threshold = 50;          
const int rejectValue = 25;        
const int averageRejectValue = 15; 
const int knockFadeTime = 150;    
const int maximumKnocks = 20;      
const int knockComplete = 1200;    

int secretCode[maximumKnocks] = {50, 25, 25, 50, 100, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  // Initial setup   "Shave and a Hair Cut, two bits."
int knockReadings[maximumKnocks];  
int knockSensorValue = 0;          
int programButtonPressed = false;  

void setup() {

  myservo.attach(lockMotor); 
  pinMode(lockMotor, OUTPUT);
  pinMode(programSwitch, INPUT);
  Serial.begin(9600);                    
  Serial.println("Program start.");       

}

void loop() {
  knockSensorValue = analogRead(knockSensor);

  if (digitalRead(programSwitch) == HIGH) {
    programButtonPressed = true;          
  } else {
    programButtonPressed = false;
  }

  if (knockSensorValue >= threshold) {
    listenToSecretKnock();
  }
}

void listenToSecretKnock() {
  Serial.println("knock starting");
  int i = 0;
  for (i = 0; i < maximumKnocks; i++) {
    knockReadings[i] = 0;
  }
  int currentKnockNumber = 0;             
  int startTime = millis();                
  int now;

  delay(knockFadeTime);                              
  do {
     knockSensorValue = analogRead(knockSensor);
    if (knockSensorValue >= threshold) {                
       Serial.println("knock.");
       now = millis();
       knockReadings[currentKnockNumber] = now - startTime;
       currentKnockNumber ++;                            
       startTime = now;
       delay(knockFadeTime);                           
    }

    now = millis();
  } while ((now - startTime < knockComplete) && (currentKnockNumber < maximumKnocks));
  if (programButtonPressed == false) {         
    if (validateKnock() == true) {
      triggerDoorUnlock();
    } else {
      Serial.println("Secret knock failed.");
    }
  } else { 
    validateKnock();
    
    Serial.println("New lock stored.");

  }
}

void triggerDoorUnlock() {
  Serial.println("Door unlocked!");
  myservo.write(0);
  delay(200);
  myservo.detach();
  delay(5000);
  myservo.attach(lockMotor);
  myservo.write(90);
}


boolean validateKnock() {
  int i = 0;
  int currentKnockCount = 0;
  int secretKnockCount = 0;
  int maxKnockInterval = 0;                

  for (i = 0; i < maximumKnocks; i++) {
    if (knockReadings[i] > 0) {
      currentKnockCount++;
    }
    if (secretCode[i] > 0) {            
      secretKnockCount++;
    }

    if (knockReadings[i] > maxKnockInterval) {  
      maxKnockInterval = knockReadings[i];
    }
  }

  if (programButtonPressed == true) {
    for (i = 0; i < maximumKnocks; i++) { 
      secretCode[i] = map(knockReadings[i], 0, maxKnockInterval, 0, 100);
    }

    delay(50);
    for (i = 0; i < maximumKnocks ; i++) {
     
      if (secretCode[i] > 0) {
        delay( map(secretCode[i], 0, 100, 0, maxKnockInterval));
       
      }
      delay(50);
}
    return false;   
 }

  int totaltimeDifferences = 0;
  int timeDiff = 0;
  for (i = 0; i < maximumKnocks; i++) { 
    knockReadings[i] = map(knockReadings[i], 0, maxKnockInterval, 0, 100);
    timeDiff = abs(knockReadings[i] - secretCode[i]);
    if (timeDiff > rejectValue) { 
      return false;
    }
    totaltimeDifferences += timeDiff;
  }
  
  if (totaltimeDifferences / secretKnockCount > averageRejectValue) {
    return false;
  }

  return true;

}
