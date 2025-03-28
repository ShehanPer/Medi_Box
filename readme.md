# Medi_Box

## Project Overview
The Medi_Box is an embedded system designed to assist users in managing their medication schedules. It features an alarm system to notify users when it's time to take their medicine, displays real-time environmental data (temperature and humidity), and provides an intuitive interface for setting alarms and viewing system information.

This project is implemented using an ESP32 microcontroller, an OLED display, a DHT22 sensor for temperature and humidity, and a buzzer for alarm notifications.

## Features

### Real-Time Clock Synchronization:
- Synchronizes time with an NTP server (pool.ntp.org).
- Displays the current time, date, and day of the week.

### Alarm System:
- Supports up to 2 alarms.
- Allows users to set, view, and delete alarms.
- Includes a snooze feature for alarms.

### Environmental Monitoring:
- Measures and displays temperature and humidity using the DHT22 sensor.
- Alerts the user if temperature or humidity exceeds predefined thresholds.

### User Interface:
- Buttons for navigation:
  - **PB_UP**: Navigate up.
  - **PB_DOWN**: Navigate down.
  - **PB_OK**: Confirm selection.
  - **PB_CANCEL**: Cancel or stop an alarm.
- OLED display for user interaction and system information.

## Hardware Requirements
- **ESP32 Microcontroller**
- **128x64 OLED Display (SSD1306)**
- **DHT22 Temperature and Humidity Sensor**
- **Buzzer**
- **Push Buttons:**
  - PB_UP, PB_DOWN, PB_OK, PB_CANCEL
- **LED Indicators:**
  - **LED_1**: Alarm indicator.
  - **LED_2**: Environmental alert indicator.

## Software Requirements
- **Arduino IDE** (or any compatible ESP32 development environment)
- **Required Libraries:**
  - Adafruit_GFX
  - Adafruit_SSD1306
  - DHTesp
  - WiFi

## Setup Instructions

### Hardware Connections:
- Connect the OLED display to the ESP32 using I2C (SDA and SCL pins).
- Connect the DHT22 sensor to a GPIO pin on the ESP32.
- Connect the buzzer and LEDs to their respective GPIO pins.
- Connect the push buttons to GPIO pins with pull-up resistors.

### Software Configuration:
1. Install the required libraries in the Arduino IDE.
2. Update the Wi-Fi credentials in the `setup()` function.
3. Upload the code to the ESP32 using the Arduino IDE.

### Running the System:
1. Power on the ESP32.
2. The system will display a welcome message and connect to Wi-Fi.
3. Once connected, the system will synchronize time with the NTP server and start functioning.

## How to Use

### Set Time:
- Navigate to the "Set Time" option in the menu.
- Use the **UP** and **DOWN** buttons to adjust the time offset.

### Set Alarms:
- Navigate to "Set Alarm 1" or "Set Alarm 2" in the menu.
- Use the **UP** and **DOWN** buttons to set the hour and minute.
- Confirm with the **OK** button.

### View Alarms:
- Navigate to "Show Alarms" in the menu to view the current alarm settings.

### Delete Alarms:
- Navigate to "Remove Alarm" in the menu.
- Select the alarm to delete using the **UP** and **DOWN** buttons.

### Environmental Monitoring:
- The system continuously monitors temperature and humidity.
- Alerts are displayed on the OLED if thresholds are exceeded.

### Alarm Notifications:
- When an alarm is triggered, the buzzer sounds, and **LED_1** lights up.
- Press the **CANCEL** button to stop the alarm or **OK** to snooze it.

## Code Structure

### Global Variables:
- Stores system states, alarm configurations, and environmental data.

### Functions:
- `setup()`: Initializes hardware and connects to Wi-Fi.
- `loop()`: Main program loop for updating time, alarms, and environmental data.
- `update_time()`: Synchronizes and updates the system clock.
- `ring_alarm()`: Handles alarm notifications and snooze functionality.
- `check_temp()`: Monitors temperature and humidity.
- `go_to_menu()`: Handles user navigation through the menu.
- `set_alarm()`: Allows users to configure alarms.
- `delete_alarm()`: Deletes an alarm.
- `show_alarms()`: Displays the current alarm settings.

## Pin Configuration
| Component    | Pin    |
|-------------|--------|
| OLED SDA    | GPIO 21 |
| OLED SCL    | GPIO 22 |
| DHT22 Data  | GPIO 12 |
| Buzzer      | GPIO 5  |
| LED_1       | GPIO 15 |
| LED_2       | GPIO 2  |
| PB_CANCEL   | GPIO 34 |
| PB_UP       | GPIO 35 |
| PB_DOWN     | GPIO 32 |
| PB_OK       | GPIO 33 |

## Known Issues

### Wi-Fi Connection:
- Ensure the correct **SSID** and **password** are provided.
- The system may take a few seconds to connect to Wi-Fi.

### NTP Synchronization:
- If the time is not synchronized, check the internet connection and NTP server availability.

### OLED Flickering:
- Avoid using `clearDisplay()` frequently to prevent flickering.

## Future Improvements
- Add support for more alarms.
- Implement a graphical user interface for better interaction.
- Add battery backup for offline operation.
- Integrate additional sensors for enhanced monitoring.

## License
This project is open-source and can be modified or redistributed under the terms of the **MIT License**.

## Contributors
**Your Name**: Developer and Maintainer

Feel free to contribute to this project by submitting pull requests or reporting issues!

