#include <Arduino.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <FirebaseArduino.h> // Install the FirebaseArduino library via the Arduino Library Manager

#define FIREBASE_HOST "your-firebase-project.firebaseio.com" 
#define FIREBASE_AUTH "your-firebase-auth-token"

#define TRIGGER_PIN 9
#define ECHO_PIN 10
#define FOOD_SERVO_PIN 12
#define PLATE_SERVO_PIN 11
#define TOUCH_SENSOR_PIN 6
#define IR_SENSOR_PIN 8
#define BUZZER_PIN 7
#define MIN_DISTANCE 10
#define FEEDING_DELAY 2000
#define IR_DETECT_RANGE 30


Servo foodServo;
Servo plateServo;


void dispenseFood() {
  foodServo.write(90);
  delay(1500);
  foodServo.write(0);
}

void cleanPlate() {
  plateServo.write(90);
  delay(3000);
  plateServo.write(0);
  delay(2000);
  plateServo.write(90);
  delay(2000);
  plateServo.write(0);
}

void playHappyBirthday() {
  for (int i = 0; i < 2; i++) {
    tone(BUZZER_PIN, 2000, 200);  // Play a tone of 2000 Hz for 200 ms
    delay(200);                   // Pause for the duration of the beep
    noTone(BUZZER_PIN);           // Stop the buzzer from playing
    delay(200);                   // Pause for the duration between beeps
  }
}

bool detectObject() {
  int irValue = analogRead(IR_SENSOR_PIN);
  if (irValue > IR_DETECT_RANGE) {
    return true;
  } else {
    return false;
  }
}

const char *ssid = "YourWiFiSSID";
const char *password = "YourWiFiPassword";

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(TOUCH_SENSOR_PIN, INPUT);
  pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  foodServo.attach(FOOD_SERVO_PIN);
  plateServo.attach(PLATE_SERVO_PIN);

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); // Initialize Firebase with your project credentials
}

void loop() {
  long duration, distance;
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = (duration / 2) / 29.1;
  Serial.println(distance);

  if (digitalRead(TOUCH_SENSOR_PIN) == HIGH) {
    playHappyBirthday();
    digitalWrite(BUZZER_PIN, HIGH);
  }

  if (distance > 0 && distance < MIN_DISTANCE) {
    cleanPlate();
    delay(FEEDING_DELAY);
    if (detectObject()) {
      dispenseFood();
    } else {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(1000);
      digitalWrite(BUZZER_PIN, LOW);
    }
  }

  // Check Firebase for updates
  if (Firebase.getString("cleanPlate")) {
    cleanPlate();
    Firebase.setString("cleanPlate", "false");
  }

  if (Firebase.getString("feed")) {
    dispenseFood();
    Firebase.setString("feed", "false");
  }
}

