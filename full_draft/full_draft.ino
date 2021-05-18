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
#define FAN_A 8
#define FAN_B 6
#define HUMID 10
#define HEAT 4
#define TEMP_FACTOR .1
#define HUMID_FACTOR .25
#define FRESH_FACTOR .25
#define MIST_FACTOR .25
#define TEMP_OFFSET 0
#define HUMID_OFFSET 0
#define ULONG_MAX 0xffffffff


// declare global vars ////////////////
Adafruit_AHTX0 aht;
sensors_event_t humidity, temp;
byte carousel = 1;
bool fahrenheit = false;
float tempL = 0;
float tempH = 0;
float humidL = 0;
float humidH = 0;
float freshAir = 24;
float mist = 24;
uint8_t bitCursor = 0;
int8_t table[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
bool rotaryTurned = false;
bool heatState = false; 
bool humidState = false;
bool mistState = false;
bool fanAState = false;
bool fanBState = false;
unsigned long fanATrigger = 3600000;        // 1 hr
unsigned long fanBTrigger = 36000000;       // 10 hr
unsigned long mistTrigger = ULONG_MAX;       // infinity
unsigned long fanBOffPeriod = 36000000;     // 10 hr
unsigned long fanBOnPeriod = 300000;        // 5 min
unsigned long lastInteract = millis();
float counter = 0;
char buff[10];


// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(9600);

  Serial.println(F("beginning"));

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

  
  Serial.println(F("sensor beginning"));
  // setup temp/humidity sensor ////////////////
  if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
  }
  Serial.println(F("sensor done"));
  aht.getEvent(&humidity, &temp);

  // finished setup 
  display.clearDisplay();
  for(int16_t i=0; i<max(display.width(),display.height())/2; i+=2) {
    display.drawCircle(display.width()/2, display.height()/2, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
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
    if (!power && !humidState && !mistState){
      fanATrigger = millis() + 30000;
    } else if (power){
      fanATrigger = ULONG_MAX;
      fanAPower(true);
    }
  }
}

void humidPower(bool power){
  if (humidState != power){
    digitalWrite(HUMID, power);
    humidState = power;
    if (!power && !heatState && !mistState){
      fanATrigger = millis() + 30000;
    } else if (power){
      fanATrigger = ULONG_MAX;
      fanAPower(true);
    }
  }
}

void mistPower(bool power){
  if (mistState != power){
    digitalWrite(HUMID, power);
    mistState = power;
    if (!power && !heatState && !mistState){
      fanATrigger = millis() + 30000;
    } else if (power){
      fanATrigger = ULONG_MAX;
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
  lastInteract = millis();
}

void homeScreen(){
	// get current reading from sensor
  aht.getEvent(&humidity, &temp);
  float currTemp;
  if (fahrenheit){
  	currTemp = ((temp.temperature + TEMP_OFFSET) * 9/5) + 32;
  } else {
	currTemp = temp.temperature + TEMP_OFFSET;
  }
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
  if (millis() > fanATrigger){
    if (fanAState){
      fanATrigger = millis() + 3600000;    // off 1 hr
    } else {
      fanATrigger = millis() + 60000;      // on 1 min
    }
    fanAPower(!fanAState);
  }

  if (millis() > fanBTrigger){
  	if (fanBState){
	  	fanBTrigger = millis() + freshAir * 3600000;	    // off x hrs
  	} else {
  		fanBTrigger = millis() + fanBOnPeriod; 			// on 5 min
  	}
    fanBPower(!fanBState);
  }

  if (millis() > mistTrigger){
    if (mistState){
      mistTrigger = millis() + mist * 3600000;     // off x hrs
    } else {
      mistTrigger = millis() + 20000;      // on 20 seconds
    }
    mistPower(!mistState);
  }

  
  // printing center ////////////////
  
  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

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
  Serial.println(currHumid);
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
  display.display();
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
  display.setTextSize(3);
  display.println(F("tempL"));
  display.display();
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
  display.setTextSize(3);
  display.println(F("tempH"));
  display.display();
}

void humidLowScreen(){
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(F("humidL"));
  display.setTextSize(4);
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
  display.display();
}

void humidHighScreen(){
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(F("humidH"));
  display.setTextSize(4);
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
  display.display();
}

void freshAirScreen(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(F("fresh air"));
  display.setTextSize(4);
  display.setCursor(0, 32);

  if (freshAir > 24){
    counter = 24 / FRESH_FACTOR;
  } 
  if (freshAir < 1){
    counter = 1 / FRESH_FACTOR;
  }

  freshAir = counter * FRESH_FACTOR;

  if (freshAir < 24){
    fanBTrigger = freshAir * 3600000;
    dtostrf((int)freshAir, 0, 0, buff);
    printFormatter();
    display.print(buff);
    display.println("hr");
  } else {
    fanBTrigger = ULONG_MAX;
    display.println("off");
  }
  display.display();
}

void mistScreen(){
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(F("mist"));
  display.setTextSize(4);
  display.setCursor(0, 32);

  if (mist > 24){
    counter = 24 / MIST_FACTOR;
  } 
  if (mist < 1){
    counter = 1 / MIST_FACTOR;
  }
  

  mist = counter * FRESH_FACTOR;

  if (mist < 24){
    mistTrigger = mist * 3600000;
    dtostrf((int)mist, 0, 0, buff);
    printFormatter();
    display.print(buff);
    display.println("hr");
  } else {
    mistTrigger = ULONG_MAX;
    display.println("off");
  }
  display.display();
}

void pauseScreen(){
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(F("Paused"));

  humidPower(false);
  heatPower(false);
  fanAPower(false);
  fanBPower(false);
  mistPower(false);

  display.display();
}

void carouselCase(const void *screen()){
  screen();
  if (millis() - lastInteract > 30000){
    carousel = 1;
  }
}

void loop() {
  // page switch statement ////////////////
  switch (carousel){
    case 0:
      pauseScreen();
      break;
    case 1:		// home
      if (millis() - lastInteract > 500){
        lastInteract = millis();
        homeScreen();
      }
      break;
    case 2:
      carouselCase(&tempHighScreen);
      break;
    case 3:
      carouselCase(&tempLowScreen);
      break;
    case 4:
      carouselCase(&humidHighScreen);
      break;
    case 5:
      carouselCase(&humidLowScreen);
      break;
    case 6:
      carouselCase(&freshAirScreen);
      break;
    case 7:
      carouselCase(&mistScreen);
      break;
  }

  // button check ////////////////
  if (!digitalRead(ROT_C)){
    carousel = (carousel % 7) + 1;
    lastInteract = millis();
    clearBuffer();
    Serial.println(F("button"));

    while (!digitalRead(ROT_C)){
       if (millis() > lastInteract + 3000){
          
          Serial.println(F("pause threshold reached"));
         carousel = 0;
         pauseScreen();
       }
      delay(10);
    }

    // counter initialization
    switch (carousel){
      case 1:
        homeScreen();
        break;
      case 2:
        counter = tempH/TEMP_FACTOR;
        break;
      case 3:
        counter = tempL/TEMP_FACTOR;
        break;
      case 4:
        counter = humidH/HUMID_FACTOR;
        break;
      case 5:
        counter = humidL/HUMID_FACTOR;
        break;
      case 6:
        counter = freshAir/FRESH_FACTOR;
        break;
      case 7:
        counter = mist/MIST_FACTOR;
        break;
      case 0:
        break;
    }
  }
}
