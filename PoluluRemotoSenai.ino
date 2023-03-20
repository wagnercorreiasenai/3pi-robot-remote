#include <OrangutanLEDs.h>
#include <OrangutanAnalog.h>
#include <OrangutanMotors.h>
#include <OrangutanLCD.h>
#include <OrangutanLEDs.h>
#include <OrangutanPushbuttons.h>
#include <OrangutanBuzzer.h>

//Principais variáveis
boolean execucao = false;
boolean tocarMusica = false;
boolean frente = false;

int velocidade = 0;
unsigned long tempoLimiteAndar = 0;

int io0 = 0;
int io1 = 1;

char bufferSerial[100];
int idxBufferSerial = 0;

#define MELODY_LENGTH 95

unsigned char note[MELODY_LENGTH] = {
  NOTE_E(5), SILENT_NOTE, NOTE_E(5), SILENT_NOTE,
  NOTE_E(5), SILENT_NOTE, NOTE_C(5), NOTE_E(5),
  NOTE_G(5), SILENT_NOTE, NOTE_G(4), SILENT_NOTE,

  NOTE_C(5), NOTE_G(4), SILENT_NOTE, NOTE_E(4), NOTE_A(4),
  NOTE_B(4), NOTE_B_FLAT(4), NOTE_A(4), NOTE_G(4),
  NOTE_E(5), NOTE_G(5), NOTE_A(5), NOTE_F(5), NOTE_G(5),
  SILENT_NOTE, NOTE_E(5), NOTE_C(5), NOTE_D(5), NOTE_B(4),

  NOTE_C(5), NOTE_G(4), SILENT_NOTE, NOTE_E(4), NOTE_A(4),
  NOTE_B(4), NOTE_B_FLAT(4), NOTE_A(4), NOTE_G(4),
  NOTE_E(5), NOTE_G(5), NOTE_A(5), NOTE_F(5), NOTE_G(5),
  SILENT_NOTE, NOTE_E(5), NOTE_C(5), NOTE_D(5), NOTE_B(4),

  SILENT_NOTE, NOTE_G(5), NOTE_F_SHARP(5), NOTE_F(5),
  NOTE_D_SHARP(5), NOTE_E(5), SILENT_NOTE, NOTE_G_SHARP(4),
  NOTE_A(4), NOTE_C(5), SILENT_NOTE, NOTE_A(4), NOTE_C(5), NOTE_D(5),

  SILENT_NOTE, NOTE_G(5), NOTE_F_SHARP(5), NOTE_F(5),
  NOTE_D_SHARP(5), NOTE_E(5), SILENT_NOTE,
  NOTE_C(6), SILENT_NOTE, NOTE_C(6), SILENT_NOTE, NOTE_C(6),

  SILENT_NOTE, NOTE_G(5), NOTE_F_SHARP(5), NOTE_F(5),
  NOTE_D_SHARP(5), NOTE_E(5), SILENT_NOTE,
  NOTE_G_SHARP(4), NOTE_A(4), NOTE_C(5), SILENT_NOTE,
  NOTE_A(4), NOTE_C(5), NOTE_D(5),

  SILENT_NOTE, NOTE_E_FLAT(5), SILENT_NOTE, NOTE_D(5), NOTE_C(5)
};

unsigned int duration[MELODY_LENGTH] = {
  100, 25, 125, 125, 125, 125, 125, 250, 250, 250, 250, 250,

  375, 125, 250, 375, 250, 250, 125, 250, 167, 167, 167, 250, 125, 125,
  125, 250, 125, 125, 375,

  375, 125, 250, 375, 250, 250, 125, 250, 167, 167, 167, 250, 125, 125,
  125, 250, 125, 125, 375,

  250, 125, 125, 125, 250, 125, 125, 125, 125, 125, 125, 125, 125, 125,

  250, 125, 125, 125, 250, 125, 125, 200, 50, 100, 25, 500,

  250, 125, 125, 125, 250, 125, 125, 125, 125, 125, 125, 125, 125, 125,

  250, 250, 125, 375, 500
};

unsigned char currentIdx = 0;

OrangutanLCD lcd;
OrangutanLEDs leds;
OrangutanPushbuttons buttons;
OrangutanAnalog analog;
OrangutanMotors motors;
OrangutanBuzzer buzzer;

void setup() {
  Serial.begin(9600);
  lcd.clear();
  lcd.print("Pronto");
}

void entrarEmExecucao() {
  execucao = true;

  leds.green(HIGH);

  lcd.clear();
  lcd.print("Execucao");
}

void sairDeExecucao() {
  execucao = false;
  frente = false;

  motors.setSpeeds(0, 0);

  leds.green(LOW);

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

void lerSerial() {
  if (Serial.available()) {
    char texto = Serial.read();
    gerenciarBuffer(texto);
  }
}

void gerenciarBuffer(char c) {

  bool zerarBuffer = false;

  //Incrementa o buffer
  if (c != '\n') {
    bufferSerial[idxBufferSerial] = c;
    idxBufferSerial++;
    zerarBuffer = false;
  } else {
    zerarBuffer = true;
  }

  if (zerarBuffer) {
    analisarComando(bufferSerial);
    memset(bufferSerial, 0, strlen(bufferSerial));
    idxBufferSerial = 0;
  }
}

void analisarComando(char comando[100]) {
  andar(comando);
  tocarBuzzer(comando);
}

void andar(char comando[100]) {
  if (comando[0] == 'a' && comando[1] == 'n' && comando[2] == 'd') {
    lcd.clear();
    lcd.print("Andar");

    if (comando[3] == 'f') {
      frente = true;
      velocidade = comando[4] - '0';

      if (velocidade < 1) {
        velocidade = 1;
      }

      unsigned long segundoDesejado = comando[5] - '0';

      if (segundoDesejado > 1) {
        segundoDesejado--;
      }

      tempoLimiteAndar = millis() + (segundoDesejado * 1000);

      lcd.clear();
      lcd.print("vel.=");
      lcd.print(velocidade);
      lcd.gotoXY(0, 1);
      lcd.print("tem.=");
      lcd.print(segundoDesejado);
    }

    if (comando[3] == 'd' || comando[3] == 'e') {
      int tempo = comando[4] - '0';
      tempo *= 100;
      fazerCurva(tempo, comando[3]);
    }

    if (comando[3] == 'g') {
      girar180();
    }
  }
}

void girar180() {
  lcd.clear();
  lcd.print("Girar");

  int forcaMotor = 20;

  motors.setSpeeds(forcaMotor, (forcaMotor * -1));
  delay(2500);
  motors.setSpeeds(0, 0);
}

void fazerCurva(int tempo, char sentido) {
  lcd.clear();

  int forcaMotorEsquerdo = 50;
  int forcaMotorDireito = 50;

  if (sentido == 'd') {
    forcaMotorDireito *= -1;
    lcd.print("Direita");
  } else if (sentido == 'e') {
    forcaMotorEsquerdo *= -1;
    lcd.print("Esquerda");
  }

  int pot = analog.readTrimpot();
  motors.setSpeeds(forcaMotorEsquerdo, forcaMotorDireito);
  delay(tempo);
  motors.setSpeeds(0, 0);
}

void paraFrente() {
  if (frente) {
    if (millis() < tempoLimiteAndar) {
      int pot = analog.readTrimpot();

      int motorSpeed = velocidade * 10;

      if (motorSpeed >= 100) {
        motorSpeed = 0;
      }

      motors.setSpeeds(motorSpeed, motorSpeed);
    } else {
      motors.setSpeeds(0, 0);
      frente = false;
      lcd.clear();
      lcd.print("Parou");
    }
  }
}

void tocarBuzzer(char comando[100]) {
  if (comando[0] == 'b' && comando[1] == 'u' && comando[2] == 'z') {
    tocarMusica = true;
    lcd.clear();
    lcd.print("Buzzer");
  }
}

void executarMusica() {
  if (tocarMusica) {
    if (currentIdx < MELODY_LENGTH && !buzzer.isPlaying()) {
      // play note at max volume
      buzzer.playNote(note[currentIdx], duration[currentIdx], 15);
      currentIdx++;
    }

    if (currentIdx >= MELODY_LENGTH) {
      currentIdx = 0;
      tocarMusica = false;
    }
  }
}

void loop() {
  lerBotao();
  lerSerial();

  //Comandos assync
  executarMusica();
  paraFrente();
}