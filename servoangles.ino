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

  setServoPosition(WHISK_ANGLE_SERVO, 60, 180, 20);
  delay(100);
  setServoPosition(MID_JOINT_SERVO, 10, 180, 20);
  delay(100);
  setServoPosition(ARM_BASE_SERVO, 30, 180, 20);
  delay(100);
  setServoPosition(BASE_SERVO, 90, 180, 20);
  delay(100);
  setServoPosition(MID_JOINT_SERVO, 32, 180, 20);
  delay(100);
  setServoPosition(WHISK_ANGLE_SERVO, 180, 180, 20);
  delay(100);
  setServoPosition(ARM_BASE_SERVO, 40, 180, 20);
  delay(100);
  setServoPosition(ARM_BASE_SERVO, 45, 180, 20);

  stepper.setMaxSpeed(300); 
  stepper.setAcceleration(150); 
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client connected