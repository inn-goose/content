const int PIN = 5;

void setup() {
  pinMode(PIN, OUTPUT);
}

void loop() {
  // the most basic pin on/off gives one square wave cycle _|¯|
  digitalWrite(PIN, !digitalRead(PIN));
}
