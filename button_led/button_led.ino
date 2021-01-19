#define ron 6
#define led 12

bool state = false;
bool button;

void setup() {
  // put your setup code here, to run once:
  pinMode(ron, INPUT);
  pinMode(led, OUTPUT);
}

void loop() {
  button = digitalRead(ron);
  if (button){
    if (!state){
      digitalWrite(led, HIGH);
      state = true;
    } else {
      digitalWrite(led, LOW);
      state = false;
    }
    while(digitalRead(ron)){
      delay(10);
    }
  }
    
  delay(10);
}
