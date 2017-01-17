#include <Wire.h>

#define _X 0
#define _Y 0
#define _Z 0

#define red 13
#define yellow 12

byte read(int reg)
{
	Wire.beginTransmission(0x68);
	Wire.write(reg);
	Wire.endTransmission(false);
	Wire.requestFrom(0x68, 1, false);
	byte val = Wire.read();
	Wire.endTransmission(true);
	return val;
}

void write(int reg, int data)
{
	Wire.beginTransmission(0x68);
	Wire.write(reg);
	Wire.write(data);
	Wire.endTransmission(true);
}

byte readMag(int reg)
{
	Wire.beginTransmission(0x0C);
	Wire.write(reg);
	Wire.endTransmission(false);
	Wire.requestFrom(0x0C, 1, false);
	byte val = Wire.read();
	Wire.endTransmission(true);
	return val;
}

void writeMag(int reg, int data)
{
	Wire.beginTransmission(0x0C);
	Wire.write(reg);
	Wire.write(data);
	Wire.endTransmission(true);
}

double scalarMult(double a[3], double b[3])
{
	double res = 0;
	for (int i = 0; i < 3; ++i) {
		res += a[i] * b[i];
	}
	return res;
}

void vectorMult(double a[3], double b[3], double res[3])
{
	for (int i = 0; i < 3; ++i) {
		res[i] = a[(i + 1) % 3] * b[(i + 2) % 3] - a[(i + 2) % 3] * b[(i + 1) % 3];
	}
}

double tot[3]; // x, y, z

bool curAvailable;
bool sAvailable;
double cur[3]; // x, y, z
double now[3]; // x, y, z

int magMeasurementNumber;

bool calibrated;

double b1[3]; // x, y, z
double b2[3]; // x, y, z
double bb[3];

void doMeasurement(int measureCount)
{
	int xh = readMag(0x04);
	int xl = readMag(0x03);
	int yh = readMag(0x06);
	int yl = readMag(0x05);
	int zh = readMag(0x08);
	int zl = readMag(0x07);

	readMag(0x09);

	double x = (xh << 8) | (xl & 0xff);
	double y = (yh << 8) | (yl & 0xff);
	double z = (zh << 8) | (zl & 0xff);

	if (magMeasurementNumber < measureCount) {
		tot[_X] += x;
		tot[_Y] += y;
		tot[_Z] += z;
		++magMeasurementNumber;
	} else {
		for (int i = 0; i < 3; ++i) {
			now[i] = tot[i] / measureCount;
		}
		tot[_X] = x;
		tot[_Y] = y;
		tot[_Z] = z;
		magMeasurementNumber = 1;
		sAvailable = true;
	}
}

#define trig_pin 7
#define echo_pin 8

void setup()
{
	pinMode(A1, OUTPUT);
	pinMode(A2, OUTPUT);
	pinMode(A3, OUTPUT);
	pinMode(1, OUTPUT);
	pinMode(9, OUTPUT);
	pinMode(10, OUTPUT);
	pinMode(red, OUTPUT);
	pinMode(yellow, OUTPUT);

	pinMode(trig_pin, OUTPUT);
	pinMode(echo_pin, INPUT);

	Wire.begin();
	write(0x6B, 0);
	write(0x6A, 0);
	write(0x37, 0x02);
	writeMag(0x0A, 0x12);
	for (int i = 0; i < 3; ++i) {
		tot[i] = 0;
	}
	magMeasurementNumber = 0;
	curAvailable = false;
	sAvailable = false;
	calibrated = false;
}

const int speed = 80;

void go_forward()
{
	digitalWrite(A1, LOW);
	digitalWrite(A2, HIGH);
	digitalWrite(A3, LOW);
	digitalWrite(1, HIGH);
	/*
	analogWrite(9, 230);
	analogWrite(10, 217);
	*/
	if (curAvailable) {
		double dir[3];
		vectorMult(cur, now, dir);
		double sm = scalarMult(dir, bb);
		if (sm > 0) {
			analogWrite(9, speed);
			analogWrite(10, speed - 5);
		} else if (sm < 0) {
			analogWrite(9, speed - 5);
			analogWrite(10, speed);
		} else {
			analogWrite(9, speed);
			analogWrite(10, speed);
		}
	} else {
		analogWrite(9, speed);
		analogWrite(10, speed);
	}
}

void go_backward()
{
	digitalWrite(A1, HIGH);
	digitalWrite(A2, LOW);
	digitalWrite(A3, HIGH);
	digitalWrite(1, LOW);
	/*
	analogWrite(9, 230);
	analogWrite(10, 217);
	*/
	analogWrite(9, speed);
	analogWrite(10, speed);
}

void stop()
{
	digitalWrite(A1, LOW);
	digitalWrite(A2, LOW);
	digitalWrite(A3, LOW);
	digitalWrite(1, LOW);
	analogWrite(9, 0);
	analogWrite(10, 0);
}

unsigned long get_distance()
{
	digitalWrite(trig_pin, LOW);
	delayMicroseconds(2);
	digitalWrite(trig_pin, HIGH);
	delayMicroseconds(5);
	digitalWrite(trig_pin, LOW);

	unsigned long echo = pulseIn(echo_pin, HIGH);
	return (1.0 * echo / 58.138);
}

void turnLeft()
{
	digitalWrite(A1, HIGH);
	digitalWrite(A2, LOW);
	digitalWrite(A3, LOW);
	digitalWrite(1, HIGH);
	analogWrite(9, speed);
	analogWrite(10, speed);
}

void turnRight()
{
	digitalWrite(A1, LOW);
	digitalWrite(A2, HIGH);
	digitalWrite(A3, HIGH);
	digitalWrite(1, LOW);
	analogWrite(9, speed);
	analogWrite(10, speed);
}

void doCalibrate()
{
	signal(yellow, 3);
	on(yellow);
	for (int i = 0; i < 502; ++i) {
		doMeasurement(500);
		delay(1);
	}

	for (int i = 0; i < 3; ++i) {
		b1[i] = now[i];
	}
	signal(yellow, 2);
	turnLeft();
	delay(300);
	stop();

	for (int i = 0; i < 502; ++i) {
		doMeasurement(500);
		delay(1);
	}

	for (int i = 0; i < 3; ++i) {
		b2[i] = now[i];
	}

	signal(yellow, 1);
	vectorMult(b1, b2, bb);
	off(yellow);
}

void signal(int pin, int times) {
	for (int i = 0; i < times; ++i) {
		digitalWrite(pin, HIGH);
		delay(300);
		digitalWrite(pin, LOW);
		delay(300);
	}
}

void on(int pin) {
	digitalWrite(pin, HIGH);
}

void off(int pin) {
	digitalWrite(pin, LOW);
}

void loop()
{
	if (!calibrated) {
		doCalibrate();
		calibrated = true;
	}
	doMeasurement(500);

	if (sAvailable) {
		for (int i = 0; i < 3; ++i) {
			cur[i] = now[i];
		}
		curAvailable = true;
	}

	unsigned long distance = get_distance();
	if (distance < 8) {
		on(red);
		stop();
		turnLeft();
		delay(500);
		stop();
		unsigned long leftDistance = get_distance();
		turnRight();
		delay(1000);
		stop();
		unsigned long rightDistance = get_distance();
		if (rightDistance < leftDistance) {
			turnLeft();
			delay(1000);
			stop();
		}
		off(red);
	} else {
		go_forward();
	}
}

// vim: ai cin
