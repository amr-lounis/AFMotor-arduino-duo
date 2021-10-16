#include "Arduino.h"
//****************************************************************************************************************************
//Define which pins will be used for the Shift Register control
#define PIN_DATA 8
#define PIN_LATCH 12
#define PIN_CLOCK 4
#define PIN_SHIFT_ENABLE 7

//Define which pins will be used for  WPM
const uint8_t PINS_PWM[4] = { 11,3,6,5 };
// -255 < value < 255
uint8_t pins_pwm_value[4] = { 0,0,0,0 };
// 0 or 1 or 2
uint8_t port_state[4] = { 0,0,0,0 }; // 0b00=BRAKE or 0b01=FORWARD or 0b10=BACKWARD
//****************************************************************************************************************************
// Bit positions in the 74HCT595 shift register output
const uint8_t PINS_IC[] = { 2,3,1,4,5,7,0,6 };
// convert virtuel position real position in  the 74HCT595 shift register output
uint8_t v2oPin(uint8_t p_data) {
    uint8_t _data_temp = 0b00000000;
    for (uint8_t i = 0; i < 8; i++) _data_temp = (((p_data >> i) & 1) << PINS_IC[i]) | _data_temp;
    return _data_temp;
}
//****************************************************************************************************************************
void initialise() {
    pinMode(PIN_DATA, OUTPUT);
    pinMode(PIN_LATCH, OUTPUT);
    pinMode(PIN_CLOCK, OUTPUT);

    pinMode(PIN_SHIFT_ENABLE, OUTPUT);
    // enable 74HCT595
    digitalWrite(PIN_SHIFT_ENABLE, LOW);
    // Initialise PWM
    for (int i = 0; i < 4; i++) pinMode(PINS_PWM[i], OUTPUT);
}
void setPorts() {
    uint8_t data_virtuel = 0b00000000;
    for (int i = 0; i < 4; i++) data_virtuel |= port_state[i] << (i * 2);
    uint8_t data_real = v2oPin(data_virtuel);
    //_debug(data_virtuel, data_real);
    digitalWrite(PIN_LATCH, LOW);                               // Set latch LOW to start sending data
    shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, data_real);  // Send the data
    digitalWrite(PIN_LATCH, HIGH);                              // Set latch HIGH to end.
    for (int i = 0; i < 4; i++)  analogWrite(PINS_PWM[i], pins_pwm_value[i]);
}
void DC_Set(uint8_t p_motor, int p_speed) {
    if (p_speed > 0) { port_state[p_motor] = 1; } // FORWARD
    else if (p_speed < 0) { port_state[p_motor] = 2; } // BACKWARD
    else { port_state[p_motor] = 0; } // BRAKE
    pins_pwm_value[p_motor] = abs(p_speed);
    setPorts();
}
void StepperRunOne(int p_port, int p_setup, int p_delay, uint8_t p_pwm) {
    p_setup = abs(p_setup % 8);
    switch (p_setup) {
    case 0: {port_state[p_port * 2 + 1] = 0b00; port_state[p_port] = 0b01; } break;
    case 1: {port_state[p_port * 2 + 1] = 0b01; port_state[p_port] = 0b01; } break;
    case 2: {port_state[p_port * 2 + 1] = 0b01; port_state[p_port] = 0b00; } break;
    case 3: {port_state[p_port * 2 + 1] = 0b01; port_state[p_port] = 0b10; } break;
    case 4: {port_state[p_port * 2 + 1] = 0b00; port_state[p_port] = 0b10; } break;
    case 5: {port_state[p_port * 2 + 1] = 0b10; port_state[p_port] = 0b10; } break;
    case 6: {port_state[p_port * 2 + 1] = 0b10; port_state[p_port] = 0b00; } break;
    case 7: {port_state[p_port * 2 + 1] = 0b10; port_state[p_port] = 0b01; } break;
    default:break;
    }
    pins_pwm_value[p_port * 2 + 1] = p_pwm; pins_pwm_value[p_port * 2] = p_pwm;
    setPorts();
    delay(p_delay);
    //--
    pins_pwm_value[p_port * 2 + 1] = 0x00; pins_pwm_value[p_port * 2] = 0x00;
    setPorts();
}

void StepperRun(int p_port, int p_setup, int p_delay, uint8_t p_pwm) {
    if (p_setup > 0) for (int i = 0; i < p_setup; i++) StepperRunOne(0, i, p_delay, p_pwm);
    else if (p_setup < 0) for (int i = abs(p_setup); i > 0; i--)  StepperRunOne(0, i, p_delay, p_pwm);
}

//------------------------------------------------------------- init
void setup()
{
    initialise();
    Serial.begin(9600);
}
//------------------------------------------------------------- loop
void loop()
{
    //for (int i = 0; i < 4; i++) {
    //    DC_Set(i, 255);
    //    delay(1000);
    //    DC_Set(i, -255);
    //    delay(1000);
    //    DC_Set(i, 0);
    //    delay(1000);
    //}

    StepperRun(0, 400, 5, 255);
    delay(2000);
    StepperRun(0, -400, 5, 255);
    delay(2000);
}
//------------------------------------------------------------- debug
void _printBinary(uint8_t p_data) {
    for (int i = 0; i < 8; i++)
    {
        bool b = p_data & 0x80;
        p_data = p_data << 1;
        Serial.print(b);
    }
}


void _debug(uint8_t p_data1, uint8_t p_data2) {
    Serial.print("virtual : ( ");
    _printBinary(p_data1);
    Serial.print(" ) => real : ( ");
    _printBinary(p_data2);
    Serial.print(" ) \n");
}