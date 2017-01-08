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

	pinMode(trig_pin, OUTPUT);
	pinMode(echo_pin, INPUT);

}

const int speed = 80;

void go_forward() {
	digitalWrite(A1, LOW);
	digitalWrite(A2, HIGH);
	digitalWrite(A3, LOW);
	digitalWrite(1, HIGH);
	/*
	analogWrite(9, 230);
	analogWrite(10, 217);
	*/
	analogWrite(9, speed);
	analogWrite(10, speed);
}

void go_backward() {
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

void stop() {
	digitalWrite(A1, LOW);
	digitalWrite(A2, LOW);
	digitalWrite(A3, LOW);
	digitalWrite(1, LOW);
	analogWrite(9, 0);
	analogWrite(10, 0);
}

unsigned long get_distance() {
	digitalWrite(trig_pin, LOW);
	delayMicroseconds(2);
	digitalWrite(trig_pin, HIGH);
	delayMicroseconds(5);
	digitalWrite(trig_pin, LOW);

	unsigned long echo = pulseIn(echo_pin, HIGH);
	return (1.0 * echo / 58.138);
}

void turnLeft() {
	digitalWrite(A1, HIGH);
	digitalWrite(A2, LOW);
	digitalWrite(A3, LOW);
	digitalWrite(1, HIGH);
	analogWrite(9, speed);
	analogWrite(10, speed);
}

void turnRight() {
	digitalWrite(A1, LOW);
	digitalWrite(A2, HIGH);
	digitalWrite(A3, HIGH);
	digitalWrite(1, LOW);
	analogWrite(9, speed);
	analogWrite(10, speed);
}

void loop()
{
	unsigned long distance = get_distance();
	if (distance < 5) {
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
	} else {
		go_forward();
	}
}

// vim: ai cin
