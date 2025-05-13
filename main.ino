#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>

// WiFi credentials
const char* ssid = "Redmi 9i";
const char* password = "9650474649";

// Server endpoint
const char* serverUrl = "https://iot-project-fk1c.onrender.com/api/arduino/data";

// HC-SR04 Pins
const int trigPin = 5;
const int echoPin = 18;

// Button Pin
const int buttonPin = 4;

// LCD setup (I2C Address, columns, rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variables
int currentStep = 0;
float length = 0, breadth = 0, height = 0;
bool buttonPressed = false;

void setup() {
  Serial.begin(115200);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  lcd.clear();
  lcd.print("WiFi Connected!");
  delay(2000);
  lcd.clear();
  lcd.print("Measuring Length");
}

float measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);  // Timeout to avoid hangs
  float distance = duration * 0.034 / 2;
  return distance;
}

void loop() {
  if (digitalRead(buttonPin) == LOW && !buttonPressed) {
    buttonPressed = true;
    currentStep++;

    lcd.clear();
    float measured = measureDistance();
    delay(500);  // Allow LCD time to update

    switch (currentStep) {
      case 1:
        length = 37.64 - measured ;
        lcd.print("Length: ");
        lcd.print(length);
        lcd.print("cm");
        delay(5000);
        lcd.clear();
        lcd.print("Measuring Width");
        break;

      case 2:
        breadth = 37.64 - measured;
        lcd.print("Width: ");
        lcd.print(breadth);
        lcd.print("cm");
        delay(5000);
        lcd.clear();
        lcd.print("Measuring Height");
        break;

      case 3:
        height = 37.64 - measured ;
        lcd.print("Height: ");
        lcd.print(height);
        lcd.print("cm");
        delay(5000);

        // Send all three values to server
        sendDataToServer(length, breadth, height);

        // Reset for next cycle
        currentStep = 0;
        delay(3000);
        lcd.clear();
        lcd.print("Ready for next");
        delay(5000);
        break;
    }

    delay(1000);  // Debounce delay
  }

  if (digitalRead(buttonPin) == HIGH) {
    buttonPressed = false;
  }
}

void sendDataToServer(float l, float b, float h) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    int volume = l*b*h;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    String jsonPayload = "{\"length\": " + String(l) + ", \"breadth\": " + String(b) + ", \"height\": " + String(h) + ", \"volume\": " + String(volume) + "}";

    int httpResponseCode = http.POST(jsonPayload);

    lcd.clear();
    if (httpResponseCode > 0) {
      lcd.print("Data Sent!");
    } else {
      lcd.print("Error Sending!");
    }

    http.end();
  } else {
    lcd.clear();
    lcd.print("WiFi Lost!");
  }
}

