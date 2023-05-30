//to do:
//creare meniu --- facut
//automat de reglare --- am facut preset-uri de la inceput, trebuie facuta doar calibrarea senzorului
//update umiditate cand se asteapta/uda --- facut
//sa apara default/last values inainte de setarea lor --- facut
//functie de reset millis --- facut

//idei:
//lipire sageti pe * - < si # - > pt meniu --- facut
//menu=C --- facut
//next=D --- facut
//A-B preset-uri de program
//cu sau fara senzor

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#define HUMIDITY_SENSOR A3
#define WATER_PUMP 12

extern volatile unsigned long timer0_millis;

LiquidCrystal_I2C lcd(0x27,16,2);

int OpenAirReading = 10000;   
int WaterReading = 0;     
int MoistureLevel = 0;
int SoilMoisturePercentage = 0;
int previousSoilMoisturePercentage = 0;
int previous_min_humidity = 40;
int min_humidity = 40; //by default: 40%
int previous_stop_watering_humidity = 70;
int stop_watering_humidity = 70;
bool stop_watering_humidity_inserted = 0;


const int ROW_NUM = 4;
const int COLUMN_NUM = 4;
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};
byte pin_rows[ROW_NUM] = {9, 8, 7, 6};
byte pin_column[COLUMN_NUM] = {5, 4, 3, 2};
Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

unsigned long previousMillis;
unsigned long currentMillis;
long previous_watering_period = 5000;
long previous_waiting_period = 60000;
long watering_period = 5000; //by default: 5s
long waiting_period = 60000; //by default: 60s

bool MENU = 0;
int page = 1;

void setup() {
  lcd.init();
  lcd.backlight();
  pinMode(WATER_PUMP, OUTPUT);
  digitalWrite(WATER_PUMP, HIGH);
  Serial.begin(9600);
  currentMillis = millis();
}

void loop() {
  bool version = version_check();
  if(version == 1) {
    intro2();
    while(true) {
      version_without_sensor();
    }
  }
  else if(version == 0) {
    intro();
    sensor_check();
    previousSoilMoisturePercentage = SoilMoisturePercentage;
    lcd.clear();
    while(true) {
      version_with_sensor();
    }
  }
}

void version_with_sensor() {
  sensor_check();
  if(check_MENU() == 1) {
    MENU = 1;
    menu();
  }
  if(SoilMoisturePercentage != previousSoilMoisturePercentage) {
    print_humidity();
    previousSoilMoisturePercentage = SoilMoisturePercentage;
    delay(200); //when done, remove
  }
  if(SoilMoisturePercentage < min_humidity) {
    watering();
  }
}

void version_without_sensor() {
  if(check_MENU2() == 1) {
    MENU = 1;
    menu2();
  }
  watering2();
}

bool version_check() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Version:");
  lcd.setCursor(0,1);
  lcd.print("Press 'NEXT'");
  if(press_D() == 1) {
    lcd.clear();
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("With sensor-A");
  lcd.setCursor(0,1);
  lcd.print("Without sensor-B");
  char c = pressed_key();
  while(1) {
    if(c == 'A') {
      return 0;
    }
    else if(c == 'B') {
      return 1;
    }
    c = pressed_key();
  }
}

bool check_MENU() {
  char key = keypad.getKey();
  if(key == 'C') {
    return 1;
  }
  return 0;
}

bool check_MENU2() {
  char key = keypad.getKey();
  if(key == 'C') {
    return 1;
  }
  return 0;
}

void intro2() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Press NEXT");
  lcd.setCursor(0,1);
  lcd.print("for calibration");
  if(press_D() == 1) {
    lcd.clear();
  }
  set_periods_minutes();
}

void intro() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Press NEXT");
  lcd.setCursor(0,1);
  lcd.print("for calibration");
  if(press_D() == 1) {
    lcd.clear();
  }
  calibration();
}

void watering() {
  while(SoilMoisturePercentage < stop_watering_humidity) {
    //Serial.println(SoilMoisturePercentage);
    if(check_MENU() == 1) {
      MENU = 1;
      break;
    }
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Watering...");
    lcd.setCursor(0,1);
    lcd.print("Humidity:");
    lcd.setCursor(9,1);
    lcd.print(SoilMoisturePercentage);
    lcd.setCursor(12,1);
    lcd.print("%");
    digitalWrite(WATER_PUMP, LOW);
    previousMillis = millis();
    currentMillis=millis();
    while(currentMillis - previousMillis <= watering_period) {
      if(check_MENU() == 1) {
        MENU = 1;
        break;
      }
      sensor_check();
      if(SoilMoisturePercentage != previousSoilMoisturePercentage) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Watering...");
        lcd.setCursor(0,1);
        lcd.print("Humidity:");
        lcd.setCursor(9,1);
        lcd.print(SoilMoisturePercentage);
        lcd.setCursor(12,1);
        lcd.print("%");
        previousSoilMoisturePercentage = SoilMoisturePercentage;
        delay(250);
      }
      currentMillis = millis(); 
    }
    digitalWrite(WATER_PUMP, HIGH);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Waiting...");
    lcd.setCursor(0,1);
    lcd.print("Humidity:");
    lcd.setCursor(9,1);
    lcd.print(SoilMoisturePercentage);
    lcd.setCursor(12,1);
    lcd.print("%");
    if(MENU == 1) {
      break;
    }
    while(currentMillis - previousMillis <= waiting_period) {
      if(check_MENU() == 1) {
        MENU = 1;
        break;
      }
      sensor_check();
      if(SoilMoisturePercentage != previousSoilMoisturePercentage) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Waiting...");
        lcd.setCursor(0,1);
        lcd.print("Humidity:");
        lcd.setCursor(9,1);
        lcd.print(SoilMoisturePercentage);
        lcd.setCursor(12,1);
        lcd.print("%");
        previousSoilMoisturePercentage = SoilMoisturePercentage;
        delay(250);
      }
      currentMillis = millis(); 
    }
    if(MENU == 1) {
      break;
    }
    sensor_check();
  }
  if(MENU == 1) {
    menu();
  }
  reset_millis();
}

void watering2() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Watering...");
  digitalWrite(WATER_PUMP, LOW);
  previousMillis = millis();
  currentMillis=millis();
  while(currentMillis - previousMillis <= watering_period) {
    if(check_MENU() == 1) {
      MENU = 1;
      break;
    }
    currentMillis = millis(); 
  }
  digitalWrite(WATER_PUMP, HIGH);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Waiting...");
  while(currentMillis - previousMillis <= waiting_period) {
    if(MENU == 1) {
      break;
    }
    if(check_MENU() == 1) {
      MENU = 1;
      break;
    }
  currentMillis = millis(); 
  }
  if(MENU == 1) {
    menu2();
  }
  reset_millis();
}

void page1() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Back");
  lcd.setCursor(15,1);
  lcd.print(">");
}

void page2() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set minimum");
  lcd.setCursor(0,1);
  lcd.print("humidity     <|>");
}

void page3() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Recalibrate");
  lcd.setCursor(0,1);
  lcd.print("sensor       <|>");
}

void page4() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set watering and");
  lcd.setCursor(0,1);
  lcd.print("waiting time <|>");
}

void page5() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set stop water-");
  lcd.setCursor(0,1);
  lcd.print("ing humidity <|>");
}

void page6() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Factory reset");
  lcd.setCursor(15,1);
  lcd.print("<");
}

void set_page_previous() {
  if(page == 1) {
    page1();
  }
  else if(page == 2) {
    page1();
    page = 1;
  }
  else if(page == 3) {
    page2();
    page = 2;
  }
  else if(page == 4) {
    page3();
    page = 3;
  }
  else if(page == 5) {
    page4();
    page = 4;
  }
  else if(page == 6) {
    page5();
    page = 5;
  }
}

void set_page_next() {
  if(page == 1) {
    page2();
    page = 2;
  }
  else if(page == 2) {
    page3();
    page = 3;
  }
  else if(page == 3) {
    page4();
    page = 4;
  }
  else if(page == 4) {
    page5();
    page = 5;
  }
  else if(page == 5) {
    page6();
    page = 6;
  }
  else if(page == 6) {
    page6();
  }
}

void menu() {
  digitalWrite(WATER_PUMP, HIGH);
  reset_millis();
  page = 1;
  page1();
  char c = pressed_key();
  while(c != 'D') {
    if(c == '*') {
      set_page_previous();
    }
    else if(c == '#') {
      set_page_next();
    }
    c = pressed_key();
  }
  if(page == 1) {
    MENU = 0;
  }
  else if(page == 2) {
    set_minimum_humidity();
    MENU = 0;
  }
  else if(page == 3) {
    calibration();
    MENU = 0;
  }
  else if(page == 4) {
    set_periods();
    MENU = 0;
  }
  else if(page == 5) {
    set_stop_watering_humidity();
    MENU = 0;
  }
  else if(page == 6) {
    loop();
    MENU = 0;
  }
}

void page1_2() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Back");
  lcd.setCursor(15,1);
  lcd.print(">");
}

void page2_2() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set watering and");
  lcd.setCursor(0,1);
  lcd.print("waiting time <|>");
}

void page3_2() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Factory reset");
  lcd.setCursor(15,1);
  lcd.print("<");
}

void set_page_previous_2() {
  if(page == 1) {
    page1_2();
  }
  else if(page == 2) {
    page1_2();
    page = 1;
  }
  else if(page == 3) {
    page2_2();
    page = 2;
  }
}

void set_page_next_2() {
  if(page == 1) {
    page2_2();
    page = 2;
  }
  else if(page == 2) {
    page3_2();
    page = 3;
  }
  else if(page == 3) {
    page3_2();
  }
}

void menu2() {
  digitalWrite(WATER_PUMP, HIGH);
  reset_millis();
  page = 1;
  page1_2();
  char c = pressed_key();
  while(c != 'D') {
    if(c == '*') {
      set_page_previous_2();
    }
    else if(c == '#') {
      set_page_next_2();
    }
    c = pressed_key();
  }
  if(page == 1) {
    MENU = 0;
  }
  else if(page == 2) {
    set_periods_minutes();
    MENU = 0;
  }
  else if(page == 3) {
    loop();
    MENU = 0;
  }
}

void calculate_stop_watering_humidity() {
  if(min_humidity <= 40) {
    stop_watering_humidity = 70;
  }
  else if(min_humidity > 40 && min_humidity <= 55) {
    stop_watering_humidity = 75;
  }
  else if(min_humidity > 55 && min_humidity <= 70) {
    stop_watering_humidity = 85;
  }
  else if(min_humidity > 70 && min_humidity <= 80) {
    stop_watering_humidity = 90;
  }
  else if(min_humidity > 80 && min_humidity <= 90) {
    stop_watering_humidity = 95;
  }
  else {
    stop_watering_humidity = min_humidity + 1;
  }
  stop_watering_humidity_inserted = 1;
}

void set_stop_watering_humidity() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set stop water-");
  lcd.setCursor(0,1);
  lcd.print("ing humidity");
  if(press_D() == 1) {
    lcd.clear();
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Auto set: '<'");
  lcd.setCursor(0,1);
  lcd.print("Manual set: '>'");
  char c = pressed_key();
  bool leave = 0;
  while(leave == 0) {
    if(c == '*') {
      calculate_stop_watering_humidity();
      leave = 1;
    }
    else if(c == '#') {
      manual_set_stop_watering_humidity();
      leave = 1;
    }
    c = pressed_key();
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Stop watering");
  lcd.setCursor(0,1);
  lcd.print("humidity:");
  lcd.setCursor(10,1);
  lcd.print(stop_watering_humidity);
  previousMillis = millis();
  currentMillis=millis();
  while(currentMillis - previousMillis <= 5000) { //must be 5000 when ready
  currentMillis = millis();
  }
}

void manual_set_stop_watering_humidity() {
  previous_stop_watering_humidity = stop_watering_humidity;
  stop_watering_humidity = 0;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Must be between");
  lcd.setCursor(0,1);
  lcd.print("1 and 99");
  if(press_D() == 1) {
    lcd.clear();
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("and greater than");
  lcd.setCursor(0,1);
  lcd.print("minimum humidity");
  if(press_D() == 1) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("New:");
    lcd.setCursor(9,0);
    lcd.print("%");
    lcd.setCursor(0,1);
    lcd.print("Previous:");
    lcd.setCursor(10,1);
    lcd.print(previous_stop_watering_humidity);
    lcd.setCursor(12,1);
    lcd.print("%");
    int pos = 5;
    char c = press_key_not_D();
    int p = 5;
    int swh = 0;
    while(c != 'D' && pos < 7) {
      if(is_number(c) == 1) {
        int number = c - '0';
        stop_watering_humidity = stop_watering_humidity * 10 + number;
        lcd.setCursor(pos,0);
        lcd.print(c);
        pos++;
      }
      c = pressed_key();
      if(is_number(c) == 1) {
        int n = c - '0';
        swh = stop_watering_humidity * 10 + n;
        p++;
      }
      if(pos == 6 && stop_watering_humidity < min_humidity && c == 'D') {
        pos = 5;
        p=5;
        stop_watering_humidity = 0;
        swh = 0;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Smaller than");
        lcd.setCursor(0,1);
        lcd.print("minimum humidity");
        delay(5000);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("New:");
        lcd.setCursor(9,0);
        lcd.print("%");
        lcd.setCursor(0,1);
        lcd.print("Previous:");
        lcd.setCursor(10,1);
        lcd.print(previous_stop_watering_humidity);
        lcd.setCursor(12,1);
        lcd.print("%");
        c = pressed_key();
      }
      if(pos == 7 && stop_watering_humidity < min_humidity) {
        pos = 5;
        p=5;
        stop_watering_humidity = 0;
        swh = 0;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Smaller than");
        lcd.setCursor(0,1);
        lcd.print("minimum humidity");
        delay(5000);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("New:");
        lcd.setCursor(9,0);
        lcd.print("%");
        lcd.setCursor(0,1);
        lcd.print("Previous:");
        lcd.setCursor(10,1);
        lcd.print(previous_stop_watering_humidity);
        lcd.setCursor(13,1);
        lcd.print("%");
        c = pressed_key();
      }
    }
  }
}

void set_minimum_humidity() {
  previous_min_humidity = min_humidity;
  min_humidity = 0;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set minimum");
  lcd.setCursor(0,1);
  lcd.print("humidity");
  if(press_D() == 1) {
    lcd.clear();
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Must be between");
  lcd.setCursor(0,1);
  lcd.print("0 and 99");
  if(press_D() == 1) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("New:");
    lcd.setCursor(9,0);
    lcd.print("%");
    lcd.setCursor(0,1);
    lcd.print("Previous:");
    lcd.setCursor(10,1);
    lcd.print(previous_min_humidity);
    lcd.setCursor(12,1);
    lcd.print("%");
    int pos = 5;
    char c = pressed_key();
    while(c != 'D' && pos < 7) {
      if(is_number(c) == 1) {
        int number = c - '0';
        min_humidity = min_humidity * 10 + number;
        lcd.setCursor(pos,0);
        lcd.print(c);
        pos++;
      }
      c = pressed_key();
    }
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Minimum");
  lcd.setCursor(0,1);
  lcd.print("humidity:");
  lcd.setCursor(10,1);
  lcd.print(min_humidity);
  previousMillis = millis();
  currentMillis=millis();
  delay(5000);
  if(min_humidity >= stop_watering_humidity) {
    calculate_stop_watering_humidity();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Stop watering");
    lcd.setCursor(0,1);
    lcd.print("humidity has");
    delay(4000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("been adjusted");
    lcd.setCursor(0,1);
    lcd.print("New value:");
    lcd.setCursor(10,1);
    lcd.print(stop_watering_humidity);
    delay(4000);
  }

}

void calibration() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Keep sensor dry:");
  lcd.setCursor(0,1);
  lcd.print("in the air");
  if(press_D() == 1) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Wait 10 seconds");
    lcd.setCursor(0,1);
    lcd.print("Thank you!");
  }
  previousMillis = millis();
  currentMillis=millis();
  while(currentMillis - previousMillis <= 10000) { //must be 10000
    MoistureLevel = analogRead(HUMIDITY_SENSOR);
    if(MoistureLevel < OpenAirReading) {
      OpenAirReading = MoistureLevel;
    }
    delay(50);
    currentMillis = millis(); 
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Insert sensor in");
  lcd.setCursor(0,1);
  lcd.print("water");
  if(press_D() == 1) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Wait 10 seconds");
    lcd.setCursor(0,1);
    lcd.print("Thank you!");
  }
  previousMillis = millis();
  currentMillis=millis();
  while(currentMillis - previousMillis <= 10000) { //must be 10000
    MoistureLevel = analogRead(HUMIDITY_SENSOR); 
    if(MoistureLevel > WaterReading) {
      WaterReading = MoistureLevel;
    }
    currentMillis = millis(); 
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Insert sensor in");
  lcd.setCursor(0,1);
  lcd.print("the soil");
  if(press_D() == 1) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("When ready,");
    lcd.setCursor(0,1);
    lcd.print("press NEXT");
  }
  if(press_D() == 1) {
    print_humidity();
  }
}

int press_D() {
  while(pressed_key() != 'D') {
    
  }
  return 1;
}

void set_periods_minutes() {
  previous_watering_period = watering_period;
  previous_waiting_period = waiting_period;
  watering_period = 0;
  waiting_period = 0;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Press NEXT for");
  lcd.setCursor(0,1);
  lcd.print("watering time");
  if(press_D() == 1) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("New(sec):");
    lcd.setCursor(0,1);
    lcd.print("Previous:");
    lcd.setCursor(9,1);
    lcd.print(previous_watering_period / 60000);
    int pos = 9;
    char c = pressed_key();
    while(c != 'D' && pos < 14) {
      if(is_number(c) == 1) {
        int number = c - '0';
        watering_period = watering_period * 10 + number;
        lcd.setCursor(pos,0);
        lcd.print(c);
        pos++;
      }
      c = pressed_key();
    }
    watering_period = watering_period * 1000;
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Press NEXT for");
  lcd.setCursor(0,1);
  lcd.print("waiting time");
  if(press_D() == 1) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("New(min):");
    lcd.setCursor(0,1);
    lcd.print("Previous:");
    lcd.setCursor(9,1);
    lcd.print(previous_waiting_period / 60000);
    int pos = 9;
    char c = pressed_key();
    while(c != 'D' && pos < 14) {
      if(is_number(c) == 1) {
        int number = c - '0';
        waiting_period = waiting_period * 10 + number;
        lcd.setCursor(pos,0);
        lcd.print(c);
        pos++;
      }
      c = pressed_key();
    }
    waiting_period = waiting_period * 60000;
  }

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Watering:");
  lcd.setCursor(9,0);
  lcd.print(watering_period/1000);
  lcd.setCursor(0,1);
  lcd.print("Waiting:");
  lcd.setCursor(8,1);
  lcd.print(waiting_period/60000);
  delay(5000);
}

void set_periods() {
  previous_watering_period = watering_period;
  previous_waiting_period = waiting_period;
  watering_period = 0;
  waiting_period = 0;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Press NEXT for");
  lcd.setCursor(0,1);
  lcd.print("watering time");
  if(press_D() == 1) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("New(sec):");
    lcd.setCursor(0,1);
    lcd.print("Previous:");
    lcd.setCursor(9,1);
    lcd.print(previous_watering_period / 1000);
    int pos = 9;
    char c = pressed_key();
    while(c != 'D' && pos < 14) {
      if(is_number(c) == 1) {
        int number = c - '0';
        watering_period = watering_period * 10 + number;
        lcd.setCursor(pos,0);
        lcd.print(c);
        pos++;
      }
      c = pressed_key();
    }
    watering_period = watering_period * 1000;
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Press NEXT for");
  lcd.setCursor(0,1);
  lcd.print("waiting time");
  if(press_D() == 1) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("New(sec):");
    lcd.setCursor(0,1);
    lcd.print("Previous:");
    lcd.setCursor(9,1);
    lcd.print(previous_waiting_period / 1000);
    int pos = 9;
    char c = pressed_key();
    while(c != 'D' && pos < 14) {
      if(is_number(c) == 1) {
        int number = c - '0';
        waiting_period = waiting_period * 10 + number;
        lcd.setCursor(pos,0);
        lcd.print(c);
        pos++;
      }
      c = pressed_key();
    }
    waiting_period = waiting_period * 1000;
  }

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Watering:");
  lcd.setCursor(9,0);
  lcd.print(watering_period/1000);
  lcd.setCursor(0,1);
  lcd.print("Waiting:");
  lcd.setCursor(8,1);
  lcd.print(waiting_period/1000);
  delay(5000);
}

bool is_number(char c) {
  if(c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9') {
    return 1;
  }
  return 0;
}

char pressed_key() {
  char key = keypad.getKey();

  while(key == NULL) {
    key = keypad.getKey();
  }
  return key;
}

char press_key_not_D() {
  char key = keypad.getKey();

  while(key == NULL) {
    key = keypad.getKey();
    if(key == 'D') {
      key = NULL;
    }
  }
  return key;
}

void print_humidity () {
  if (SoilMoisturePercentage >= 100)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Soil humidity:");
    lcd.setCursor(0,1);
    lcd.print(SoilMoisturePercentage);
    lcd.setCursor(3,1);
    lcd.print("%");
  }
  else if (SoilMoisturePercentage <= 0)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Soil humidity:");
    lcd.setCursor(0,1);
    lcd.print(SoilMoisturePercentage);
    lcd.setCursor(1,1);
    lcd.print("%");
  }
  else if (SoilMoisturePercentage > 0 && SoilMoisturePercentage < 100)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Soil humidity:");
    lcd.setCursor(0,1);
    lcd.print(SoilMoisturePercentage);
    if(SoilMoisturePercentage < 10) {
      lcd.setCursor(1,1);
      lcd.print("%");
    }
    else {
      lcd.setCursor(2,1);
      lcd.print("%");
    }
  }
}

void sensor_check() {
  MoistureLevel = analogRead(HUMIDITY_SENSOR); 
  SoilMoisturePercentage = map(MoistureLevel, OpenAirReading, WaterReading, 0, 100);
 
  if (SoilMoisturePercentage >= 100)
  {
    SoilMoisturePercentage = 100;
  }
  else if (SoilMoisturePercentage <= 0)
  {
    SoilMoisturePercentage = 0;
  }
}

void reset_millis() {
  noInterrupts();
  timer0_millis=0;
  interrupts();
  previousMillis = millis();
  currentMillis=millis();
}
