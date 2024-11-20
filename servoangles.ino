#include <WiFiS3.h>
#include <AccelStepper.h>
#include <Servo.h> 

#define BASE_SERVO_PIN 8
#define ARM_BASE_SERVO_PIN 9
#define MID_JOINT_SERVO_PIN 10
#define WHISK_ANGLE_SERVO_PIN 11

#define STEPPER_PIN_1 4
#define STEPPER_PIN_2 5
#define STEPPER_PIN_3 6
#define STEPPER_PIN_4 7

#define STEPS_PER_REVOLUTION 200 

AccelStepper stepper(AccelStepper::FULL4WIRE, STEPPER_PIN_1, STEPPER_PIN_2, STEPPER_PIN_3, STEPPER_PIN_4);
WiFiServer server(80);

Servo baseServo;
Servo armBaseServo;
Servo midJointServo;
Servo whiskAngleServo;

int currentBaseAngle = 90;
int currentArmBaseAngle = 45;
int currentMidJointAngle = 32;
int currentWhiskAngle = 180;

unsigned long whiskStartTime = 0;
bool startWhisking = false;
bool isForwardMotion = true;
unsigned long lastStepperUpdate = 0;
bool isAligning = false;
void setup() {
  Serial.begin(9600);

  WiFi.beginAP("MatchaBot_AP", "matcha123");
  Serial.println("WiFi AP started");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();

  baseServo.attach(BASE_SERVO_PIN);
  armBaseServo.attach(ARM_BASE_SERVO_PIN);
  midJointServo.attach(MID_JOINT_SERVO_PIN);
  whiskAngleServo.attach(WHISK_ANGLE_SERVO_PIN);

  initializeServos();

  stepper.setMaxSpeed(50);    
  stepper.setAcceleration(25);

  stepper.setCurrentPosition(10); 
  Serial.println("Stepper initialized at position 0");
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
      Serial.println("Whisking stopped after 1 minute");
      resetPositions(); 
    }
  }

  if (isAligning) {
    stepper.run();
    if (!stepper.isRunning()) {
      isAligning = false; 
    }
  }

  if (startWhisking || isAligning) {
    stepper.run();
  }
}

void performWhisking() {
  unsigned long currentTime = millis();

  if (currentTime - lastStepperUpdate >= 1000) { 
    lastStepperUpdate = currentTime;

    if (isForwardMotion) {
      stepper.moveTo(50); 
      Serial.println("Stepper moving forward");
    } else {
      stepper.moveTo(-50); 
      Serial.println("Stepper moving backward");
    }

    isForwardMotion = !isForwardMotion; 
  }
}

void resetPositions() {
  Serial.println("Resetting positions...");
  moveServo(baseServo, currentBaseAngle, 55, 20);
  Serial.println("Moved base to water cup position");

  moveServo(armBaseServo, currentArmBaseAngle, 45, 20);  
  moveServo(midJointServo, currentMidJointAngle, 40, 20); 
  moveServo(whiskAngleServo, currentWhiskAngle, 180, 20);  
  alignStepperToWhiskAngle(currentWhiskAngle); 
}

void initializeServos() {
  Serial.println("Initializing servos...");
  baseServo.write(currentBaseAngle);
  armBaseServo.write(currentArmBaseAngle);
  midJointServo.write(currentMidJointAngle);
  whiskAngleServo.write(currentWhiskAngle);
  alignStepperToWhiskAngle(currentWhiskAngle);
  delay(1000);
}

void moveServo(Servo &servo, int &currentAngle, int targetAngle, int delayMs) {
  int step = (targetAngle > currentAngle) ? 1 : -1;

  while (currentAngle != targetAngle) {
    currentAngle += step;
    servo.write(currentAngle);
    delay(delayMs); 
  }
}

void alignStepperToWhiskAngle(int whiskAngle) { // fix ts
  long targetStepperPosition = map(whiskAngle, 0, 180, 0, STEPS_PER_REVOLUTION);

  stepper.moveTo(targetStepperPosition);
  isAligning = true;
}
