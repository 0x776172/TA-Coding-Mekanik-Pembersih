#include <LiquidCrystal_I2C.h>
#include <Arduino.h>

#define lsHome 2
#define ls1 3
#define ls2 4
#define lsMax 5
#define pinProx 6

#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11
#define PINM 16
int panelDirty = 0;
bool isIdle = false;

enum posDevice
{
  initializing,
  home,
  satu,
  dua,
  edge
} currDevicePosition;

enum stateMotor
{
  stop,
  kiri,
  kanan
} motorState;

enum stateCleaning
{
  panel1,
  panel2,
  panel3,
  doneCleaning
} cleaningState;

enum stateRotate
{
  mati,
  nyala
};

enum stateAir
{
  aktif,
  nonaktif
};

// function declaration
posDevice position(posDevice currPos); // return the next position of device
stateCleaning cleaning(stateCleaning currCleaning, int countClean = 4);
void motor(stateMotor currMotor, stateRotate rotateMotor = mati, stateAir air = nonaktif);

void setup()
{
  Serial.begin(115200);
  // Serial2.begin(115200);
  // INPUT DECLARATION
  pinMode(lsHome, INPUT);
  pinMode(ls1, INPUT);
  pinMode(ls2, INPUT);
  pinMode(lsMax, INPUT);
  pinMode(pinProx, INPUT);
  // output declaration
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(PINM, OUTPUT);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  digitalWrite(PINM, LOW);
  currDevicePosition = initializing;
  Serial.println("device initializing");
  while (currDevicePosition == initializing)
  {
    currDevicePosition = position(home);
  }
  Serial.println("device in home");
  // Serial2.println("D1;");
  isIdle = true;
  panelDirty = 1;

  // done checking
  delay(10000);
}

void loop()
{

  if (isIdle && currDevicePosition == home)
  {
    switch (panelDirty)
    {
    case 1:
      Serial.print("Panel dirty: ");
      Serial.println(panelDirty);
      cleaning(panel1);
      delay(500);
      cleaning(doneCleaning);
      break;

    case 2:
      Serial.print("Panel dirty: ");
      Serial.println(panelDirty);
      cleaning(panel2);
      delay(500);
      cleaning(doneCleaning);
      break;

    case 3:
      Serial.print("Panel dirty: ");
      Serial.println(panelDirty);
      cleaning(panel3);
      delay(500);
      cleaning(doneCleaning);
      break;
    }
  }
}

// void serialEvent2()
// {
//   switch (Serial2.read())
//   {
//   case 'P':
//     panelDirty = Serial2.parseInt();
//     break;
//   }
// }

stateCleaning cleaning(stateCleaning currCleaning, int countClean)
{
  isIdle = false;
  stateCleaning res;
  int lastState;
  int stateProx;
  int currCount = 0;
  uint8_t lastButtonHome = digitalRead(lsHome);
  uint8_t buttonStateHome;
  uint8_t lastButtonEdge = digitalRead(lsMax);
  uint8_t buttonStateEdge;
  switch (currCleaning)
  {
  case panel1:
    Serial.println("Cleaning panel 1");
    lastState = digitalRead(pinProx);
    delay(10);
    lastState = digitalRead(pinProx);
    currDevicePosition = position(satu);

    while (currCount <= countClean)
    {
      buttonStateHome = digitalRead(lsHome);
      delay(10);
      buttonStateHome = digitalRead(lsHome);
      if (buttonStateHome)
      {
        if (lastButtonHome != buttonStateHome)
        {
          if (buttonStateHome)
          {
            motor(kanan, nyala, aktif);
          }
        }
      }
      else
      {
        stateProx = digitalRead(pinProx);
        delay(10);
        stateProx = digitalRead(pinProx);
        if (lastState != stateProx)
        {
          if (!stateProx)
          {
            currCount++;
            motor(kiri, nyala, aktif);
          }
        }
        lastState = stateProx;
      }
      lastButtonHome = buttonStateHome;
    }
    motor(stop);
    res = panel1;
    break;

  case panel2:
    Serial.println("Cleaning panel 2");
    lastState = digitalRead(pinProx);
    delay(10);
    lastState = digitalRead(pinProx);
    currDevicePosition = position(dua);
    while (currCount <= countClean * 2)
    {
      stateProx = digitalRead(pinProx);
      delay(10);
      stateProx = digitalRead(pinProx);
      if (lastState != stateProx)
      {
        if ((!stateProx) && currCount % 2 == 0)
        {
          currCount++;
          motor(kiri, nyala);
        }
        else if ((!stateProx) && currCount % 2 != 0)
        {
          currCount++;
          motor(kanan, nyala);
        }
      }
      lastState = stateProx;
    }
    motor(stop);
    res = panel2;
    break;

  case panel3:
    Serial.println("Cleaning panel 3");

    currDevicePosition = position(edge);
    lastState = digitalRead(pinProx);
    delay(10);
    lastState = digitalRead(pinProx);
    while (currCount <= countClean)
    {
      buttonStateEdge = digitalRead(lsMax);
      delay(10);
      buttonStateEdge = digitalRead(lsMax);
      if (buttonStateEdge)
      {
        if (lastButtonEdge != buttonStateEdge)
        {
          if (buttonStateEdge)
          {
            currCount++;
            motor(kiri, nyala);
          }
        }
      }
      else
      {
        stateProx = digitalRead(pinProx);
        delay(10);
        stateProx = digitalRead(pinProx);
        if (lastState != stateProx)
        {
          if (!stateProx)
          {
            // currCount++;
            motor(kanan, nyala);
          }
        }
        lastState = stateProx;
      }
      lastButtonEdge = buttonStateEdge;
    }
    motor(stop);
    res = panel3;
    break;

  case doneCleaning:
    Serial.println("Done Cleaning");
    currDevicePosition = position(home);
    panelDirty = 0;
    isIdle = true;
    Serial2.println("D1;");
    break;
  }
  return res;
}

posDevice position(posDevice currPos)
{
  posDevice res;
  int lastStateProx;
  int stateProx;
  switch (currPos)
  {

  case initializing:
    break;

  case home:
    Serial.println("Going Home");
    motor(kiri);
    while (true)
    {
      if (digitalRead(lsHome))
      {
        delay(10);
        if (digitalRead(lsHome))
        {
          motor(stop);
          break;
        }
      }
    }
    // motor(stop);
    // Serial2.println("done");
    Serial.println("in Home");
    res = home;
    break;

  case edge:
    Serial.println("Going Edge");
    motor(kanan);
    while (true)
    {
      if (digitalRead(lsMax))
      {
        delay(10);
        if (digitalRead(lsMax))
        {
          motor(stop);
          break;
        }
      }
    }
    Serial.println("In edge");
    res = edge;
    break;

  case satu:
    Serial.println("Going panel1");
    motor(kanan);
    lastStateProx = digitalRead(pinProx);
    delay(10);
    lastStateProx = digitalRead(pinProx);
    while (true)
    {
      stateProx = digitalRead(pinProx);
      delay(10);
      stateProx = digitalRead(pinProx);
      if (lastStateProx != stateProx)
      {
        if (stateProx == LOW)
        {
          motor(stop);
          break;
        }
      }
    }
    Serial.println("in panel1");
    lastStateProx = stateProx;
    res = satu;
    break;

  case dua:
    int counter = 0;
    Serial.println("Going panel2");
    lastStateProx = digitalRead(pinProx);
    delay(10);
    lastStateProx = digitalRead(pinProx);
    motor(kanan);
    while (true)
    {
      stateProx = digitalRead(pinProx);
      delay(10);
      stateProx = digitalRead(pinProx);
      if (lastStateProx != stateProx)
      {
        if (!stateProx)
        {
          // motor(stop);
          counter++;
          if (counter > 1)
          {
            counter = 0;
            motor(stop, mati);
            break;
          }
        }
      }
      lastStateProx = stateProx;
    }
    Serial.println("in panel1");
    res = dua;
    break;
  }
  return res;
}

void motor(stateMotor currMotor, stateRotate rotateMotor, stateAir air)
{
  switch (currMotor)
  {
  case kanan:
    Serial.println("Motor kanan");
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    // digitalWrite(IN3, HIGH);
    // digitalWrite(IN4, LOW);
    break;

  case kiri:
    Serial.println("Motor Kiri");
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    // digitalWrite(IN3, HIGH);
    // digitalWrite(IN4, LOW);
    break;

  case stop:
    Serial.println("Motor stop");
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    break;
  }

  switch (rotateMotor)
  {
  case nyala:
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    break;

  case mati:
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    break;
  }

  switch (air)
  {
  case aktif:
    digitalWrite(16, HIGH);
    break;

  case nonaktif:
    digitalWrite(16, LOW);
    break;
  }
}