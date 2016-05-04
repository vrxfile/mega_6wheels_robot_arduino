/*
  Робот 6-колесная тележка DAGU на Arduino
  Created by Rostislav Varzar
*/

#include <SoftwareSerial.h>

// Определения для портов
#define CRASHSENSOR1 A2

#define US1_trigPin A4
#define US1_echoPin A5
#define US2_trigPin 10
#define US2_echoPin 11
#define US3_trigPin 8
#define US3_echoPin 9
#define US4_trigPin 2
#define US4_echoPin 3
#define minimumRange 0
#define maximumRange 200

#define PWMA 5
#define DIRA 4
#define PWMB 7
#define DIRB 6

// Параметры ПИД регулятора
#define KPID 0.75

// Параметры дистанций
#define DIST1 20
#define DIST2 45
#define DIST4 2

// Параметры моторов
#define MPWR 30
#define LLLL 0

// Программный UART для Bluetooth
//SoftwareSerial BT(A0, A1);

// UART1 для Bluetooth
#define BT Serial1

// Реле управления фарами
#define FORWARD_LAMPS 13

// Переменные для управления роботом
char command = 'S';
char prevCommand = 'A';
int velocity = 0;
unsigned long timer0 = 2000;
unsigned long timer1 = 0;

// Задержка между считываниями датчиков
#define USDELAY 15

int flag = 0;
long left1 = 0;
long right1 = 0;
long oldleft1 = 0;
long oldright1 = 0;
long left2 = 0;
long right2 = 0;
long oldleft2 = 0;
long oldright2 = 0;
long colcounter = 0;
float u = 0;

void setup()
{
  // Инициализация последовательного порта
  Serial.begin(9600);

  // Инициализация последовательного порта для Bluetooth
  BT.begin(9600);

  // Инициализация выводов для работы с УЗ датчиком
  /*
    pinMode(US1_trigPin, OUTPUT);
    pinMode(US1_echoPin, INPUT);
    pinMode(US2_trigPin, OUTPUT);
    pinMode(US2_echoPin, INPUT);
    pinMode(US3_trigPin, OUTPUT);
    pinMode(US3_echoPin, INPUT);
    pinMode(US4_trigPin, OUTPUT);
    pinMode(US4_echoPin, INPUT);
  */

  // Инициализация выходов для управления моторами
  pinMode(DIRA, OUTPUT);
  pinMode(DIRB, OUTPUT);

  // Инициализация выхода для управления фарами
  digitalWrite(FORWARD_LAMPS, false);
  pinMode(FORWARD_LAMPS, OUTPUT);
}

void loop()
{

  /*
    // Тестирование датчиков
    while (true)
    {
      Serial.print(readUS1_distance());
      Serial.print("\t");
      delay(25);
      Serial.print(readUS2_distance());
      Serial.print("\t");
      delay(25);
      Serial.print(readUS3_distance());
      Serial.print("\t");
      delay(25);
      Serial.print(readUS4_distance());
      Serial.print("\t");
      delay(25);
      Serial.println("");
    }
  */

  /*
    motorA_setpower(-100, false);
    motorB_setpower(-100, false);
    delay(1000);
    motorA_setpower(-1, false);
    motorB_setpower(-1, false);
    delay(1000);
    while (true)
    {

    }
  */

  // Работа по Bluetooth
  //while (digitalRead(CRASHSENSOR1))
  while (true)
  {
    // Bluetooth
    if (BT.available() > 0) {
      timer1 = millis();
      prevCommand = command;
      command = BT.read();
      if (command != prevCommand) {
        Serial.println(command);
        switch (command) {
          case 'F':
            motorA_setpower(velocity + LLLL, false);
            motorB_setpower(velocity, false);
            break;
          case 'B':
            motorA_setpower(-velocity - LLLL, false);
            motorB_setpower(-velocity, false);
            break;
          case 'R':
            motorA_setpower(-velocity - LLLL, false);
            motorB_setpower(velocity, false);
            break;
          case 'L':
            motorA_setpower(velocity + LLLL, false);
            motorB_setpower(-velocity, false);
            break;
          case 'S':
            motorA_setpower(-1, false);
            motorB_setpower(-1, false);
            break;
          case 'G':  //FR
            //yellowCar.ForwardRight_4W(velocity);
            motorA_setpower(velocity + LLLL, false);
            motorB_setpower(velocity / 2, false);
            break;
          case 'H':  //BR
            motorA_setpower(-velocity - LLLL, false);
            motorB_setpower(-velocity / 2, false);
            break;
          case 'I':  //FL
            motorA_setpower((velocity  + LLLL) / 2, false);
            motorB_setpower(velocity, false);
            break;
          case 'J':  //BL
            motorA_setpower((-velocity - LLLL) / 2, false);
            motorB_setpower(-velocity, false);
            break;
          case 'W':
            digitalWrite(FORWARD_LAMPS, true);
            break;
          case 'w':
            digitalWrite(FORWARD_LAMPS, false);
            break;
          case 'U':
            break;
          case 'u':
            break;
          case 'V':
            break;
          case 'v':
            break;
          case 'X':
            break;
          case 'x':
            break;
          case 'D':  //Everything OFF
            motorA_setpower(-1, false);
            motorB_setpower(-1, false);
            break;
          default:  //Get velocity
            if (command == 'q') {
              velocity = 100;  //Full velocity
            }
            else {
              //Chars '0' - '9' have an integer equivalence of 48 - 57, accordingly.
              if ((command >= 48) && (command <= 57)) {
                //Subtracting 48 changes the range from 48-57 to 0-9.
                //Multiplying by 25 changes the range from 0-9 to 0-225.
                velocity = (command - 48) * 10;
                Serial.println(velocity);
              }
            }
        }
      }
    }
    else {
      timer0 = millis();  //Get the current time (millis since execution started).
      //Check if it has been 500ms since we received last command.
      if ((timer0 - timer1) > 500) {
        //More tan 500ms have passed since last command received, car is out of range.
        //Therefore stop the car and turn lights off.
        motorA_setpower(-1, false);
        motorB_setpower(-1, false);
      }
    }
  }
  delay(5000);

}

// УЗ датчик 1
long readUS1_distance()
{
  long duration = 0;
  long distance = 0;
  digitalWrite(US1_trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(US1_trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(US1_trigPin, LOW);
  duration = pulseIn(US1_echoPin, HIGH, 50000);
  distance = duration / 58.2;
  if (distance >= maximumRange || distance <= minimumRange) {
    distance = -1;
  }
  return distance;
}

// УЗ датчик 2
long readUS2_distance()
{
  long duration = 0;
  long distance = 0;
  digitalWrite(US2_trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(US2_trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(US2_trigPin, LOW);
  duration = pulseIn(US2_echoPin, HIGH, 50000);
  distance = duration / 58.2;
  if (distance >= maximumRange || distance <= minimumRange) {
    distance = -1;
  }
  return distance;
}

// УЗ датчик 3
long readUS3_distance()
{
  long duration = 0;
  long distance = 0;
  digitalWrite(US3_trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(US3_trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(US3_trigPin, LOW);
  duration = pulseIn(US3_echoPin, HIGH, 50000);
  distance = duration / 58.2;
  if (distance >= maximumRange || distance <= minimumRange) {
    distance = -1;
  }
  return distance;
}

// УЗ датчик 4
long readUS4_distance()
{
  long duration = 0;
  long distance = 0;
  digitalWrite(US4_trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(US4_trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(US4_trigPin, LOW);
  duration = pulseIn(US4_echoPin, HIGH, 50000);
  distance = duration / 58.2;
  if (distance >= maximumRange || distance <= minimumRange) {
    distance = -1;
  }
  return distance;
}

// Мощность мотора "A" от -100% до +100% (от знака зависит направление вращения)
void motorA_setpower(int pwr, bool invert)
{
  // Проверка, инвертирован ли мотор
  if (invert)
  {
    pwr = -pwr;
  }
  // Проверка диапазонов
  if (pwr < -100)
  {
    pwr = -100;
  }
  if (pwr > 100)
  {
    pwr = 100;
  }
  // Установка направления
  if (pwr < 0)
  {
    digitalWrite(DIRA, LOW);
  }
  else
  {
    pwr = 100 - pwr; // Для Arduino Mega
    digitalWrite(DIRA, HIGH);
  }
  // Установка мощности
  int pwmvalue = abs(pwr) * 2.55;
  analogWrite(PWMA, pwmvalue);
}

// Мощность мотора "B" от -100% до +100% (от знака зависит направление вращения)
void motorB_setpower(int pwr, bool invert)
{
  // Проверка, инвертирован ли мотор
  if (invert)
  {
    pwr = -pwr;
  }
  // Проверка диапазонов
  if (pwr < -100)
  {
    pwr = -100;
  }
  if (pwr > 100)
  {
    pwr = 100;
  }
  // Установка направления
  if (pwr < 0)
  {
    digitalWrite(DIRB, LOW);
  }
  else
  {
    pwr = 100 - pwr; // Для Arduino Mega
    digitalWrite(DIRB, HIGH);
  }
  // Установка мощности
  int pwmvalue = abs(pwr) * 2.55;
  analogWrite(PWMB, pwmvalue);
}

