#include <WiFiS3.h>
#include <AccelStepper.h>
#include <Servo.h>
#include <queue>

// Define pins
#define BASE_SERVO_PIN 8
#define ARM_BASE_SERVO_PIN 9
#define MID_JOINT_SERVO_PIN 10
#define WHISK_ANGLE_SERVO_PIN 11

#define STEPPER_PIN_1 4
#define STEPPER_PIN_2 5
#define STEPPER_PIN_3 6
#define STEPPER_PIN_4 7

Servo baseServo;
Servo armBaseServo;
Servo midJointServo;
Servo whiskAngleServo;

// Initialize servo angles
int currentBaseAngle = 90;
int currentArmBaseAngle = 45;
int currentMidJointAngle = 32;
int currentWhiskAngle = 180;

AccelStepper stepper(AccelStepper::FULL4WIRE, STEPPER_PIN_1, STEPPER_PIN_2, STEPPER_PIN_3, STEPPER_PIN_4);

WiFiServer server(80);

// Queue and state management
unsigned long whiskStartTime = 0;
bool isWhisking = false;
std::queue<int> cupQueue;
int currentCup = -1;

// Variables for stepper motion
unsigned long lastStepperUpdate = 0;
bool isForwardMotion = true;

void setup() {
  Serial.begin(9600);

  // Start WiFi in Access Point mode
  WiFi.beginAP("MatchaBot_AP", "matcha123");
  server.begin();

  // Attach servos
  baseServo.attach(BASE_SERVO_PIN);
  armBaseServo.attach(ARM_BASE_SERVO_PIN);
  midJointServo.attach(MID_JOINT_SERVO_PIN);
  whiskAngleServo.attach(WHISK_ANGLE_SERVO_PIN);

  initializeServos();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();

    if (request.indexOf("/status") >= 0) {
      sendStatus(client);
    } else {
      int cup = parseCupLocation(request);
      if (cup != -1) {
        cupQueue.push(cup);
        client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nCup added to queue");
      } else {
        client.print("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nInvalid request");
      }
    }
    client.stop();
  }

  processQueue();
  stepper.run();
}

void initializeServos() {
  Serial.println("Initializing servos...");
  baseServo.write(currentBaseAngle);
  armBaseServo.write(currentArmBaseAngle);
  midJointServo.write(currentMidJointAngle);
  whiskAngleServo.write(currentWhiskAngle);
  delay(1000);
}

int parseCupLocation(String request) {
  int cupIndex = request.indexOf("cup=");
  if (cupIndex >= 0) {
    return request.substring(cupIndex + 4).toInt();
  }
  return -1;
}

void processQueue() {
  if (!isWhisking && !cupQueue.empty()) {
    currentCup = cupQueue.front();
    int targetBaseAngle = (currentCup == 1) ? 90 : 15;
    moveServo(baseServo, currentBaseAngle, targetBaseAngle, 20);
    isWhisking = true;
    whiskStartTime = millis();
  }

  if (isWhisking) {
    unsigned long elapsedTime = millis() - whiskStartTime;
    if (elapsedTime < 10000) { // Whisking duration: 10 seconds for testing
      performWhisking();
    } else {
      isWhisking = false;
      cupQueue.pop(); // Remove finished cup
      currentCup = -1;
      moveServo(baseServo, currentBaseAngle, 55, 20); // Move to cleaning position
    }
  }
}

void performWhisking() {
  unsigned long currentTime = millis();
  if (currentTime - lastStepperUpdate >= 1000) { // Update every 1 second
    lastStepperUpdate = currentTime;

    if (isForwardMotion) {
      stepper.moveTo(50); // Move forward
      Serial.println("Stepper moving forward");
    } else {
      stepper.moveTo(-50); // Move backward
      Serial.println("Stepper moving backward");
    }

    isForwardMotion = !isForwardMotion; // Toggle direction
  }
}

void moveServo(Servo &servo, int &currentAngle, int targetAngle, int delayMs) {
  int step = (targetAngle > currentAngle) ? 1 : -1;
  while (currentAngle != targetAngle) {
    currentAngle += step;
    servo.write(currentAngle);
    delay(delayMs);
  }
}

void sendStatus(WiFiClient &client) {
  String response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{";
  response += "\"queue\": [";
  std::queue<int> tempQueue = cupQueue;
  while (!tempQueue.empty()) {
    response += String(tempQueue.front());
    tempQueue.pop();
    if (!tempQueue.empty()) response += ",";
  }
  response += "],";
  response += "\"currentCup\": " + String(currentCup) + ",";
  response += "\"timeLeft\": " + String(isWhisking ? (10000 - (millis() - whiskStartTime)) / 1000 : 0); // 10 seconds for testing
  response += "}";
  client.print(response);
}
