#include <WiFiS3.h>
#include <Adafruit_PWMServoDriver.h>
#include <AccelStepper.h>

#define BASE_SERVO 3
#define ARM_BASE_SERVO 2
#define MID_JOINT_SERVO 1
#define WHISK_ANGLE_SERVO 0
#define SERVOMIN 125
#define SERVOMAX 625

#define STEPPER_PIN_1 2
#define STEPPER_PIN_2 3
#define STEPPER_PIN_3 4
#define STEPPER_PIN_4 5

int currentBaseServoAngle = 90;
int currentArmBaseServoAngle = 45;
int currentMidJointServoAngle = 32;
int currentWhiskAngleServoAngle = 180;

AccelStepper stepper(AccelStepper::FULL4WIRE, STEPPER_PIN_1, STEPPER_PIN_2, STEPPER_PIN_3, STEPPER_PIN_4);
Adafruit_PWMServoDriver board1 = Adafruit_PWMServoDriver(0x40);

WiFiServer server(80);

unsigned long whiskStartTime = 0;
bool startWhisking = false;
bool isForwardMotion = true;
bool dirtyCup = false;
unsigned long lastStepperUpdate = 0;

void setup() {
  Serial.begin(9600);

  WiFi.beginAP("MatchaBot_AP", "matcha123");
  Serial.println("WiFi AP started");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.localIP());
  
  server.begin();

  board1.begin();
  board1.setPWMFreq(60);

  setServoPosition(WHISK_ANGLE_SERVO, 60, 180, 40);
  delay(100);
  setServoPosition(MID_JOINT_SERVO, 10, 180, 40);
  delay(100);
  setServoPosition(ARM_BASE_SERVO, 30, 180, 40);
  delay(100);
  setServoPosition(BASE_SERVO, 95, 180, 40);
  delay(100);
  setServoPosition(MID_JOINT_SERVO, 32, 180, 40);
  delay(100);
  setServoPosition(WHISK_ANGLE_SERVO, 180, 180, 40);
  delay(100);
  setServoPosition(ARM_BASE_SERVO, 40, 180, 40);
  delay(100);
  setServoPosition(ARM_BASE_SERVO, 45, 180, 40);

  stepper.setMaxSpeed(300); 
  stepper.setAcceleration(150);
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client connected");
    String request = client.readStringUntil('\r');
    client.flush();
    
    Serial.print("Request: ");
    Serial.println(request);
    
    if (request.indexOf("GET /start") >= 0) {
      Serial.println("Received /start request");
      whiskStartTime = millis();
      startWhisking = true;
      isForwardMotion = true;
      client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nWhisking started");
    } else {
      client.print("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nEndpoint not found");
    }
    client.stop();
  }

  if (startWhisking) {
    unsigned long elapsedTime = millis() - whiskStartTime;
    if (elapsedTime < 60000) { 
      performWhisking();
    } else {
      startWhisking = false;
      dirtyCup = true;
      Serial.println("Whisking stopped after 1 minute");
      resetPositions();
    }
  }

  if (dirtyCup) {
    setServoPosition(BASE_SERVO, 55, 180, 40);
  }

  stepper.run();
}

void performWhisking() {
  unsigned long currentTime = millis();

  if (currentTime - lastStepperUpdate >= 200) {
    lastStepperUpdate = currentTime;

    if (isForwardMotion) {
      stepper.moveTo(50); 
      isForwardMotion = false;
    } else {
      stepper.moveTo(-50);
      isForwardMotion = true;
    }
  }
}

void resetPositions() {
  Serial.println("Resetting positions...");
  setServoPosition(BASE_SERVO, 90, 180, 20);
  setServoPosition(ARM_BASE_SERVO, 45, 180, 20);
  setServoPosition(MID_JOINT_SERVO, 40, 180, 20);
  setServoPosition(WHISK_ANGLE_SERVO, 180, 180, 20);
}

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
