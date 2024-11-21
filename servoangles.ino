#include <WiFiS3.h>
#include <AccelStepper.h>
#include <Servo.h>
#include <queue>

// Define pins
#define BASE_SERVO_PIN 8
#define ARM_BASE_SERVO_PIN 9
#define MID_JOINT_SERVO_PIN 10
#define WHISK_ANGLE_SERVO_PIN 11

#define STEPPER_PIN_1 3
#define STEPPER_PIN_2 4
const int stepPin = 3; 
const int dirPin = 4; 

Servo baseServo;
Servo armBaseServo;
Servo midJointServo;
Servo whiskAngleServo;

// Initialize servo angles
int currentBaseAngle = 90;
int currentArmBaseAngle = 35;
int currentMidJointAngle = 42;
int currentWhiskAngle = 250;

// Stepper motor setup
AccelStepper stepper(AccelStepper::FULL2WIRE, STEPPER_PIN_1, STEPPER_PIN_2);
bool whiskDirection = true;
int whiskTargetPosition = 0;
bool isForwardMotion = true;
unsigned long lastStepperUpdate = 0;

// WiFi setup
WiFiServer server(80);

// Queue and state management
std::queue<int> cupQueue;
bool isWhisking = false;
unsigned long whiskStartTime = 0;
int currentCup = -1;

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

  // Initialize stepper motor
  stepper.setMaxSpeed(40);        // Set maximum speed
  stepper.setAcceleration(10000);   // Set acceleration
  stepper.setCurrentPosition(0);  // Start position       // Set initial position
  int stepsPerDegree = 200 / 360; // Assuming 200 steps per full revolution
  int targetPosition = 20 * stepsPerDegree; // 20-degree move

  // Move to the target position
  stepper.moveTo(targetPosition);
  stepper.move(targetPosition);
  stepper.run();

  // Run the stepper until it reaches the position
//   while (stepper.distanceToGo() != 0) {
//     stepper.run();
//   }
}

void loop() {
  // Handle WiFi client requests
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

  // Process cup queue
  processQueue();

  // Run stepper motor for whisking
  if (isWhisking) {
    performWhisking();
  }
}

void initializeServos() {
  Serial.println("Initializing servos...");
  baseServo.write(currentBaseAngle);
  armBaseServo.write(0); //35
 // moveServo(armBaseServo, currentBaseAngle, targetBaseAngle, 20);
  midJointServo.write(0);
  whiskAngleServo.write(0);
  delay(1000);

//attempt 1
  // armBaseServo.write(4);
  // delay(1000);
  // midJointServo.write(30);
  // delay(1000);
  // //whiskAngleServo.write(260);
  // delay(1000);

//attempt 2
// armBaseServo.write(15);
// armBaseServo.write(30);
// armBaseServo.write(40);
// delay(1000);

// midJointServo.write(50);
// delay(1000);
// whiskAngleServo.write(260);
// delay(1000);

// midJointServo.write(35);
// delay(1000);
// armBaseServo.write(45);
// delay(1000);
// // midJointServo.write(32);
// whiskAngleServo.write(272);

//attempt 3
armBaseServo.write(40);
delay(1000);
midJointServo.write(90);
delay(1000);
whiskAngleServo.write(260);









// raise arm by 45

// raise midjoint by 90

// raise arm to 90

// drop whisk

// drop arm
}

int parseCupLocation(String request) {
  int cupIndex = request.indexOf("cup=");
  if (cupIndex >= 0) {
    return request.substring(cupIndex + 4).toInt();
  }
  return -1;
}

void performWhisking() {
  unsigned int smallRangeSteps = 25; // Define a small number of steps for limited motion
  unsigned long currentTime = millis();

  // Check if enough time has passed to toggle the stepper's position
  if (currentTime - lastStepperUpdate >= 100) { // Adjust timing as needed for smoother motion
    lastStepperUpdate = currentTime;

    if (isForwardMotion) {
      // Move forward
      digitalWrite(dirPin, HIGH); // Set direction to forward
      for (int x = 0; x < smallRangeSteps; x++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(500); // Adjust speed (faster)
        digitalWrite(stepPin, LOW);
        delayMicroseconds(500);
      }
      Serial.println("Stepper moved forward");
    } else {
      // Move backward
      digitalWrite(dirPin, LOW); // Set direction to backward
      for (int x = 0; x < smallRangeSteps; x++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(500); // Adjust speed (faster)
        digitalWrite(stepPin, LOW);
        delayMicroseconds(500);
      }
      Serial.println("Stepper moved backward");
    }

    // Toggle direction
    isForwardMotion = !isForwardMotion;
  }
}


// void performWhisking() {
//   if (stepper.distanceToGo() == 0) { // Check if stepper reached the target
//     // Toggle direction and set next target position
//     whiskDirection = !whiskDirection;
//     whiskTargetPosition = whiskDirection ? 20 : -20;
//     stepper.moveTo(whiskTargetPosition);
//     Serial.print("Changing direction: ");
//     Serial.println(whiskTargetPosition);
//   }

//   // Smooth motion logic
//   stepper.run(); // Run the stepper with acceleration profile

//   // Debugging info
//   Serial.print("Current Position: ");
//   Serial.println(stepper.currentPosition());
// }

void processQueue() {
  if (!isWhisking && !cupQueue.empty()) {
    currentCup = cupQueue.front();
    int targetBaseAngle = (currentCup == 1) ? 90 : 5;
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
