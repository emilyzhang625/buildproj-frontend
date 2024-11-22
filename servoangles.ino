#include <WiFiS3.h>
#include <Adafruit_PWMServoDriver.h>
#include <Stepper.h>
#include <queue>

// Servo constants and configuration
#define BASE_SERVO 0
#define ARM_BASE_SERVO 1
#define MID_JOINT_SERVO 2
#define WHISK_ANGLE_SERVO 3
#define SERVOMIN 125
#define SERVOMAX 625

// Stepper motor configuration
#define STEPPER_PIN_1 4
#define STEPPER_PIN_2 5
const int stepsPerRevolution = 200; // Steps per revolution for the stepper
Stepper myStepper(stepsPerRevolution, STEPPER_PIN_1, STEPPER_PIN_2);

int currentBaseServoAngle = 124;
int currentArmBaseServoAngle = 4;
int currentMidJointServoAngle = 4;
int currentWhiskAngleServoAngle = 9;

Adafruit_PWMServoDriver board1 = Adafruit_PWMServoDriver(0x40);
WiFiServer server(80);

// Queue and state management
std::queue<int> taskQueue; // Holds target positions for cups
unsigned long taskStartTime = 0;
bool isTaskRunning = false;
bool isWashing = false;

void setup() {
  Serial.begin(115200);

  // Start Wi-Fi in Access Point mode
  WiFi.beginAP("MatchaBot_AP", "matcha123");
  Serial.println("WiFi AP started");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();

  // Initialize servo driver and stepper motor
  board1.begin();
  board1.setPWMFreq(60);

  initializeServos();
}

void loop() {
  // Handle incoming client requests
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client connected");
    String request = client.readStringUntil('\r');
    client.flush();

    // Add CORS headers and handle the cup location
    if (request.indexOf("GET /cup=") >= 0) {
      int cupLocation = parseCupLocation(request);
      if (cupLocation != -1) {
        taskQueue.push(cupLocation == 1 ? 80 : 165); // Adjust angles based on cup positions
        taskQueue.push(125);

        client.println("HTTP/1.1 200 OK");
        client.println("Access-Control-Allow-Origin: *");
        client.println("Content-Type: text/plain");
        client.println();
        client.println("Task added to queue");

        startNextTask();
      }
    }
    client.stop();
  }

  // Process tasks if any are running
  if (isTaskRunning) {
    unsigned long elapsedTime = millis() - taskStartTime;

    if (isWashing) {
      if (elapsedTime >= 20000) { // Finish washing after 20 seconds
        finishTask();
      } else {
        performWhisking();
      }
    } else {
      if (elapsedTime >= 30000) { // Finish whisking after 30 seconds
        finishTask();
      } else {
        performWhisking();
      }
    }
  }
}

// Initialize servos to starting positions
void initializeServos() {
  setServoPosition(BASE_SERVO, 125, 180, 40); // extra movemenr here
  delay(100);
  setServoPosition(WHISK_ANGLE_SERVO, 10, 180, 100);
  delay(100);
  setServoPosition(ARM_BASE_SERVO, 5, 180, 250);
  delay(100);
  setServoPosition(MID_JOINT_SERVO, 5, 180, 80); // theres an extra movement here?
  delay(100);
  setServoPosition(ARM_BASE_SERVO, 30, 180, 250);
  delay(100);
  setServoPosition(MID_JOINT_SERVO, 10, 180, 80); // theres an extra movement here?
  delay(100);
  setServoPosition(WHISK_ANGLE_SERVO, 60, 180, 100);
  delay(100);
  setServoPosition(MID_JOINT_SERVO, 26, 180, 40);
  delay(100);
  setServoPosition(WHISK_ANGLE_SERVO, 170, 180, 40);
  delay(100);
  setServoPosition(ARM_BASE_SERVO, 55, 180, 40);
  delay(100);
  setServoPosition(MID_JOINT_SERVO, 24, 180, 40);
  delay(100);
  int currentBaseServoAngle = 125;  

}

void startNextTask() {
  if (!taskQueue.empty()) {
    int targetPosition = taskQueue.front();
    taskQueue.pop();

    taskStartTime = millis();
    isTaskRunning = true;
    isWashing = (targetPosition == 45); // Washing position

    Serial.print("Starting task. Target position: ");
    Serial.println(targetPosition);

    // Step 1: Raise the arm first
    setServoPosition(WHISK_ANGLE_SERVO, 170, 180, 40);
    delay(1000);
    setServoPosition(MID_JOINT_SERVO, 50, 180, 40);
    delay(1000);

    // Step 2: Move the base servo to the target position
    setServoPosition(BASE_SERVO, targetPosition, 180, 40);
    currentBaseServoAngle = targetPosition; // Update after movement
    delay(1000);

    setServoPosition(WHISK_ANGLE_SERVO, 175, 180, 40);
    delay(500);
    // Step 3: Lower the arm to whisking position
    setServoPosition(MID_JOINT_SERVO, 22, 180, 40);
    delay(500);
    setServoPosition(WHISK_ANGLE_SERVO, 165, 180, 40);
    delay(500);
  } else {
    Serial.println("No tasks left in the queue.");
  }
}

// Finish the current task
void finishTask() {
  Serial.println(isWashing ? "Washing finished." : "Whisking finished.");
  isTaskRunning = false;
  if (!taskQueue.empty()) {
    startNextTask();
  } else {
    Serial.println("All tasks completed.");
  }
}

void performWhisking() {
  myStepper.setSpeed(200); 

  // Define servo motion range and parameters
  const int baseServoMinAngle = currentBaseServoAngle - 15; // Minimum angle for side-to-side motion
  const int baseServoMaxAngle = currentBaseServoAngle + 5; // Maximum angle
  const int baseServoStep = 1;       // Step size for each movement
  int baseServoAngle = currentBaseServoAngle;          // Starting position for the servo
  bool increasing = true;            // Direction flag for servo movement

  // Perform whisking
  for (int i = 0; i < 5; i++) { // Perform 5 whisking cycles
    // Backward motion
    Serial.println("Starting whisking motion: backward");
    myStepper.step(-100); // Move backward by 50 steps
    Serial.println("Finished whisking motion: backward");

    // Adjust the base servo angle
    if (increasing) {
      baseServoAngle += baseServoStep;
      if (baseServoAngle >= baseServoMaxAngle) {
        increasing = false; // Reverse direction
      }
    } else {
      baseServoAngle -= baseServoStep;
      if (baseServoAngle <= baseServoMinAngle) {
        increasing = true; // Reverse direction
      }
    }
    setServoPosition(BASE_SERVO, baseServoAngle, 180, 5); // Update servo position

    // Forward motion
    Serial.println("Starting whisking motion: forward");
    myStepper.step(100); // Move forward by 50 steps
    Serial.println("Finished whisking motion: forward");

    // Adjust the base servo angle again
    if (increasing) {
      baseServoAngle += baseServoStep;
      if (baseServoAngle >= baseServoMaxAngle) {
        increasing = false; // Reverse direction
      }
    } else {
      baseServoAngle -= baseServoStep;
      if (baseServoAngle <= baseServoMinAngle) {
        increasing = true; // Reverse direction
      }
    }
    setServoPosition(BASE_SERVO, baseServoAngle, 180, 5); // Update servo position
    int currentBaseServoAngle = baseServoAngle;  
  }

  // Optionally, set the servo back to a neutral position after whisking
  // setServoPosition(BASE_SERVO, 125, 180, 5); // Return to neutral position
}


// Function to set servo position using the Adafruit_PWMServoDriver
void setServoPosition(int servo, int targetAngle, int maxAngle, int movementDelay) {
  int *currentAnglePtr;

  switch (servo) {
    case BASE_SERVO:
      currentAnglePtr = &currentBaseServoAngle;
      break;
    case ARM_BASE_SERVO:
      currentAnglePtr = &currentArmBaseServoAngle;
      break;
    case MID_JOINT_SERVO:
      currentAnglePtr = &currentMidJointServoAngle;
      break;
    case WHISK_ANGLE_SERVO:
      currentAnglePtr = &currentWhiskAngleServoAngle;
      break;
    default:
      return;
  }

  int currentAngle = *currentAnglePtr;
  int step = (targetAngle > currentAngle) ? 1 : -1;

  while (currentAngle != targetAngle) {
    currentAngle += step;
    int pulse = map(currentAngle, 0, maxAngle, SERVOMIN, SERVOMAX);
    board1.setPWM(servo, 0, pulse);
    delay(movementDelay);
  }

  *currentAnglePtr = currentAngle;
}

// Function to parse the cup location from the HTTP request
int parseCupLocation(String request) {
  int cupIndex = request.indexOf("cup=");
  if (cupIndex >= 0) {
    return request.substring(cupIndex + 4).toInt();  // Extract cup location
  }
  return -1;  // Return -1 if "cup=" is not found
}

