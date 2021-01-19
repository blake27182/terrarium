#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_AHTX0.h>

Adafruit_AHTX0 aht;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define ROT_A  11
#define ROT_B  12
#define ROT_BUT  10

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


// declare running vars
int looper, carousel;
bool lastA, lastB, lastBut, currA, currB;
float tempL, tempH, humidL, humidH, humidTolerance, tempTolerance;
sensors_event_t humidity, temp;

void setup() {
  Serial.begin(115200);

  // setup rotary encoder
  pinMode(ROT_A, INPUT);
  pinMode(ROT_B, INPUT);
  pinMode(ROT_BUT, INPUT);
  lastA = digitalRead(ROT_A);
  lastB = digitalRead(ROT_B);
  lastBut = 0;

  // setup oled display
  carousel = 0;
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
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
  looper = 100;
  tempL = 15;
  tempH = 30;
  humidL = 75;
  humidH = 100;
  tempTolerance = 1;
  humidTolerance = 5;
  
}

void homeScreen(){
  
  aht.getEvent(&humidity, &temp);
    
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  
  display.print("T: ");
  display.print(String(temp.temperature, 1));
  display.println("  C");
  display.print("H: ");
  display.print((int) humidity.relative_humidity);
  display.println("    %");
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
  currA = digitalRead(ROT_A);
  currB = digitalRead(ROT_B);
  if (currA != lastA){
    if (currB != currA){
      counter = counter - (increment/4);
    } else {
      counter = counter + (increment/4);
    }
  } else if (currB != lastB){
    if (currA == currB){
      counter = counter - (increment/4);
    } else {
      counter = counter + (increment/4);
    }
  } else {
    return false;
  }
  lastA = currA;
  lastB = currB;
  return true;
}

void customDelay(int x){
  
}

void loop() {
  
  switch (carousel){
    case 0:
      delay(1000);
      homeScreen();
      display.display();
      break;
    case 1:
      if (rotaryRead(tempL, .5)){
        tempLowScreen();
        display.display();
      }
      break;
    case 2:
      if (rotaryRead(tempH, .5)){
        tempHighScreen();
        display.display();
      }
      break;
    case 3:
      if (rotaryRead(humidL, 1)){
        humidLowScreen();
        display.display();
      }
      break;
    case 4:
      if (rotaryRead(humidH, 1)){
        humidHighScreen();
        display.display();
      }
      break;
  }

  

  if (digitalRead(ROT_BUT)){
    carousel = (carousel + 1) % 5;
    while (digitalRead(ROT_BUT)){
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
