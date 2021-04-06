#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_AHTX0.h>

// constants
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define ROT_A  2
#define ROT_B  3
#define ROT_C  5
#define FAN_A 6
#define FAN_B 8
#define HUMID 10
#define HEAT 4
#define TEMP_FACTOR .1
#define HUMID_FACTOR .23
#define TEMP_OFFSET 0
#define HUMID_OFFSET 0
#define ULONG_MAX 0xffffffff


// declare global vars ////////////////
Adafruit_AHTX0 aht;
sensors_event_t humidity, temp;
byte carousel;
float tempL = 0;
float tempH = 0;
float humidL = 0;
float humidH = 0;
uint8_t bitCursor = 0;
int8_t table[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
bool rotaryTurned = false;
bool heatState = false; 
bool humidState = false;
bool fanAState = false;
bool fanBState = false;
unsigned long fanADelay = 3600000;    // 1 hr
unsigned long fanBDelay = 36000000;   // 10 hr
unsigned long lastInteract = millis();
float counter = 0;
char buff[10];


// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(9600);

  // rotary encoder ////////////////
  pinMode(ROT_A, INPUT);
  pinMode(ROT_B, INPUT);
  pinMode(ROT_C, INPUT);
  attachInterrupt(0, rotaryRead, CHANGE);
  attachInterrupt(1, rotaryRead, CHANGE);
  // nichrome wire ////////////////
  pinMode(HEAT, OUTPUT);
  // ultrasonic transducer
  pinMode(HUMID, OUTPUT);
  // dc fans ////////////////
  pinMode(FAN_A, OUTPUT);
  pinMode(FAN_B, OUTPUT);
  //todo: add pulldown resistor to ground the base of this transistor
  digitalWrite(HEAT, LOW);
  digitalWrite(HUMID, LOW);

  // setup oled display ////////////////
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  display.clearDisplay();
  display.display(); 
  
  // setup temp/humidity sensor ////////////////
  if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  aht.getEvent(&humidity, &temp);
}

// printing helper
void printFormatter(){
  for (int i=0;i<4;i++){
    if (buff[i] == '\0'){
      buff[i] = ' ';
    }
  }
  buff[4] = '\0';
}

void clearBuffer(){
 for (int i=0;i<4;i++){
  buff[i] = ' ';
 }
}

// hardware control center ////////////////
void heatPower(bool power){
  if (power != heatState){
    digitalWrite(HEAT, power);
    heatState = power;
    if (!power && !humidState){
      fanADelay = millis() + 30000;
    } else {
      fanADelay = ULONG_MAX;
      fanAPower(true);
    }
  }
}

void humidPower(bool power){
  if (humidState != power){
    digitalWrite(HUMID, HIGH);
    delay(200);
    digitalWrite(HUMID, LOW);
    humidState = power;
    if (!power && !heatState){
      fanADelay = millis() + 30000;
    } else {
      fanADelay = ULONG_MAX;
      fanAPower(true);
    }
  }
}

void fanAPower(bool power){
  if (power != fanAState){
    digitalWrite(FAN_A, power);
    fanAState = power;
  }
}

void fanBPower(bool power){
  if (power != fanBState){
    digitalWrite(FAN_B, power);
    fanBState = power;
  }
}

void rotaryRead(){
  delay(2);
  bitCursor <<= 2;
  if (digitalRead(ROT_A)) {bitCursor |= 0b10;}
  if (digitalRead(ROT_B)) {bitCursor |= 0b01;}
  bitCursor &= 0x0f;
  counter += table[bitCursor];
  rotaryTurned = table[bitCursor] != 0;
}

void homeScreen(){
  aht.getEvent(&humidity, &temp);
  float currTemp = temp.temperature + TEMP_OFFSET;
  int currHumid = humidity.relative_humidity + HUMID_OFFSET;

  // in or out of bounds center ///////////////
  if (currTemp < tempL){
    heatPower(true);
  } else if (currTemp >= tempH){
    heatPower(false);
  }

  if (currHumid < humidL){
    humidPower(true);
  } else if (currHumid >= humidH){
    humidPower(false);
  }



  // timers ////////////////
  if (millis() > fanADelay){
    if (fanAState){
      fanADelay = millis() + 3600000;    // off 1 hr
    } else {
      fanADelay = millis() + 60000;      // on 1 min
    }
    fanAPower(!fanAState);
  }

  if (millis() > fanBDelay){
  	if (fanBState){
	  	fanBDelay = millis() + 36000000;		// off 10 hrs
  	} else {
  		fanBDelay = millis() + 300000; 			// on 5 min
  	}
    fanBPower(!fanBState);
  }
  
  // printing center ////////////////
  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 5);

  dtostrf(currTemp, 4, 1, buff);
  Serial.println(buff);
  
  display.print(buff);
  if (heatState){
    if (currTemp < 10){
      display.print(" ");
    }
    display.println("*");
  } else {
    display.println("");
  }

  display.print(currHumid);
  if (humidState){
    if (currHumid == 100){
      display.print(" ");
    } else if (currHumid >= 10){
      display.print("  ");
    } else {
      display.print("   ");
    }
    display.println("*");
  } else {
    display.println("");
  }
}

void tempLowScreen(){
  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  if (tempL > tempH){
    counter = tempH / TEMP_FACTOR;
  }

  tempL = (float)((int)(counter * TEMP_FACTOR*10)-(int)(counter * TEMP_FACTOR*10)%5)/10.0;

  tempL = ((int)(tempL*10)-(int)(tempL*10)%5)/10.0;
  dtostrf(tempL, 0, 1, buff);
  printFormatter();
  display.print(buff);
  display.println("C");
}

void tempHighScreen(){
  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  if (tempH < tempL){
    counter = tempL / TEMP_FACTOR;
  }

  tempH = (float)((int)(counter * TEMP_FACTOR*10)-(int)(counter * TEMP_FACTOR*10)%5)/10.0;
  
  dtostrf(tempH, 0, 1, buff);
  printFormatter();
  display.print(buff);
  display.println("C");
}

void humidLowScreen(){
  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0, 32);
  
  if (humidL < 0){
    counter = 0;
  }
  if (humidL > humidH){
    counter = humidH / HUMID_FACTOR;
  }

  humidL = counter * HUMID_FACTOR;
  
  dtostrf((int)humidL, 0, 0, buff);
  printFormatter();
  display.print(buff);
  display.println("%");
}

void humidHighScreen(){
  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0, 32);

  if (humidH > 95){
    counter = 95 / HUMID_FACTOR;
  }
  if (humidH < humidL){
    counter = humidL / HUMID_FACTOR;
  }
  
  humidH = counter * HUMID_FACTOR;

  dtostrf((int)humidH, 0, 0, buff);
  printFormatter();
  display.print(buff);
  display.println("%");
}

void loop() {
  // page switch statement ////////////////
  switch (carousel){
    case 0:		// home
      if (millis() - lastInteract > 500){
        lastInteract = millis();
        homeScreen();
        display.display();
      }
      delay(25);
      break;
    case 2:		// minimum temperature
      if (rotaryTurned){
        lastInteract = millis();
        tempLowScreen();
        display.display();
      } else if (millis() - lastInteract > 30000){
        carousel = 0;
      }
      break;
    case 1:		// maximum temperature
      if (rotaryTurned){
        rotaryTurned = false;
        lastInteract = millis();
        tempHighScreen();
        display.display();
      } else if (millis() - lastInteract > 30000){
        carousel = 0;
      }
      break;
    case 4:		// minimum humidity
      if (rotaryTurned){
        rotaryTurned = false;
        lastInteract = millis();
        humidLowScreen();
        display.display();
      } else if (millis() - lastInteract > 30000){
        carousel = 0;
      }
      break;
    case 3:		// maximum humidity
      if (rotaryTurned){
        rotaryTurned = false;
        lastInteract = millis();
        humidHighScreen();
        display.display();
      } else if (millis() - lastInteract > 30000){
        carousel = 0;
      }
      break;
  }

  // button check ////////////////
  if (!digitalRead(ROT_C)){
    carousel = (carousel + 1) % 5;
    lastInteract = millis();
    clearBuffer();
    
    while (!digitalRead(ROT_C)){
      delay(10);
    }
    switch (carousel){
    case 0:
      homeScreen();
      break;
    case 2:
      counter = tempL/TEMP_FACTOR;
      tempLowScreen();
      break;
    case 1:
      counter = tempH/TEMP_FACTOR;
      tempHighScreen();
      break;
    case 4:
      counter = humidL/HUMID_FACTOR;
      humidLowScreen();
      break;
    case 3:
      counter = humidH/HUMID_FACTOR;
      humidHighScreen();
      break;
    }
    display.display();
  }
}
