
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_AHTX0.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define ROT_A  2
#define ROT_B  3
#define ROT_C  5
#define FAN_A 8
#define FAN_B 10
#define HUMID 12
#define HEAT 6


// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// temp/humid sensor
// BMP280 i2c address: 0x77 AHT20 i2c address: 0x38
Adafruit_AHTX0 aht;
const char tComplete[] PROGMEM = " test complete";
const char tStarting[] PROGMEM = " test starting";
int counter = 0;
uint8_t bitCursor = 0;
int8_t table[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(ROT_A, INPUT);
  pinMode(ROT_B, INPUT);
  pinMode(ROT_C, INPUT);
  pinMode(FAN_A, OUTPUT);
  pinMode(FAN_B, OUTPUT);
  pinMode(HUMID, OUTPUT);
  pinMode(HEAT, OUTPUT);

  digitalWrite(HEAT, LOW);
  digitalWrite(FAN_A, LOW);
  digitalWrite(FAN_B, LOW);
  digitalWrite(HUMID, LOW);
}

void fanTest(){
  oneLiner(2, F("fan test starting"));
  delay(2000);
  oneLiner(4, F("fan A"));
  digitalWrite(FAN_A, HIGH);
  delay(10000);
  digitalWrite(FAN_A, LOW);
  
  oneLiner(4, F("fan B"));
  delay(2000);
  digitalWrite(FAN_B, HIGH);
  delay(10000);
  digitalWrite(FAN_B, LOW);
  oneLiner(2, F("fan test complete"));
  delay(2000);
}

void sensorTest(){
  oneLiner(2, F("sensor test starting"));
  delay(2000);
  if (! aht.begin()) {
    oneLiner(2, F("Could not find AHT? Check wiring"));
  }
  unsigned long start = millis();
  sensors_event_t h, t;
  while (millis() < start + 10000){
    aht.getEvent(&h, &t);
    display.clearDisplay();
    display.setTextSize(4);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println(String(t.temperature, 1));
    display.println((int)h.relative_humidity);
    display.display();
    delay(100);
  }
  oneLiner(2, F("sensor test complete"));
  delay(2000);
}

bool rotaryRead(){
  delay(2);
  bitCursor <<= 2;
  if (digitalRead(ROT_A)) {bitCursor |= 0b10;}
  if (digitalRead(ROT_B)) {bitCursor |= 0b01;}
  bitCursor &= 0x0f;
  counter += table[bitCursor];
  if(table[bitCursor] != 0){
    Serial.println(counter);
    return true;
  }
  return false;
}

void rotaryTest(){
  oneLiner(2, F("rotary test starting"));
  delay(2000);
  
  attachInterrupt(digitalPinToInterrupt(ROT_A), rotaryRead, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROT_B), rotaryRead, CHANGE);

  unsigned long testPeriod = millis() + 10000;
  int b_counter = 0;
  while (millis() < testPeriod){
    if (!digitalRead(ROT_C)){
      oneLiner(3, F("button"));
    } else {
      oneLiner(4, String(counter));
    }
    
  }

  detachInterrupt(digitalPinToInterrupt(ROT_A));
  detachInterrupt(digitalPinToInterrupt(ROT_B));

  oneLiner(2, F("rotary test complete"));
  delay(2000);
}

bool displayTest(){
  Serial.println(F("display test starting"));
  delay(2000);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    return false;
  } else {
    Serial.println(F("found display"));
  }

  display.clearDisplay();
  for(int16_t i=0; i<max(display.width(),display.height())/2; i+=2) {
    display.drawCircle(display.width()/2, display.height()/2, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  
  delay(2000);
  
  oneLiner(2, F("display test complete"));
  delay(2000);
  return true;
}

void heatTest(){
  oneLiner(2, F("heat test starting"));
  delay(2000);
  digitalWrite(HEAT, HIGH);
  delay(10000);
  digitalWrite(HEAT, LOW);
  oneLiner(2, F("heat test complete"));
  delay(2000);
}

void humidifierTest(){
  oneLiner(2, F("humidifier test starting"));
  digitalWrite(HUMID, HIGH);
  delay(10000);
  digitalWrite(HUMID, LOW);
  oneLiner(2, F("humidifier test complete"));
  delay(2000);
}

void oneLiner(int size, String thing){
  Serial.println(thing);
  
  display.clearDisplay();
  display.setTextSize(size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(thing);
  display.display();
}

void loop() {
  Serial.println(F("starting unit tests"));
  
  if (displayTest()){
//    rotaryTest(); 
//    sensorTest();
//     fanTest();
//     humidifierTest();
     heatTest();
//    oneLiner(4, "done!");
  }
  
  
  Serial.println(F("all unit tests complete"));
  while(1) delay(100);
}
