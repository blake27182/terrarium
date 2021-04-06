

#define button 10
#define outA 3
#define outB 2

void setup() {
  // put your setup code here, to run once:
  pinMode(button, INPUT);
  pinMode(outA, INPUT);
  pinMode(outB, INPUT); 
  Serial.begin(9600);
}

bool lastState = 0;
bool currState = 0;
int currPos = 0;

void loop() {
  // put your main code here, to run repeatedly:
  
  currState = digitalRead(outA);
  if (currState != lastState){
    lastState = currState;
    if (currState == digitalRead(outB)){
      currPos++;
    } else {
      currPos--;
    }
  }
//
//  if (!digitalRead(button)){
//    currPos = 0;
//    lastState = currState = 0;
//  }

  Serial.println(currPos);
  delay(50);
}
