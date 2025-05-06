#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>
#include <WiFi.h>
#include <time.h>
#include <ESP32Servo.h>
#include <math.h>
#include <PubSubClient.h>


// Declare a Servo object
Servo servo;


#define NTP_SERVER  "pool.ntp.org"
#define UTC_OFFSET  0
#define UTC_OFFSET_DST 0

#define SCREEN_WIDTH 123
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define BUZZER 5
#define LED_1 15
#define LED_2 2
#define PB_CANCEL 34
#define PB_UP 35
#define PB_DOWN 32
#define PB_OK 33
#define DHT 12
#define LDR_PIN 36
#define SERVO_PIN 16

#define UP 1
#define DOWN 2
#define CANCEL 3
#define OK 4

//MQTT Broker settings
#define MQTT_BROKER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_PASSWORD ""

#define MQTT_PUBLISH_TOPIC "medibox/data"
#define MQTT_SUBSCRIBE_TOPIC "medibox/commands"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHTesp dhtSensor;

//Global Variable
char weekday[8];
int days = 0;
int hours = 0;
int minutes = 0;
int seconds = 0;

int offset_days = 0;
int offset_hour = 0;
int offset_min = 0;
int zone_offset = 0;

bool alarm_enabled[] = {false,false};
int n_alarms = 2;
int alarm_hours[] = {0, 14};
int alarm_minutes[] = {1, 52};
bool alarm_triggered[] = {false, false};
int ringing_alarm;
bool alarm_snoozed = false;
int snoozed_alarm = 0;
int alarm_count = 0;

int n_notes = 8;
int C = 262;
int D = 294;
int E = 330;
int F = 349;
int G = 392;
int A = 440;
int B = 494;
int C_H = 523;
int notes[] = {C, D, E, F, G, A, B, C_H};

int current_mode = 0;
int max_modes = 5;
String options[] = {"1-Set Time", "2-Set Alarm 1", "3-Set Alarm 2","4-Show Alarms", "5-Remove Alarm"};


float temp = 0;
float humd = 0;

int ldr_sum = 0;
int sampling_count = 0;
float light_intensity = 0.0;

float theta_offset = 30.0;
float gamma = 0.75;
float T_med = 30.0;

// MQTT client initialization
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void print_line(String text, int column, int row, int text_size);
void update_time_with_check_alarm();
void check_temp();
void go_to_menu();
void run_mode(int mode);
void set_time();
int wait_for_button_press();
void set_alarm(int alarm_number);
void delete_alarm();
void ring_alarm(int ringing_alarm);
void update_time();
void print_time_now(void);
void show_alarms();
void snooze_alarm();
void get_ldr_value(int seconds);
void change_servo_angle(float intensity, float temperature,float ts,float tu);

void setup() {
  // put your setup code here, to run once:
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(PB_CANCEL, INPUT);
  pinMode(PB_UP, INPUT);
  pinMode(PB_DOWN, INPUT);
  pinMode(PB_OK, INPUT);
  pinMode(LDR_PIN, INPUT);
  
  // Attach the servo to the specified pin
  servo.attach(SERVO_PIN);
 
  // ledcSetup(0, 5000, 8); 
  // ledcAttachPin(BUZZER, 0);
  
  // //pwm channel for servo motor
  // ledcSetup(1, 50, 16); // 50Hz frequency, 8-bit resolution
  // ledcAttachPin(SERVO_PIN, 1); // Attach the servo pin to PWM channel 1

  Serial.begin(9600);

  dhtSensor.setup(DHT, DHTesp::DHT22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.display();
  delay(2000);

  display.clearDisplay();
  print_line("Welcome to Medibox", 10, 20, 2);
  delay(3000);
  display.clearDisplay();

  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    display.clearDisplay();
    print_line("Connecting to WiFi", 2, 0, 0);

  }

  display.clearDisplay();
  print_line("Connected to wiFi",2,0,0);

  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
  delay(200);
  display.clearDisplay();
}


//main loop
void loop() {
  update_time_with_check_alarm();
 
  if (digitalRead(PB_OK) == LOW) {
    delay(200);
    go_to_menu();
  }

  check_temp();
  print_line("Tem:" + String(int(temp)) +" Hum:"+ String(int(humd)) , 0, 50, 1);
  get_ldr_value(seconds);
 
  delay(200);
  
}

//Print the text on the screen
void print_line(String text, int column, int row, int text_size) {

  display.setTextSize(text_size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(column, row);
  display.println(text);

  display.display();

}

//Print the time for the user
void print_time_now(void) {

  display.clearDisplay();

// Print weekday and day
print_line(weekday, 45, 0, 1);
print_line(":", 65, 0, 1);
print_line((days < 10 ? "0" : "") + String(days), 75, 0, 1);

// Print hours, minutes, and seconds with leading zeros
print_line((hours < 10 ? "0" : "") + String(hours), 25, 10, 2);
print_line(":", 45, 10, 2);
print_line((minutes < 10 ? "0" : "") + String(minutes), 55, 10, 2);
print_line(":", 75, 10, 2);
print_line((seconds < 10 ? "0" : "") + String(seconds), 85, 10, 2);

}

//Update the time for the user
void update_time(void) {
  struct tm timeinfo;
  getLocalTime(&timeinfo);

  char day_str[8];
  char hour_str[8];
  char min_str[8];
  char sec_str[8];

  strftime(day_str, sizeof(day_str), "%d", &timeinfo);
  strftime(sec_str, sizeof(sec_str), "%S", &timeinfo);
  strftime(hour_str, sizeof(hour_str), "%H", &timeinfo);
  strftime(min_str, sizeof(min_str), "%M", &timeinfo);
  strftime(weekday, sizeof(weekday), "%a", &timeinfo); // Store weekday globally

  hours = atoi(hour_str);
  minutes = atoi(min_str);
  days = atoi(day_str);
  seconds = atoi(sec_str);
}


    


//Ring the alarm for the user
void ring_alarm(int ringing_alarm) {

  display.clearDisplay();
  alarm_snoozed = false;
  alarm_count = 0;
  bool alarm_happened = false;

  print_line("MEDICINE TIME!", 0, 0, 2);
  digitalWrite(LED_1, HIGH);
  
  //Ring Buzzer
  while (alarm_happened == false && digitalRead(PB_CANCEL) == HIGH ) {
    for (int i = 0; i < n_notes; i++) {
      if(digitalRead(PB_CANCEL)==LOW){
        delay(300);
        alarm_happened = true;
        alarm_triggered[ringing_alarm] = true;
        alarm_count = 0;
        alarm_minutes[ringing_alarm] -= 5*snoozed_alarm;
        if(alarm_minutes[ringing_alarm]<0){
          alarm_minutes[ringing_alarm] += 60;
          alarm_hours[ringing_alarm] -= 1;
          if(alarm_hours[ringing_alarm]<0){
            alarm_hours[ringing_alarm] += 24;
          }
        }
        snoozed_alarm = 0;

      }
      if(snoozed_alarm<=3){
        if(alarm_count>=5){
          delay(2000);
          alarm_happened = false;
          alarm_triggered[ringing_alarm] = false;
          alarm_minutes[ringing_alarm] += 5;
          alarm_snoozed = true;

          if(alarm_minutes[ringing_alarm]>=60){
            alarm_minutes[ringing_alarm] -= 60;
            alarm_hours[ringing_alarm] += 1;
            if(alarm_hours[ringing_alarm]>=24){
              alarm_hours[ringing_alarm] -= 24;
            }
          }
          snoozed_alarm +=1;
          if (snoozed_alarm == 3){
            alarm_triggered[ringing_alarm] = true;
            alarm_snoozed = false;
            alarm_count = 0;
            alarm_minutes[ringing_alarm] -= 5*snoozed_alarm;
            if(alarm_minutes[ringing_alarm]<0){
              alarm_minutes[ringing_alarm] += 60;
              alarm_hours[ringing_alarm] -= 1;
              if(alarm_hours[ringing_alarm]<0){
                alarm_hours[ringing_alarm] += 24;
              }
            }
            snoozed_alarm = 0;
            break;
            
          }
        break;
        
        }
      }
      
      tone(0, notes[i]); // Channel 0, frequency from notes array
      delay(500);
      noTone(0); // Stop tone
      delay(2);

    }
    alarm_count++;
    if (alarm_snoozed == true){
      break;
    }

  }

  digitalWrite(LED_1, LOW);
  display.clearDisplay();

}

//Update the time and check if the alarm is triggered
void update_time_with_check_alarm(void) {
  update_time();
  print_time_now();
  
  if (hours == 0 && minutes == 0) { // Reset alarms at midnight
    for (int i = 0; i < n_alarms; i++) {
      alarm_triggered[i] = false;
    }
  }
  
  for (int i = 0; i < n_alarms; i++) {
      if (alarm_enabled[i]==true && alarm_triggered[i] == false && alarm_hours[i] == hours && alarm_minutes[i] == minutes) {
        ringing_alarm = i;
        ring_alarm(ringing_alarm);
        
      }
    }
  }

//Wait for the user to press a button
int wait_for_button_press() {
  while (true) {
    if (digitalRead(PB_UP) == LOW) {
      delay(200);
      return UP;
    }

    else if (digitalRead(PB_DOWN) == LOW) {
      delay(200);
      return DOWN;
    }

    else if (digitalRead(PB_CANCEL) == LOW) {
      delay(200);
      return CANCEL;
    }

    else if (digitalRead(PB_OK) == LOW) {
      delay(200);
      return OK;
    }

    update_time();
  }
}

//Go to the menu
void go_to_menu() {
  while (digitalRead(PB_CANCEL) == HIGH) {
    display.clearDisplay();
    print_line(options[current_mode], 2, 0, 2);

    int pressed = wait_for_button_press();

    if (pressed == UP) {
      current_mode += 1;
      current_mode %= max_modes;
      print_line(options[current_mode], 2, 0, 2);
      delay(200);
    }

    else if (pressed == DOWN) {
      current_mode -= 1;
      if (current_mode < 0) {
        current_mode = max_modes - 1;
        print_line(options[current_mode], 2, 0, 2);
        delay(200);
      }
    }

    else if (pressed == OK) {
      Serial.println(current_mode);
      delay(200);
      run_mode(current_mode);
    }
  }
}

//Run the mode selected by the user
void run_mode(int mode) {
  if (mode == 0) {
    set_time();
  }
  else if (mode == 1 ) {
    set_alarm(1);
  }
  else if (mode == 2) {
    set_alarm(2);
  }
  else if (mode ==3){
    show_alarms();
  }
  else if (mode == 4) {
    delete_alarm();
  }
}

//Set the time for the user
void set_time() {
  int sign = 1;  // Default to positive offset
  int temp_hour = 0;
  int temp_minute = 0;

  // Select + or - for offset
  while (true) {
    display.clearDisplay();
    print_line("Select Offset Type:", 0, 0, 1);
    print_line((sign == 1) ? "+ (UP)" : "- (DOWN)", 0, 20, 2);
    
    int pressed = wait_for_button_press();
    if (pressed == UP || pressed == DOWN) {
      sign = (pressed == UP) ? 1 : -1;
      delay(200);
    } 
    else if (pressed == OK) {
      delay(200);
      break;
    }
    else if (pressed == CANCEL) {
      delay(200);
      break;
    }
  }

  // Enter Offset Hours
  while (true) {
    display.clearDisplay();
    print_line("Enter Offset Hour:", 0, 0, 1);
    print_line(String(temp_hour), 0, 20, 2);

    int pressed = wait_for_button_press();
    if (pressed == UP) {
        delay(200);
        temp_hour = (temp_hour + 1) % 13;  // Limit within 0-12
    } 
    else if (pressed == DOWN) {
        delay(200);
        temp_hour = (temp_hour - 1 < 0) ? 12 : temp_hour - 1;
    } 
    else if (pressed == OK) {
        delay(200);
        break;
    }
    else if (pressed == CANCEL) {
      delay(200);
      break;
    }
}

  // Enter Offset Minutes
  while (true) {
    display.clearDisplay();
    print_line("Enter Offset Minute:", 0, 0, 1);
    print_line( String(temp_minute), 0, 20, 2);

    int pressed = wait_for_button_press();
    if (pressed == UP) {
      delay(200);
      temp_minute = (temp_minute + 1) % 60;  // Limit within 0-59
    } 
    else if (pressed == DOWN) {
      delay(200);
      temp_minute = (temp_minute - 1 < 0) ? 59 : temp_minute - 1;
    } 
    else if (pressed == OK) {
      delay(200);
      offset_hour = sign * temp_hour;
      offset_min = sign * temp_minute;
      zone_offset = UTC_OFFSET + offset_hour * 3600 + offset_min * 60;
      configTime(zone_offset, UTC_OFFSET_DST, NTP_SERVER);
      Serial.println("Time offset set to " + String(offset_hour) + "h " + String(offset_min) + "m");
  
      break;
    }
    else if (pressed == CANCEL) {
      delay(200);
      break;
    }
  }

  // Apply the offset
  
  display.clearDisplay();
  print_line("Time Offset Set", 0, 0, 2);
  delay(1000);
}


//Set the alarm for the user 
void set_alarm(int alarm_number) {
  int temp_hour = alarm_hours[alarm_number - 1];
  while (true) {
    display.clearDisplay();
    print_line("Enter hour: " + String(temp_hour), 0, 0, 2);

    int pressed = wait_for_button_press();
    if (pressed == UP) {
      delay(200);
      temp_hour += 1;
      temp_hour = temp_hour % 24;
    }

    else if (pressed == DOWN) {
      delay(200);
      temp_hour -= 1;
      if (temp_hour < 0) {
        temp_hour = 23;
      }
    }

    else if (pressed == OK) {
      delay(200);
      alarm_hours[alarm_number-1]= temp_hour;
      break;
    }

    else if (pressed == CANCEL) {
      delay(200);
      break;
    }
  }

  int temp_minute = alarm_minutes[alarm_number - 1];
  while (true) {
    display.clearDisplay();
    print_line("Enter minute: " + String(temp_minute), 0, 0, 2);

    int pressed = wait_for_button_press();
    if (pressed == UP) {
      delay(200);
      temp_minute += 1;
      temp_minute = temp_minute % 60;
    }

    else if (pressed == DOWN) {
      delay(200);
      temp_minute -= 1;
      if (temp_minute < 0) {
        temp_minute = 59;
      }
    }

    else if (pressed == OK) {
      delay(200);
      alarm_minutes[alarm_number-1] = temp_minute;
      Serial.println("Alarm set for " + String(temp_hour) + ":" + String(temp_minute));
      alarm_triggered[alarm_number-1] = false;
      alarm_enabled[alarm_number-1] = true;
      break;
    }

    else if (pressed == CANCEL) {
      delay(200);
      break;
    }
  }

  display.clearDisplay();
  print_line("Time is set", 0, 0, 2);
  delay(1000);
}

//Show all alarms set by the user 
void show_alarms()
{
  display.clearDisplay();
  print_line("Alarm " + String(1), 0, 0, 1);
  print_line("Time: " + String(alarm_hours[0]) + " : " + String(alarm_minutes[0]), 0, 10, 1);
  
  if (alarm_enabled[0]) {
    print_line("Enabled", 0, 20, 1);
  }
  else {
    print_line("Disabled", 0, 20, 1);
    }

  print_line("Alarm " + String(2), 0, 30, 1);
  print_line("Time: " + String(alarm_hours[1]) + " : " + String(alarm_minutes[1]), 0, 40, 1);

  if (alarm_enabled[1]) {
    print_line("Enabled", 0, 50, 1);
  }
  else {
    print_line("Disabled", 0, 50, 1);
  }

  delay(8000);
}



//Delete the alarm set by the user
void delete_alarm() {
  int alarm_number = 1;
  while (true) {
    display.clearDisplay();
    print_line("Delete alarm " + String(alarm_number), 0, 0, 2);

    int pressed = wait_for_button_press();
    if (pressed == UP) {
      delay(200);
      alarm_number += 1;
      if(alarm_number > 2) {
        alarm_number = 1;
      }
    }

    else if (pressed == DOWN) {
      delay(200);
      alarm_number -= 1;
      if (alarm_number < 1) {
        alarm_number = 2;
      }
    }

    else if (pressed == OK) {
      delay(200);
      alarm_hours[alarm_number - 1] = 0;
      alarm_minutes[alarm_number - 1] = 0;
      alarm_enabled[alarm_number - 1] = false;
      alarm_triggered[alarm_number - 1] = false;
      break;
    }

    else if (pressed == CANCEL) {
      delay(200);
      break;
    }
  }

  display.clearDisplay();
  print_line("Alarm deleted", 0, 0, 2);
  delay(1000);
}

//Check the temperature and humidity of the environment
void check_temp(void) {
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  
  temp = data.temperature;
  humd = data.humidity;

  bool all_good = true;
  if (data.temperature > 32) {
    all_good = false;
    digitalWrite(LED_2, HIGH);
    print_line("TEMP_HIGH", 1, 30, 0);
  }

  else if (data.temperature < 24) {
    all_good = false;
    digitalWrite(LED_2, HIGH);
    print_line("TEMP LOW", 1, 30, 0);
  }

  if (data.humidity > 85) {
    all_good = false;
    digitalWrite(LED_2, HIGH);
    print_line("HUMD HIGH", 1, 40, 0);
  }

  else if (data.humidity < 65) {
    all_good = false;
    digitalWrite(LED_2, HIGH);
    print_line("HUMD LOW", 1, 40, 0);
  }

  if (all_good) {
    digitalWrite(LED_2, LOW);
  }
}

// Get LDR value in 5 seconds samples and average it
void get_ldr_value(int seconds){
  
  int tu = 24; //change after mqtt 
  int ts = 5;

  static unsigned long lastSampleTime = 0; // Tracks the last sample time
  const unsigned long sampleInterval = ts*1000; // 5 seconds in milliseconds

  if (millis() - lastSampleTime >= sampleInterval) {

    lastSampleTime = millis(); // Update the last sample time

    int ldr_val = analogRead(LDR_PIN);
    ldr_sum += ldr_val;
    sampling_count += 1;
    Serial.println("LDR Value: " + String(ldr_val)+ " Sampling Count: " + String(sampling_count)+"seconds"+ String(seconds));
  }

  if(sampling_count==tu){
    int average = ldr_sum / tu;
    ldr_sum = 0;
    sampling_count = 0;

     // Map the average LDR value to a range of 0 to 1
    light_intensity = float(average) / 4095.0; // Normalize to 0-1 range
    Serial.println("Average LDR Value: " + String(average) + " | Light Intensity: " + String(light_intensity));
  }

}

void change_servo_angle(float intensity, float temperature,float ts,float tu) {
   // Prevent division by zero or log(0)
   if (tu <= 0 || ts <= 0 || T_med <= 0) return;

   float log_ratio = log(ts / tu);
   float angle = theta_offset + (180.0 - theta_offset) * intensity * gamma * log_ratio * (temperature / T_med);
 
   // Clamp angle to valid range for servo
   angle = constrain(angle, 0.0, 180.0);
 
   // Move servo to calculated angle
   servo.write((int)angle);

}


