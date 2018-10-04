#include <Servo.h>
#include <PS4BT.h>
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

USB Usb;
//USBHub Hub1(&Usb); // Some dongles have a hub inside
BTD Btd(&Usb); // You have to create the Bluetooth Dongle instance like so

/* You can create the instance of the PS4BT class in two ways */
// This will start an inquiry and then pair with the PS4 controller - you only have to do this once
// You will need to hold down the PS and Share button at the same time, the PS4 controller will then start to blink rapidly indicating that it is in pairing mode
PS4BT PS4(&Btd, PAIR);

//----------INITIALIZING SERVOS&ESC's-------------
Servo rudder;    // STEERING RUDDER SERVO
Servo liftfan;   // LIFT FAN MOTOR/ESC
Servo thrustfan; // THRUST FAN MOTOR/ESC

int offset = 0;  // RUDDER TRIM
int steer = 127; // RUDDER ANGLE 0 - 180 DEG
int lift = 56;   // LIFT FAN INITIAL MIN VAL TO CAL ESC
int thrust = 56; // THRUST FAN INTIAL MIN VAL TO CAL ESC

// After that you can simply create the instance like so and then press the PS button on the device
//PS4BT PS4(&Btd);

bool printAngle, printTouch;
uint8_t oldL2Value, oldR2Value;

void setup()
{
  //---------SET PINS AND ATTACH SERVOS&ESC's--------
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(4, OUTPUT);

  rudder.attach(5);
  liftfan.attach(6);
  thrustfan.attach(4);

  Serial.begin(115200);

#if !defined(__MIPSEL__)
  while (!Serial)
    ; // Wait for serial port to connect
#endif

  if (Usb.Init() == -1)
  {
    Serial.print(F("\r\nOSC did not start"));
    while (1)
      ; // Halt
  }

  Serial.print(F("\r\nPS4 Bluetooth Library Started"));
}
void loop()
{
  Usb.Task();

  if (PS4.connected())
  {
    //-------------STEERING--------------
    steer = PS4.getAnalogHat(RightHatX);

    if (PS4.getButtonClick(RIGHT))
    { // trim right
      offset += 10;

      Serial.print("\n\n\t\tOFFSET= ");
      Serial.print(offset);
    }
    if (PS4.getButtonClick(LEFT))
    { // trim left
      offset -= 10;

      Serial.print("\n\n\t\tOFFSET= ");
      Serial.print(offset);
    }

    steer = steer + offset; // incorporate trim into steer command
    if (steer > 255)
      steer = 255; // enforce upper limit
    if (steer < 0)
      steer = 0; // enforce lower limit

    rudder.write(map(steer, 0, 255, 10, 18 0)); // set rudder servo to new steer value

    if (PS4.getAnalogHat(RightHatX) > 137 || PS4.getAnalogHat(RightHatX) < 117)
    {
      Serial.print("\n\n\t\tSTEERING= ");
      Serial.print(steer);
    }

    //----------------LIFT------------------

    if (PS4.getAnalogButton(L2) > 30)
    {
      if (PS4.getButtonClick(UP))
      {
        lift += 1;

        Serial.print("\n\n\t\tLIFT= ");
        Serial.print(lift);
      }
      if (PS4.getButtonClick(DOWN))
      {
        lift -= 1;

        Serial.print("\n\n\t\tLIFT= ");
        Serial.print(lift);
      }

      if (lift > 65)
        lift = 65; // enforce upper limit
      if (lift < 56)
        lift = 56;

      liftfan.write(lift);
    }
    else
    {
      liftfan.write(56);
    }

    //----------------THRUST------------------
    thrust = 50 + map(PS4.getAnalogButton(R2), 0, 255, 0, 20);

    thrustfan.write(thrust);

    if (PS4.getAnalogButton(L2) || PS4.getAnalogButton(R2))
    { // These are the only analog buttons on the PS4 controller
      Serial.print("\n\n\t\tTHROTTLE= ");
      Serial.print(thrust);
    }

    //----------------RUMBLE------------------
    if (PS4.getAnalogButton(L2) != oldL2Value || PS4.getAnalogButton(R2) != oldR2Value) // Only write value if it's different
    {
      PS4.setRumbleOn(PS4.getAnalogButton(R2));
      oldL2Value = PS4.getAnalogButton(L2);
      oldR2Value = PS4.getAnalogButton(R2);
    }
  }
}
