#define ROT_A 3
#define ROT_B 2
#define BUTTON 5

int counter = 0;
uint8_t bitCursor = 0;
int8_t table[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  pinMode(ROT_A, INPUT);
  pinMode(ROT_B, INPUT);
  
  attachInterrupt(digitalPinToInterrupt(ROT_A), rotaryRead, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROT_B), rotaryRead, CHANGE);
}

bool rotaryRead(){
  delay(2);
  bitCursor <<= 2;
  if (digitalRead(ROT_B)) {bitCursor |= 0b10;}
  if (digitalRead(ROT_A)) {bitCursor |= 0b01;}
  bitCursor &= 0x0f;
  counter += table[bitCursor];
  if(table[bitCursor] != 0){
    Serial.println(counter);
    return true;
  }
  return false;
}

//void rotating(){
//  delay(2);
//  if (digitalRead(DATA) == digitalRead(CLOCK)){
//    counter--;
//  } else {
//    counter++;
//  }
//  Serial.println(counter);
//}

void loop() {
  // put your main code here, to run repeatedly:
  if (!digitalRead(BUTTON)){
    while(!digitalRead(BUTTON)){};
    Serial.println("button");
  }
}
