
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_AHTX0.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define ROT_A  11
#define ROT_B  12
#define ROT_C  10
#define FAN_A 8
#define FAN_B 9
#define HUMID 6
#define HEAT 7

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// temp/humid sensor
Adafruit_AHTX0 aht;






void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(FAN_A, OUTPUT);
  pinMode(FAN_B, OUTPUT);
  pinMode(ROT_A, INPUT);
  pinMode(ROT_B, INPUT);
  pinMode(ROT_C, INPUT);
}

void fanTest(){
  Serial.println("fan test starting");
  Serial.println("fan A");
  digitalWrite(FAN_A, HIGH);
  delay(2000);
  digitalWrite(FAN_A, LOW);
  delay(1000);
  Serial.println("fan B");
  digitalWrite(FAN_B, HIGH);
  delay(2000);
  digitalWrite(FAN_B, LOW);
  Serial.println("fan test complete");
  delay(1000);
}

void sensorTest(){
  Serial.println("sensor test starting");
  if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
  }
  unsigned long start = millis();
  sensors_event_t h, t;
  while (millis() < start + 5000){
    aht.getEvent(&h, &t);
    Serial.print("temp: ");
    Serial.print(t.temperature);
    Serial.print("   humid: ");
    Serial.println(h.relative_humidity);
  }
  Serial.println("sensor test complete");
  delay(1000);
}

void rotaryTest(){
  Serial.println("rotary test starting");
  bool lastA = digitalRead(ROT_A);
  bool lastB = digitalRead(ROT_B);
  bool lastC = 0;
  int counter = 0;
  unsigned long start = millis();
  while (millis() < start + 3000){
    bool currA = digitalRead(ROT_A);
    bool currB = digitalRead(ROT_B);
    if (currA != lastA){
      if (currB != currA){
        counter = counter - (1/4);
      } else {
        counter = counter + (1/4);
      }
    } else if (currB != lastB){
      if (currA == currB){
        counter = counter - (1/4);
      } else {
        counter = counter + (1/4);
      }
    }
    lastA = currA;
    lastB = currB;
    if (!digitalRead(ROT_C)){
      Serial.println("button");
    } else {
      Serial.println(counter);
    }
  }
  Serial.println("rotary test complete");
  delay(1000);
}

void displayTest(){
  Serial.println("display test starting");
  
  Serial.println("display test complete");
  delay(1000);
}

void heatTest(){
  Serial.println("heat test starting");
  digitalWrite(HEAT, HIGH);
  delay(2000);
  digitalWrite(HEAT, LOW);
  Serial.println("heat test complete");
  delay(1000);
}

void humidifierTest(){
  Serial.println("humidifier test starting");
  digitalWrite(HUMID, HIGH);
  delay(2000);
  digitalWrite(HUMID, LOW);
  Serial.println("humidifier test complete");
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("starting unit tests");
  
//  sensorTest();
  rotaryTest();
//  displayTest();
//  fanTest();
//  humidifierTest();
//  heatTest();
  
  Serial.println("all unit tests complete");
  while(1) delay(100);
}
