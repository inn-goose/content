// ========================================
// !!! REDEFINE FOR YOUR WIRING SCHEME
// ========================================

// PIN -- !BSY -- |    | -- VCC  -- VCC
// PIN --  A12 -- |    | -- !WE  -- PIN
// PIN --   A7 -- |    | -- NC   -- –––
// PIN --   A6 -- |    | -- A8   -- PIN
// PIN --   A5 -- |    | -- A9   -- PIN
// PIN --   A4 -- |    | -- A11  -- PIN
// PIN --   A3 -- |    | -- !OE  -- PIN
// PIN --   A2 -- |    | -- A10  -- PIN
// PIN --   A1 -- |    | -- !CE  -- PIN
// PIN --   A0 -- |    | -- IO7  -- PIN
// PIN --  IO0 -- |    | -- IO6  -- PIN
// PIN --  IO1 -- |    | -- IO5  -- PIN
// PIN --  IO2 -- |    | -- IO4  -- PIN
// GND --  GND -- |    | -- IO3  -- PIN

// wire pins from 0 to N
// const int ADDRESS_PINS[] = { A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12 };
// const int DATA_PINS[] = { IO0, IO1, IO2, IO3, IO4, IO5, IO6, IO7 };

const int ADDR_BUS_SIZE = 13;
const int ADDRESS_PINS[] = { A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, 64, 65, 66 };

const int DATA_BUS_SIZE = 8;
const int DATA_PINS[] = { 2, 3, 4, 5, 6, 7, 8, 9 };

void setup() {
  Serial.begin(9600);

  // configure address and data pins
  for (int i = 0; i < ADDR_BUS_SIZE; i++) {
    pinMode(ADDRESS_PINS[i], OUTPUT);
  }
  for (int i = 0; i < DATA_BUS_SIZE; i++) {
    pinMode(DATA_PINS[i], INPUT);
  }

  // set address pins to statis output
  int addressBin[] = { 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0 };
  for (int i = 0; i < ADDR_BUS_SIZE; i++) {
    digitalWrite(ADDRESS_PINS[i], addressBin[i]);
  }
}

void loop() {
  delay(1000);

  // read data bus in the loop
  Serial.print("data[LSB]: b");
  for (int i = 0; i < DATA_BUS_SIZE; i++) {
    int data = digitalRead(DATA_PINS[i]) ? 1 : 0;
    Serial.print(data);
  }
  Serial.println();
}
