#include <OrangutanLEDs.h>
#include <OrangutanAnalog.h>
#include <OrangutanMotors.h>
#include <OrangutanLCD.h>
#include <OrangutanLEDs.h>
#include <OrangutanPushbuttons.h>

//Principais variáveis
int execucao = 0;

int io0 = 0;

OrangutanLCD lcd;
OrangutanLEDs leds;
OrangutanPushbuttons buttons;
OrangutanAnalog analog;
OrangutanMotors motors;

void setup() {
  pinMode(io0, OUTPUT);
  lcd.clear();
  lcd.print("Pronto");
}

void entrarEmExecucao() {
  execucao = 1;

  leds.green(HIGH);
  leds.red(HIGH);
  digitalWrite(io0, HIGH);

  lcd.clear();
  lcd.print("Execucao");
}

void sairDeExecucao() {
  execucao = 0;

  motors.setSpeeds(0, 0);

  leds.green(LOW);
  leds.red(LOW);
  digitalWrite(io0, LOW);

  lcd.clear();
  lcd.print("Parado");
}

void lerBotao() {

  unsigned char estadoBotao = buttons.isPressed(ANY_BUTTON);

  if (estadoBotao == BOTTOM_BUTTON) {
    entrarEmExecucao();
  } else if (estadoBotao == MIDDLE_BUTTON) {
    sairDeExecucao();
  }
}

void andar() {
  int pot = analog.readTrimpot();  // determine the trimpot position
  int motorSpeed = pot / 2 - 220;  // turn pot reading into number between -256 and 255
  if (motorSpeed == -256)
    motorSpeed = -50;  // 256 is out of range
  motors.setSpeeds(motorSpeed, motorSpeed);
}

void loop() {
  lerBotao();

  if (execucao == 1) {
    andar();
  }
}
