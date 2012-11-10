#include <Stepper.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Ethernet.h>
#include <String.h>

byte mac[] = {0x90, 0xA2, 0xDA, 0x00, 0x01, 0x1B};
byte ip[] = {192, 168, 0, 77};
byte server[] = {192, 168, 0, 3};

// number of stepper motor steps:
#define MOTOR_STEPS 200

// number of slides and lenses
#define SLIDES 10
#define LENSES 5

// control pins for the stepper motor:
#define MOTOR_PIN1 6
#define MOTOR_PIN2 7
#define MOTOR_PIN3 8
#define MOTOR_PIN4 9

// control pins for the demux
#define DEMUX_A 4
#define DEMUX_B 3
#define DEMUX_C 2

// motor demux numbers
#define SLIDE_MOTOR        3 //final
#define TOP_LENSE_MOTOR    6 //final
#define BOT_LENSE_MOTOR    4 //final
#define X_MOTOR            0 //final
#define Y_MOTOR            1 //final
#define Z_MOTOR            3 //final 


// define the no motor selection
#define OFF -1

// define motor speed
#define M_SPEED 40

// create the stepper instance:
Stepper MyStepper = Stepper(MOTOR_STEPS, MOTOR_PIN1, MOTOR_PIN2, MOTOR_PIN3, MOTOR_PIN4);

PubSubClient client(server, 1885, callback);

//global variables
int currentSlide = 0;     // current slide selected
int currentTopLense = 0;  // current top lense selected
int currentBotLense = 0;  // current bottom lense selected
int currentMotor;         // current motor running
int currentDirection;     // 1 or -1 (multiplier)
int kill = 1;             // 1 when no motor running, 0 otherwise

// initialise the demux
void initSelector() {
  pinMode(DEMUX_A, OUTPUT);
  pinMode(DEMUX_B, OUTPUT);
  pinMode(DEMUX_C, OUTPUT);
}

// select the motor to run by specifying the demux number
void selectMotor(int motor) {
  switch (motor) {
    case 0: 
      digitalWrite(DEMUX_A, LOW);
      digitalWrite(DEMUX_B, LOW);
      digitalWrite(DEMUX_C, LOW);
      break;
    case 1:
      digitalWrite(DEMUX_A, HIGH);
      digitalWrite(DEMUX_B, LOW);
      digitalWrite(DEMUX_C, LOW);
      break;
    case 2:
      digitalWrite(DEMUX_A, LOW);
      digitalWrite(DEMUX_B, HIGH);
      digitalWrite(DEMUX_C, LOW);
      break;
    case 3:
      digitalWrite(DEMUX_A, HIGH);
      digitalWrite(DEMUX_B, HIGH);
      digitalWrite(DEMUX_C, LOW);
      break;
    case 4:
      digitalWrite(DEMUX_A, LOW);
      digitalWrite(DEMUX_B, LOW);
      digitalWrite(DEMUX_C, HIGH);
      break;
    case 5:
      digitalWrite(DEMUX_A, HIGH);
      digitalWrite(DEMUX_B, LOW);
      digitalWrite(DEMUX_C, HIGH);
      break;
    case 6:
      digitalWrite(DEMUX_A, LOW);
      digitalWrite(DEMUX_B, HIGH);
      digitalWrite(DEMUX_C, HIGH);
      break;     
    default:
      digitalWrite(DEMUX_A, HIGH);
      digitalWrite(DEMUX_B, HIGH);
      digitalWrite(DEMUX_C, HIGH);
  }    
}

// drive the specified demux motor for the given steps.
// delays until the motor has finished running
void driveMotor(int motor, int steps) {
  selectMotor(motor);
  MyStepper.step(steps);
}

// turn to the selected slide
void setSlide(int slideNum) {
  MyStepper.setSpeed(M_SPEED);
  int steps = (slideNum - currentSlide) * (MOTOR_STEPS/SLIDES);
  driveMotor(SLIDE_MOTOR, steps);
  currentSlide = slideNum;
}

// turn to the selected top lense
void setTopLense(int lenseNum) {
  MyStepper.setSpeed(M_SPEED);
  int steps = (lenseNum - currentTopLense) * (MOTOR_STEPS/LENSES);
  driveMotor(TOP_LENSE_MOTOR, steps);
  currentTopLense = lenseNum;
}

// turn to the selected bot lense
void setBotLense(int lenseNum) {
  MyStepper.setSpeed(M_SPEED);
  int steps = (lenseNum - currentBotLense) * (MOTOR_STEPS/LENSES);
  driveMotor(BOT_LENSE_MOTOR, steps);
  currentBotLense = lenseNum;
}


// payload format: mds || m#
// m = motor:
//     - s = slide motor
//     - t = top lense motor
//     - b = bottom lense motor
//     - x = x motor
//     - y = y motor
//     - z = z motor
//     - k = kill motors
// d = direction
//     - f = forward
//     - b = backward
// s = speed (RPM)
// # = select slide/lense number 
void callback(char* topic, byte* payload, unsigned int length) {
  int i;
  // check if its a kill command
  if (payload[0] == 'k') {
    kill = 1;
    Serial.println("kill motors");
    return;
  } else {
    kill = 0;
  }
  // determine what motor to operate and which direction
  switch ((char) payload[0]) {
    case ('s'):
      kill = 1;
      //sscanf((char*)payload, "s%d", &i);
      i = payload[1] - '0';
      Serial.print("Change to slide ");
      Serial.println(i);
      setSlide(i);
      return;
    case ('t'):
      kill = 1;
      //sscanf((char*)payload, "t%d", &i);
      i = payload[1] - '0';
      Serial.print("Change to top lense ");
      Serial.println(i);
      setTopLense(i);
      return;
    case ('b'):
      kill = 1;
      //sscanf((char*)payload, "b%d", &i);
      i = payload[1] - '0';
      Serial.print("Change to bottom lense ");
      Serial.println(i);
      setBotLense(i);
      return;
    case ('x'):
      currentMotor = X_MOTOR;
      Serial.print("drive x motor ");
      if ((char) payload[1] == 'f') {
        currentDirection = 1;
        Serial.print("forward at ");
        sscanf((char*)payload, "xf%d", &i);
        Serial.print(i);
        Serial.println("RPM");
        MyStepper.setSpeed(i);
      } else if ((char) payload[1] == 'b') {
        currentDirection = -1;
        Serial.print("backward at ");
        sscanf((char*)payload, "xb%d", &i);
        Serial.print(i);
        Serial.println("RPM");
        MyStepper.setSpeed(i);
      } else {
        kill = 1;
      }
      break;
    case ('y'):
      currentMotor = Y_MOTOR;
      Serial.print("drive y motor ");
      if ((char) payload[1] == 'f') {
        currentDirection = 1;
        Serial.print("forward at ");
        sscanf((char*)payload, "yf%d", &i);
        Serial.print(i);
        Serial.println("RPM");
        MyStepper.setSpeed(i);
      } else if ((char) payload[1] == 'b') {
        currentDirection = -1;
        Serial.print("backward at ");
        sscanf((char*)payload, "yb%d", &i);
        Serial.print(i);
        Serial.println("RPM");
        MyStepper.setSpeed(i);
      } else {
        kill = 1;
      }
      break;
    case ('z'):
      currentMotor = Z_MOTOR;
      Serial.print("drive z motor ");
      if ((char) payload[1] == 'f') {
        currentDirection = 1;
        Serial.print("forward at ");
        sscanf((char*)payload, "zf%d", &i);
        Serial.print(i);
        Serial.println("RPM");
        MyStepper.setSpeed(i);
      } else if ((char) payload[1] == 'b') {
        currentDirection = -1;
        Serial.print("backward at ");
        sscanf((char*)payload, "zb%d", &i);
        Serial.print(i);
        Serial.println("RPM");
        MyStepper.setSpeed(i);
      } else {
        kill = 1;
      }
      break;
    default:
      kill = 1;
      return;
  }
}
  
void setup() {
  // set the motor speed to 60RPMs (1 per sec):
  MyStepper.setSpeed(M_SPEED);
  // initialise the demux
  initSelector();
  // Initialise the serial port
  Ethernet.begin(mac, ip);
  Serial.begin(9600);
  if (client.connect("microscope")) {
    client.subscribe("/Microscope");
    Serial.println("connected");
  }
  
}

void loop() {
  if(!client.loop()) {
    client.connect("microscope");
    client.subscribe("/Microscope");
    Serial.println("reconnected");
  }
  if (!kill) {
    driveMotor(currentMotor, currentDirection*20);
    Serial.print('.');
  } else {
    selectMotor(OFF);
  }
}
