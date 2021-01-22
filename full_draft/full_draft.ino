#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_AHTX0.h>

Adafruit_AHTX0 aht;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define ROT_A  4
#define ROT_B  3
#define ROT_C  2
#define FAN_A 7
#define FAN_B 6
#define HUMID 8
#define HEAT 5

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


// declare running vars
int looper, carousel;
bool lastA, lastB, lastBut, currA, currB;
float tempL, tempH, humidL, humidH, humidTolerance, tempTolerance;
sensors_event_t humidity, temp;
uint8_t cursor = 0;
int8_t table[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
bool heatState, humidState;
unsigned long lastInteract;

void setup() {
  Serial.begin(9600);

  // setup rotary encoder
  pinMode(ROT_A, INPUT);
  pinMode(ROT_B, INPUT);
  pinMode(ROT_C, INPUT);
  pinMode(HEAT, OUTPUT);
  pinMode(HUMID, OUTPUT);
  pinMode(FAN_A, OUTPUT);
  pinMode(FAN_B, OUTPUT);

  digitalWrite(HEAT, LOW);
  
  lastInteract = millis();
  carousel = 0;

  // setup oled display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  display.clearDisplay();

  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  // Display static text
  display.println("hey?");
  display.display(); 
  
  // setup temp sensor
  if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  aht.getEvent(&humidity, &temp);
  tempL = 10;
  tempH = 30;
  humidL = 5;
  humidH = 50;
  heatState = false;
  humidState = false;
}

void heatSwitch(bool power){
  digitalWrite(HEAT, power);
  digitalWrite(FAN_A, power|humidState);
  heatState = power;
}

void humidSwitch(bool power){
  if (humidState != power){
    digitalWrite(FAN_A, power|heatState);
//    digitalWrite(HUMID, HIGH);
//    delay(10);
//    digitalWrite(HUMID, LOW);
    humidState = power;
  }
}

void homeScreen(){
  
  aht.getEvent(&humidity, &temp);

  if (!heatState){
    if (temp.temperature < tempL){
      heatSwitch(true);
    }
  } else {
    if (temp.temperature >= tempH){
      heatSwitch(false);
    }
  }

  if (!humidState){
    if (humidity.relative_humidity < humidL){
      humidSwitch(true);
    }
  } else {
    if (humidity.relative_humidity >= humidH){
      humidSwitch(false);
    }
  }
    
  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  
  display.print(String(temp.temperature, 1));
  display.println("");
  display.print((int) humidity.relative_humidity);
  display.println("");
}

void tempLowScreen(){
  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  if (tempL >= tempH - tempTolerance){
    tempL = tempH - tempTolerance;
  }

  float temp = ((int)(tempL*10)-(int)(tempL*10)%5)/10.0;
  display.print(String(temp, 1));
  display.println("C");
}

void tempHighScreen(){
  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  if (tempH <= tempL + tempTolerance){
    tempH = tempL + tempTolerance;
  }
  
  float temp = ((int)(tempH*10)-(int)(tempH*10)%5)/10.0;
  display.print(String(temp, 1));
  display.println("C");
}

void humidLowScreen(){
  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  
  if (humidL < 0){
    humidL = 0;
  }
  if (humidL >= humidH - humidTolerance){
    humidL = humidH - humidTolerance;
  }
  display.print((int)humidL);
  display.println("%");
}

void humidHighScreen(){
  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  if (humidH > 100){
    humidH = 100;
  }
  if (humidH <= humidL + humidTolerance){
    humidH = humidL + humidTolerance;
  }
  display.print((int)humidH);
  display.println("%");
}

bool rotaryRead(float &counter, float increment){
  cursor <<= 2;
  if (digitalRead(ROT_A)) cursor |= 0x2;
  if (digitalRead(ROT_B)) cursor |= 0x1;
  cursor &= 0x0f;
  counter += table[cursor] * increment * .25;
  if(table[cursor] == 0){
    return false;
  }
  return true;
}

void loop() {
  
  switch (carousel){
    case 0:
      if (millis() - lastInteract > 500){
        lastInteract = millis();
        homeScreen();
        display.display();
      }
      delay(25);
      break;
    case 1:
      if (rotaryRead(tempL, .5)){
        lastInteract = millis();
        tempLowScreen();
        display.display();
      } else if (millis() - lastInteract > 30000){
        carousel = 0;
      }
      break;
    case 2:
      if (rotaryRead(tempH, .5)){
        lastInteract = millis();
        tempHighScreen();
        display.display();
      } else if (millis() - lastInteract > 30000){
        carousel = 0;
      }
      break;
    case 3:
      if (rotaryRead(humidL, 1)){
        lastInteract = millis();
        humidLowScreen();
        display.display();
      } else if (millis() - lastInteract > 30000){
        carousel = 0;
      }
      break;
    case 4:
      if (rotaryRead(humidH, 1)){
        lastInteract = millis();
        humidHighScreen();
        display.display();
      } else if (millis() - lastInteract > 30000){
        carousel = 0;
      }
      break;
  }

  

  if (!digitalRead(ROT_C)){
    carousel = (carousel + 1) % 5;
    lastInteract = millis();
    while (!digitalRead(ROT_C)){
      delay(10);
    }
    switch (carousel){
    case 0:
      homeScreen();
      break;
    case 1:
      tempLowScreen();
      break;
    case 2:
      tempHighScreen();
      break;
    case 3:
      humidLowScreen();
      break;
    case 4:
      humidHighScreen();
      break;
    }
    display.display();
  }
  
}
