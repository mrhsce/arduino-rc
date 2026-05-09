
# arduino-rc

arduino-rc is a simple remote-control car project that uses an ESP8266-based board (WeMos D1 / NodeMCU) as a Wi‑Fi access point to receive UDP control packets from an Android controller app, forwards those commands to an Arduino Uno over a serial link, and the Uno drives an L298N motor driver to control the motors.

This README documents what the project contains, required hardware, wiring, and detailed setup and build instructions for both Arduino and Android parts.

Contents of this repository
- /arduino/Uno_controller: Arduino Uno sketch (Uno_controller.ino) — command queue and motor control logic for the L298N driver.
- /arduino/WiFiAccessPoint: ESP8266 (WeMos D1) sketch (WiFiAccessPoint.ino) — creates a Wi‑Fi Access Point and listens for UDP packets, then forwards valid commands to the Uno via SoftwareSerial.
- /android/RC_Controller2: Android Studio project — a minimal app with two virtual joysticks that sends UDP commands to the ESP8266 AP.
- /fritzig parts: Fritzing part files (for reference)
- README.md: (this file)

High-level behavior
- Android app: two on-screen joysticks. Every 100 ms when a joystick is active the app sends a UDP packet with the payload in the format: X{speed}Y{direction}
  - Example: "X3Y-1"
  - The Android app targets IP 192.168.4.1 (the ESP8266 softAP default) and UDP port 4320.
- ESP8266 (WiFiAccessPoint): creates AP (SSID/password configurable in the sketch), listens on UDP port 4320, validates packets containing both 'X' and 'Y', forwards the message payload as a newline-terminated string to the Uno via SoftwareSerial.
- Arduino Uno (Uno_controller): receives command strings via SoftwareSerial, parses X and Y values, enqueues each command for a configurable time slice (default 100 ms), and sets motor outputs via an L298N driver. The Uno sketch implements a simple queue manager, serial manager and actuator manager.

Hardware required (suggested)
- Arduino Uno (or compatible)
- ESP8266 board (WeMos D1 or NodeMCU)
- L298N motor driver (or another H-bridge; pin mapping in the sketch assumes L298N)
- Two DC motors and motor power supply (battery pack) — original project notes mention ~10V for motors
- Jumper wires and a breadboard
- Two resistors to build a voltage divider (2.2kΩ and 1kΩ were used in the original project) to safely connect the Uno TX (5V) to the ESP8266 RX (3.3V)
- USB cables to flash the boards

Important wiring notes
- Logic level directions
  - ESP8266 (WeMos D1) is a 3.3V device. Its TX (serial transmit) is 3.3V and is read fine by the Uno RX (5V device). However, the Uno TX is 5V and must not be tied directly to ESP8266 RX: use a level shifter or a simple resistor voltage divider.
  - The repository author used a divider made from 2.2kΩ and 1kΩ to reduce 5V -> ~3.3V.

- Serial wiring (software serial pins used by the sketches)
  - Uno sketch defines: RX = 10, TX = 11 (SoftwareSerial on the Uno)
  - ESP8266 sketch defines: RX = 4, TX = 5 (SoftwareSerial on the ESP8266)
  - Connect as follows (logical):
    - ESP TX -> UNO RX (UNO pin 10)
    - UNO TX (UNO pin 11) -> voltage divider -> ESP RX

- Motor driver pins (as defined in Uno_controller.ino)
  - enA -> Uno D9 (PWM)  (enable motor A)
  - inA1 -> Uno D6 (direction)
  - inA2 -> Uno D7 (direction)
  - enB -> Uno D5 (PWM)  (enable motor B)
  - inB1 -> Uno D3 (direction)
  - inB2 -> Uno D4 (direction)

- Power
  - Motor power should be provided to the L298N motor power input (commonly VIN for motors); the logic supply (5V) for the Uno must be stable and the ESP8266 powered from 3.3V regulator or USB.
  - Do NOT power the ESP8266 directly from the Uno 3.3V pin if the ESP8266 draws more current than the regulator can supply — use a proper 3.3V regulator or power the ESP board from USB.

Software setup

Arduino (ESP8266 and Uno)
1. Install the Arduino IDE (if you don't have it already).
2. Install the ESP8266 board package in the Arduino Boards Manager (for WeMos D1/NodeMCU). Instructions: https://arduino-esp8266.readthedocs.io/en/latest/installing.html
3. Open `arduino/WiFiAccessPoint/WiFiAccessPoint.ino` in the Arduino IDE.
   - Optional: Edit the SSID/password on lines where `ssid` and `password` are defined.
   - Note: UDP port is 4320 by default.
4. Select the correct ESP8266 board and upload the sketch to the WeMos D1.
5. Connect the WeMos TX pin to the Uno RX (software serial RX pin 10). Connect the Uno TX (software serial TX pin 11) to the WeMos RX through a voltage divider (2.2kΩ + 1kΩ) or a level shifter.
6. Open `arduino/Uno_controller/Uno_controller.ino` and upload it to the Arduino Uno.

Android app
1. Open Android Studio and import the project at `android/RC_Controller2`.
2. The app uses the virtual joystick library `io.github.controlwear:virtualjoystick:1.9.2`. The library is referenced in the module Gradle file, but if you import the project into a newer Android Studio you may need to update Gradle plugin and dependency coordinates (the project currently uses Android support libraries and older SDK settings).
3. Build & run the app on your Android device (or an emulator with network access — an emulator will not connect to a phone-hosted Wi‑Fi AP easily so test on a real device).
4. The app sends UDP packets to 192.168.4.1:4320. Connect your phone to the ESP8266 AP (SSID/password as configured) and then open the app.

How the command format works
- Commands are ASCII strings like: X{speed}Y{direction}
  - X: forward/back speed (range -5..5 in the original code)
    - Positive values are forward, negative are backward.
  - Y: steering left/right (range -5..5 assumed)
    - Positive values are right, negative are left.
  - Example: X5Y0 -> full forward, no turn. X0Y-3 -> rotate/turn left.

Notes about the code
- `WiFiAccessPoint.ino` (ESP8266)
  - Creates a soft AP with the SSID/password defined in the sketch.
  - Listens on UDP port 4320 and forwards any received message that contains both 'X' and 'Y' to the Uno over SoftwareSerial.
  - Uses SoftwareSerial on pins RX = 4, TX = 5 in the sketch.

- `Uno_controller.ino` (Arduino Uno)
  - Uses SoftwareSerial on pins RX = 10, TX = 11.
  - Parses incoming messages of the form X...Y... and enqueues them for a short execution time slice (`commandTime` is 100 ms by default).
  - Controls an L298N driver using pin assignments shown earlier.

Android app
- `MainActivity.java` uses the virtual joystick widget and reflects its angle/strength into integer speed values.
- It creates a DatagramSocket and sends messages every 100 ms when joystick values are non-zero to 192.168.4.1:4320.

Known issues and suggestions
- The Android project uses older Android support libraries (appcompat v26 beta, etc.). If you import the project into a recent Android Studio, update dependencies to AndroidX or recent support libs and update Gradle plugin/Gradle wrapper as needed.
- The joystick library used is `io.github.controlwear:virtualjoystick:1.9.2`. If you update to AndroidX you may need an AndroidX-compatible replacement or migrate the project with Android Studio's refactor tool.
- The SoftwareSerial implementation on ESP8266 (and on the Uno) can be fragile for high throughputs. This project sends very small low-rate commands so it should be fine.

Testing checklist
1. Flash `WiFiAccessPoint.ino` to the ESP8266. Open Serial Monitor at 9600 baud and verify the AP IP is shown.
2. Connect an Android device to the ESP8266 AP using the SSID/password in the sketch.
3. Flash `Uno_controller.ino` to the Uno, open its Serial Monitor (9600) to observe forwarded messages and debug prints.
4. Run the Android app on your device, operate the joysticks and verify the Uno receives commands and the motors respond.

Security and safety
- Do not operate motors without proper mechanical mounting and an appropriate power source. Ensure wiring is secure and batteries can supply required current.
- Do not connect 5V signals directly to the ESP8266 RX pin; use a level shifter or a divider.

Extending the project
- Add sensor feedback (ultrasonic, encoders) and send telemetry back to the Android app.
- Replace UDP+AP architecture with STA mode + router for longer range or internet-based control.

License
- Existing files include a BSD-style license header in the ESP sketch. The rest of the project has no explicit license file — if you plan to reuse or redistribute, add a LICENSE file with your preferred license.

Contact / Credits
- Original author / repo: (no contact info in repo). Code is simple and intended as a hobbyist educational project.

