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
#define SERVOMAX 700

// Stepper motor configuration
#define STEPPER_PIN_1 2
#define STEPPER_PIN_2 3
const int stepsPerRevolution = 200; // Steps per revolution for the stepper
Stepper myStepper(stepsPerRevolution, STEPPER_PIN_1, STEPPER_PIN_2);

int currentBaseServoAngle = 44;
int currentArmBaseServoAngle = 44;
int currentMidJointServoAngle = 29;
int currentWhiskAngleServoAngle = 59;

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
        taskQueue.push(cupLocation == 1 ? 90 : 5); // Adjust angles based on cup positions
        taskQueue.push(45);

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
        setServoPosition(WHISK_ANGLE_SERVO, 175, 180, 40);
      }
    }
  }
}

// Initialize servos to starting positions
void initializeServos() {
  setServoPosition(BASE_SERVO, 40, 180, 40); // extra movemenr here
  delay(1000);
  // setServoPosition(ARM_BASE_SERVO, 30, 180, 40);
  // delay(100);
  // setServoPosition(MID_JOINT_SERVO, 10, 180, 40); // theres an extra movement here?
  // delay(100);
  // setServoPosition(WHISK_ANGLE_SERVO, 160, 180, 40);
  // delay(100);
  // setServoPosition(MID_JOINT_SERVO, 40, 180, 40); // theres an extra movement here?
  // delay(100);
  // setServoPosition(ARM_BASE_SERVO, 20, 180, 40);
  // delay(100);
  // setServoPosition(WHISK_ANGLE_SERVO, 180, 180, 40);
  // delay(100);
  // setServoPosition(MID_JOINT_SERVO, 10, 180, 40); // theres an extra movement here?
  // delay(100);
  
  // setServoPosition(MID_JOINT_SERVO, 10, 180, 40); // theres an extra movement here?
  // delay(100);
  // setServoPosition(WHISK_ANGLE_SERVO, 0, 180, 40);
  // delay(100);
  // setServoPosition(WHISK_ANGLE_SERVO, 20, 180, 40);
  // delay(100);
  // setServoPosition(WHISK_ANGLE_SERVO, 40, 180, 40);
  // delay(100);
  // setServoPosition(WHISK_ANGLE_SERVO, 60, 180, 40);
  // delay(100);
  // setServoPosition(WHISK_ANGLE_SERVO, 80, 180, 40);
  // delay(100);
  // setServoPosition(WHISK_ANGLE_SERVO, 100, 180, 40);
  // delay(100);
  // setServoPosition(WHISK_ANGLE_SERVO, 120, 180, 40);
  // delay(100);
  // setServoPosition(WHISK_ANGLE_SERVO, 140, 180, 40);
  // delay(100);
  // setServoPosition(WHISK_ANGLE_SERVO, 160, 180, 40);
  // delay(100);
  // setServoPosition(WHISK_ANGLE_SERVO, 180, 180, 40);
  // delay(100);


  setServoPosition(WHISK_ANGLE_SERVO, 60, 180, 40);
  delay(100);
  setServoPosition(MID_JOINT_SERVO, 10, 180, 40); // theres an extra movement here?
  delay(100);
  setServoPosition(ARM_BASE_SERVO, 30, 180, 40);
  delay(100);
  setServoPosition(MID_JOINT_SERVO, 30, 180, 40);
  delay(1000);
  setServoPosition(WHISK_ANGLE_SERVO, 160, 180, 40);
  delay(1000);
  setServoPosition(WHISK_ANGLE_SERVO, 180, 180, 40);
  delay(1000);
  setServoPosition(ARM_BASE_SERVO, 40, 180, 40);
  delay(100);
}

// Start the next task in the queue
void startNextTask() {
  if (!taskQueue.empty()) {
    int targetPosition = taskQueue.front();
    taskQueue.pop();

    taskStartTime = millis();
    isTaskRunning = true;
    isWashing = (targetPosition == 45); // Washing position

    Serial.print("Starting task. Target position: ");
    Serial.println(targetPosition);

    setServoPosition(WHISK_ANGLE_SERVO, 180, 180, 40);
    delay(1500);
    setServoPosition(MID_JOINT_SERVO, 50, 180, 40);
    delay(1500);
    setServoPosition(BASE_SERVO, targetPosition, 180, 40);
    delay(1500);
    setServoPosition(MID_JOINT_SERVO, 30, 180, 40);
    delay(1500);
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

// Perform whisking movement with the stepper motor
void performWhisking() {
  myStepper.setSpeed(30); // Set speed to 30 RPM

  // Backward motion
  Serial.println("Starting whisking motion: backward");
  myStepper.step(-50); // Move backward by 50 steps
  Serial.println("Finished whisking motion: backward");

  // Forward motion
  Serial.println("Starting whisking motion: forward");
  myStepper.step(50); // Move forward by 50 steps
  Serial.println("Finished whisking motion: forward");
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
