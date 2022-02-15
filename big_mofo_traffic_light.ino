unsigned char lightStates[][10] = {
  {0, B10100100, 1, 0, 9, 1, 1, 1, 1, 1},
  {1, B10110100, 2, 0, 0, 2, 1, 1, 1, 1},
  {2, B10001100, 3, 3, 3, 3, 10, 2, 1, 2},
  {3, B10010100, 4, 4, 4, 4, 2, 2, 1, 2},
  {4, B10100100, 5, 0, 9, 10, 1, 1, 1, 1},
  {5, B10100110, 6, 5, 9, 6, 1, 1, 1, 1},
  {6, B10100001, 7, 7, 7, 7, 10, 2, 1, 2},
  {7, B10100010, 8, 8, 8, 8, 2, 2, 1, 2},
  {8, B10100100, 0, 0, 9, 10, 1, 0, 1, 0},
  {9, B00000000, 0, 0, 8, 0, 1, 0, 1, 0},
  {10, B10100100, 0, 0, 8, 11, 1, 0, 1, 1},
  {11, B01100100, 0, 0, 8, 1, 1, 0, 1, 5},
};
/*Array called lightStates
   0,Index
   1,States
   2,next state
   3,next state in emergency conditions
   4,next state under fault conditions
   5,next state in pedestrian conditions
   6,time in state
   7,time in state during emergency conditions
   8,time in state under fault conditions
   9,time in state under pedestrian conditons
*/

//Defining array elements
#define index 0
#define stateData 1
#define nextState 2
#define nextStateEmer 3
#define nextStateFault 4
#define nextStateCross 5
#define stateTime 6
#define stateTimeEmer 7
#define stateTimeFault 8
#define stateTimeCross 9
//Defining pins
#define redA 12
#define amberA 11
#define greenA 10
#define redB 9
#define amberB 8
#define greenB 7
#define crossWait 6
#define crossGo 5
#define crossStop 4
//defining bits for the states
#define bitCrossStop 7
#define bitCrossGo 6
#define bitRedA 5
#define bitAmberA 4
#define bitGreenA 3
#define bitRedB 2
#define bitAmberB 1
#define bitGreenB 0
//other variables
#define crossButton 2
#define timer 13    //shows timer clock pulse on inbuilt LED on pin 13
//defining other variables
byte currentState = 0;
int multiplier = 0;
bool emerFlag = false;
bool faultFlag = false;
bool crossFlag = false;
String input = "";
bool string = false;



void setup() {
  Serial.begin(9600); // Begin serial monitor
  input.reserve(200); // Reserve 200 bytes for user input
  startLights(); // Visual test to show lights are working
  startTimer(); // Sets up and starts timer interrupts
  attachInterrupt(digitalPinToInterrupt(crossButton), crossRequest, FALLING);
}

void loop() {

  if (string) {
    checkString();
    input = "";
    string = false;
  }
  if (multiplier == 0) {
    if (emerFlag) {
      currentState = (lightStates[currentState][nextStateEmer]);
      setMultiplier(lightStates[currentState][stateTimeEmer]);
      Serial.println("Emergency mode activated. Enter 'c' to stop.");
    }
    else if (faultFlag) {
      currentState = (lightStates[currentState][nextStateFault]);
      setMultiplier(lightStates[currentState][stateTimeFault]);
      Serial.println("Fault mode activated. Enter 'c' to stop.");
    }
    else if (crossFlag) {
      currentState = (lightStates[currentState][nextStateCross]);
      setMultiplier(lightStates[currentState][stateTimeCross]);

      if ((crossFlag) && (lightStates[currentState][stateData] == B01100100)) {
        digitalWrite(crossWait, LOW);
        Serial.println("Crossing is GREEN");
        crossFlag = false;
      }
    }
    else {
      currentState = (lightStates[currentState][nextState]);
      setMultiplier(lightStates[currentState][stateTime]);
      Serial.println("Crossing is Red");
      Serial.println("Lights under normal operation");
    }
    setLights(lightStates[currentState][stateData]);
  }
}

void startLights() {
  pinMode(timer, OUTPUT);
  pinMode(redA, OUTPUT);
  pinMode(amberA, OUTPUT);
  pinMode(greenA, OUTPUT);
  pinMode(redB, OUTPUT);
  pinMode(amberB, OUTPUT);
  pinMode(greenB, OUTPUT);
  pinMode(crossButton, INPUT_PULLUP);
  pinMode(crossWait, OUTPUT);
  pinMode(crossStop, OUTPUT);
  pinMode(crossGo, OUTPUT);

  setLights(1023); // All on to test
  digitalWrite(crossWait, HIGH);
  delay(1000);
  setLights(0); //All flash test
  digitalWrite(crossWait, LOW);

  delay(500);
  setLights(1023);
  digitalWrite(crossWait, HIGH);
  delay(500);

  // initialise lights state and tracking
  setLights(lightStates[currentState][stateData]); // Array element 0 and all red
  setMultiplier(lightStates[currentState][stateTime]); // Kick off the time countdown
  Serial.println("Lights are under normal operating conditions");
}

void setMultiplier(int timeDelay) {

  multiplier = round(timeDelay);
}


void crossRequest() {
  crossFlag = true;
  setMultiplier(lightStates[currentState][stateTimeCross]); // Kick off pedestrian timer countdown
  digitalWrite(crossWait, HIGH);
  Serial.println("Crossing button activated");
}

void checkString() {
  input.trim();
  if (input == "e") {
    emerFlag = true;
    setMultiplier(lightStates[currentState][stateTimeEmer]); // Kick off emergency timer countdown
  }
  else if (input == "c") {
    emerFlag = false;
    faultFlag = false;
    currentState = (lightStates[8][nextState]);
  }
  else if (input == "p") {
    crossRequest();
  }
  else if (input == "f") {
    faultFlag = true;
    setMultiplier(lightStates[currentState][stateTimeFault]); // Kick off fault timer countdown
  }
  else if (input == "r") {
    startLights();
    faultFlag = false;
    emerFlag = false;
    crossFlag = false;
  }
}

void setLights(unsigned char lights) {
  digitalWrite(redA, (lights >> bitRedA) & 1 );
  digitalWrite(amberA, (lights >> bitAmberA) & 1 );
  digitalWrite(greenA, (lights >> bitGreenA) & 1 );
  digitalWrite(redB, (lights >> bitRedB) & 1 );
  digitalWrite(amberB, (lights >> bitAmberB) & 1 );
  digitalWrite(greenB, (lights >> bitGreenB) & 1 );
  digitalWrite(crossStop, (lights >> bitCrossStop) & 1 );
  digitalWrite(crossGo, (lights >> bitCrossGo) & 1 );
}


void serialEvent() {
  while (Serial.available()) {
    char command = (char)Serial.read(); // get the new byte:
    input += command; // add it to the inputString:
    // if the incoming character is a newline, set a flag so the main loop can do something about it:
    if (command == '\n') {
      string = true;
      Serial.print(input); // Confirmation
    }
    if (command == 'a') {
      unsigned char changeTime = (unsigned char)Serial.parseInt();
      if ((changeTime > 0) && (changeTime < 60)) {
        (lightStates[2][stateTime]) = changeTime;
        Serial.print("Green A time= ");
        Serial.print(lightStates[2][stateTime]);
        Serial.println(" seconds");
      }
      else {
        Serial.println("out of range. Please enter value between 1 and 60 seconds");
      }
    }
    if (command == 'b') {
      unsigned char changeTime = (unsigned char)Serial.parseInt();
      if ((changeTime > 0) && (changeTime < 60)) {
        (lightStates[6][stateTime]) = changeTime;
        Serial.print("Green B time= ");
        Serial.print(lightStates[6][stateTime]);
        Serial.println(" seconds");
      }
      else {
        Serial.println("out of range. Please enter value between 1 and 60 seconds");
      }
    }
  }


}


void startTimer() {
  // initialise timer1
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  // Max Period = (Prescale)*(1/Frequency)*(2^17) or (Prescale)*(6.25 x 10^-8)*(131072)
  TCNT1 = 34286;            // preload timer 65536-16MHz/256/2Hz - 34286 = roughly half a second overflow
  TCCR1B |= (1 << CS12);    // 256 prescaler
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
}
//******************************************************************************************************
// interrupt service routine that wraps a user defined function supplied by attachInterrupt
ISR(TIMER1_OVF_vect)
{
  TCNT1 = 34286;            // preload timer. Smaller number = longer delay as we are counting up to overflow
  digitalWrite(timer, digitalRead(timer) ^ 1); // Invert pin value
  multiplier--;
}
