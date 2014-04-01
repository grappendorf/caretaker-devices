/**
 * Firmware for the Reflow Oven Controller
 *
 * Based on code from Stanislav Yanov,
 * http://www.kukata86.com/en/what-is-reflow-oven-controller
 *
 * Processor ATmega328P 16 MHz 5V
 *
 * Low Fuse:		0xFF
 * High Fuse:		0xD9
 * Extended Fuse:	0xFF
 *
 * Reflow process:
 *
 * Temperature [°C]
 *
 * 225-|                                               x  x
 *     |                                            x        x
 *     |                                         x              x
 *     |                                      x                    x
 * 200-|                                   x                          x
 *     |                              x    |                          |   x
 *     |                         x         |                          |       x
 *     |                    x              |                          |
 * 150-|               x                   |                          |
 *     |             x |                   |                          |
 *     |           x   |                   |                          |
 *     |         x     |                   |                          |
 *     |       x       |                   |                          |
 *     |     x         |                   |                          |
 *     |   x           |                   |                          |
 * 30 -| x             |                   |                          |
 *     |<  60 - 90 s  >|<    90 - 120 s   >|<       90 - 120 s       >|
 *     | Preheat Stage |   Soaking Stage   |       Reflow Stage       | Cool
 *  0  |_ _ _ _ _ _ _ _|_ _ _ _ _ _ _ _ _ _|_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
 *                                                                      Time [s]
 */

/**
 * If you want to use this code inside the Arduino IDE, remove the Arduino.h
 * include and delete the path prefixes from the other includes.
 */

#include <Arduino.h>
#include <LiquidCrystal/LiquidCrystal.h>
#include <Bounce/Bounce.h>
#include <PID/PID.h>
#include <Adafruit_MAX31855/Adafruit_MAX31855.h>

// IO pins

const uint8_t LCD_RS = 6;
const uint8_t LCD_EN = 5;
const uint8_t LCD_D4 = 7;
const uint8_t LCD_D5 = 4;
const uint8_t LCD_D6 = 3;
const uint8_t LCD_D1 = 2;
const uint8_t BUTTON_1 = 16;
const uint8_t BUTTON_2 = 17;
const uint8_t BUTTON_3 = 18;
const uint8_t BUTTON_4 = 19;
const uint8_t RELAY = 9;
const uint8_t THERMO_DO = 10;
const uint8_t THERMO_CLK = 15;
const uint8_t THERMO_CS = 14;
const uint8_t FAN = 8;

// LCD

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D1);

uint8_t degreeSymbol[] = { 140, 146, 146, 140, 128, 128, 128, 128 };
uint8_t heaterSymbol[] = { 137, 146, 146, 145, 137, 137, 146, 128 };
uint8_t fanSymbol[] = { 132, 149, 142, 159, 142, 149, 132, 128 };

enum Symbols {
	SYM_DEGREE, SYM_HEATER, SYM_FAN, SYM_ARROW = 0x7E
};

// Buttons

const unsigned long BUTTON_REPEAT_INTERVAL = 200;

Bounce buttonRed(BUTTON_1, 5);
Bounce buttonGreen(BUTTON_2, 5);
Bounce buttonYellowLeft(BUTTON_3, 5);
Bounce buttonYellowRight(BUTTON_4, 5);

unsigned long nextButtonRepeat = 0;

// Temperature sensor

const unsigned long THERMO_READ_INTERVAL = 100;

Adafruit_MAX31855 thermo(THERMO_CLK, THERMO_CS, THERMO_DO);

double temp = 0.0;

boolean tempError = false;

unsigned long nextThermoRead = 0;

const double TEMP_CORRECTION_FACTOR = 1.0;

// PID

const unsigned long PID_SAMPLE_INTERVAL = 1000;

const double PID_KP_PREHEAT = 80;
const double PID_KI_PREHEAT = 0.03;
const double PID_KD_PREHEAT = 20;

const double PID_KP_SOAK = 180;
const double PID_KI_SOAK = 0.5;
const double PID_KD_SOAK = 60;

const double PID_KP_REFLOW = 150;
const double PID_KI_REFLOW = 0.1;
const double PID_KD_REFLOW = 25;

const double REFLOW_TEMP_START = 50;
const double REFLOW_TEMP_SOAK = 140;
const double REFLOW_TEMP_REFLOW = 170;
const double REFLOW_TEMP_MAX = 205;
const double REFLOW_TEMP_MAX_OFFSET = 5;
const double REFLOW_TEMP_COOL = 190;
const double REFLOW_TEMP_END = 60;
const double SOAK_TEMP_STEP = 5;

const double OFF_COOLING_TEMP = 40.0;

const unsigned long SOAK_INTERVAL = 15000;

const double MIN_TEMP = 23.0;
const double MAX_TEMP = 350.0;

double setpoint, input, output;

PID pid(&input, &output, &setpoint, PID_KP_PREHEAT, PID_KI_PREHEAT, PID_KD_PREHEAT, DIRECT);

unsigned long nextSoakUpdate = 0;

unsigned long windowSize = 2500;

unsigned long windowStartTime;

double offCoolingTempTarget = OFF_COOLING_TEMP;

// State machine

enum State {
	STATE_IDLE,
	STATE_ERROR,
	STATE_SET,
	STATE_HEAT,
	STATE_PRECOOL,
	STATE_PREHEAT,
	STATE_SOAK,
	STATE_REFLOW,
	STATE_REFLOW_COOL,
	STATE_COOL,
	STATE_COMPLETE
};

const char* stateNames[] = { "Idle", "Error", "Set", "Heat", "Pre-cool", "Pre-heat", "Soak", "Reflow", "Cool", "Cool",
		"Complete" };

State state = STATE_ERROR;

enum Mode {
	MODE_OFF, MODE_REFLOW, MODE_MANUAL, MODE_COOL
};

const char* modeNames[] = { "Off", "Reflow", "Manual", "Cool" };

Mode mode = MODE_OFF;

// Time measurement

unsigned int elapsedSeconds = 0;

unsigned long nextElapsedSecondsUpdate = 0;

/**
 * Switch the heater on or off.
 * @param on if true, the heater is switched on
 */
void heater(boolean on)
{
	digitalWrite(RELAY, on ? HIGH : LOW);
}

/**
 * Switch the fan on or off.
 * @param on if true, the fan is switched on
 */
void fan(boolean on)
{
	digitalWrite(FAN, on ? HIGH : LOW);
}

/**
 * Update the state of the PID controller.
 */
void updatePID()
{
	input = temp;
	pid.Compute();
	if (millis() > windowStartTime + windowSize) {
		windowStartTime += windowSize;
	}
}

/**
 * Switches the heater on or off depending on the current PWM state, outputted
 * by the PID controller.
 */
void updateHeater()
{
	heater(output > (millis() - windowStartTime));
}

/**
 * Update the LCD display with current information about the state, the tempeature,
 * and so on.
 */
void updateDisplay()
{
	lcd.setCursor(0, 0);

	// Print the temperature
	if (!tempError) {
		lcd.print((int) temp);
		lcd.write((byte) SYM_DEGREE);
	} else {
		lcd.print("N.C.");
	}

	// Print the heater and fan symbols
	if (digitalRead(RELAY) == HIGH) {
		lcd.write(SYM_HEATER);
	} else if (digitalRead(FAN) == HIGH) {
		lcd.write(SYM_FAN);
	} else {
		lcd.write(' ');
	}

	// Print the temperature set point
	if (state == STATE_SET || state == STATE_HEAT || state == STATE_PRECOOL || state == STATE_PREHEAT
			|| state == STATE_SOAK || state == STATE_REFLOW) {
		lcd.write(' ');
		lcd.write(SYM_ARROW);
		lcd.print((int) setpoint);
		lcd.write((byte) 0);
	}

	// Print the elapsed seconds
	if (state == STATE_PREHEAT || state == STATE_SOAK || state == STATE_REFLOW || state == STATE_REFLOW_COOL) {
		lcd.write(' ');
		lcd.print(elapsedSeconds);
		lcd.write('s');
	}
	lcd.print("          ");

	// Print the current mode and state
	lcd.setCursor(0, 1);
	lcd.print(modeNames[mode]);
	lcd.print('(');
	lcd.print(stateNames[state]);
	lcd.print(')');
	lcd.print("        ");
}

/**
 * Switch the fan on and cool until the OFF_COOLING_TEMP is reached.
 */
void offCooling()
{
	heater(false);
	if (temp > OFF_COOLING_TEMP + 5.0) {
		fan(true);
	} else if (temp < OFF_COOLING_TEMP) {
		fan(false);
	}
}

/**
 * Initialization
 */
void setup()
{
	pinMode(BUTTON_1, INPUT);
	pinMode(BUTTON_2, INPUT);
	pinMode(BUTTON_3, INPUT);
	pinMode(BUTTON_4, INPUT);
	pinMode(RELAY, OUTPUT);
	pinMode(FAN, OUTPUT);

	digitalWrite(BUTTON_1, HIGH);
	digitalWrite(BUTTON_2, HIGH);
	digitalWrite(BUTTON_3, HIGH);
	digitalWrite(BUTTON_4, HIGH);

	lcd.begin(16, 2);
	lcd.createChar(SYM_DEGREE, degreeSymbol);
	lcd.createChar(SYM_HEATER, heaterSymbol);
	lcd.createChar(SYM_FAN, fanSymbol);

	pid.SetOutputLimits(0, windowSize);
	pid.SetSampleTime(PID_SAMPLE_INTERVAL);
	pid.SetMode(AUTOMATIC);

	setpoint = 23;
	windowStartTime = millis();
}

/**
 * State machine mode: OFF
 * - Safe mode, do nothing
 * - Switch to next mode on mode button press
 */
void modeOff()
{
	heater(false);
	fan(false);
	if (buttonRed.fallingEdge()) {
		mode = MODE_REFLOW;
	}
}

/**
 * State machine mode: COOL
 * - Cool down
 * - Switch to next mode on mode button press
 */
void modeCool()
{
	offCooling();
	if (buttonRed.fallingEdge()) {
		fan(false);
		mode = MODE_MANUAL;
	}
}

/**
 * State machine mode: MANUAL
 * - Manually set the temperature with the up/down buttons
 * - Switch to next mode on mode button press
 */
void modeManual()
{
	switch (state) {
		case STATE_IDLE:
			heater(false);
			if (buttonRed.fallingEdge()) {
				mode = MODE_OFF;
			}
			if (buttonGreen.fallingEdge()) {
				state = STATE_SET;
			}
			break;
		case STATE_SET:
			if (buttonRed.fallingEdge()) {
				state = STATE_IDLE;
			}
			if (buttonGreen.fallingEdge()) {
				pid.SetTunings(PID_KP_REFLOW, PID_KI_REFLOW, PID_KD_REFLOW);
				state = STATE_HEAT;
			}
			if (buttonYellowLeft.fallingEdge()) {
				setpoint = min(setpoint + 1, MAX_TEMP);
			}
			if (buttonYellowLeft.read() == LOW && buttonYellowLeft.duration() >= BUTTON_REPEAT_INTERVAL) {
				if (millis() > nextButtonRepeat) {
					setpoint = min(setpoint + 1, MAX_TEMP);
					nextButtonRepeat = millis() + BUTTON_REPEAT_INTERVAL;
				}
			}
			if (buttonYellowRight.fallingEdge()) {
				setpoint = max(setpoint - 1, 0);
			}
			if (buttonYellowRight.read() == LOW && buttonYellowRight.duration() >= BUTTON_REPEAT_INTERVAL) {
				if (millis() > nextButtonRepeat) {
					setpoint = max(setpoint - 1, 0);
					nextButtonRepeat = millis() + BUTTON_REPEAT_INTERVAL;
				}
			}
			break;
		case STATE_HEAT:
			updatePID();
			updateHeater();
			if (buttonRed.fallingEdge()) {
				heater(false);
				state = STATE_IDLE;
			}
			if (buttonGreen.fallingEdge()) {
				heater(false);
				state = STATE_SET;
			}
			break;
		default:
			break;
	}
}

/**
 * State machine mode: REFLOW
 * - Perform a complete reflow process
 * - Switch to next mode on mode button press
 */
void modeReflow()
{
	switch (state) {
		case STATE_IDLE:
			if (buttonRed.fallingEdge()) {
				mode = MODE_COOL;
			}
			if (buttonGreen.fallingEdge()) {
				setpoint = REFLOW_TEMP_START;
				state = STATE_PRECOOL;
			}
			break;
		case STATE_PRECOOL:
			if (buttonRed.fallingEdge()) {
				heater(false);
				fan(false);
				state = STATE_IDLE;
			}
			fan(temp > REFLOW_TEMP_START);
			if (temp < REFLOW_TEMP_START) {
				setpoint = REFLOW_TEMP_SOAK;
				pid.SetTunings(PID_KP_PREHEAT, PID_KI_PREHEAT, PID_KD_PREHEAT);
				elapsedSeconds = 0;
				windowStartTime = millis();
				state = STATE_PREHEAT;
			}
			break;
		case STATE_PREHEAT:
			if (buttonRed.fallingEdge()) {
				heater(false);
				state = STATE_IDLE;
			}
			updatePID();
			updateHeater();
			if (temp > REFLOW_TEMP_SOAK) {
				pid.SetTunings(PID_KP_SOAK, PID_KI_SOAK, PID_KD_SOAK);
				setpoint = REFLOW_TEMP_SOAK + SOAK_TEMP_STEP;
				elapsedSeconds = 0;
				nextSoakUpdate = millis() + SOAK_INTERVAL;
				state = STATE_SOAK;
			}
			break;
		case STATE_SOAK:
			if (buttonRed.fallingEdge()) {
				heater(false);
				state = STATE_IDLE;
			}
			updatePID();
			updateHeater();
			if (temp > REFLOW_TEMP_REFLOW) {
				pid.SetTunings(PID_KP_REFLOW, PID_KI_REFLOW, PID_KD_REFLOW);
				setpoint = REFLOW_TEMP_MAX;
				elapsedSeconds = 0;
				state = STATE_REFLOW;
			} else if (millis() > nextSoakUpdate) {
				if (setpoint < REFLOW_TEMP_REFLOW) {
					setpoint += SOAK_TEMP_STEP;
				}
				nextSoakUpdate = millis() + SOAK_INTERVAL;
			}
			break;
		case STATE_REFLOW:
			if (buttonRed.fallingEdge()) {
				heater(false);
				state = STATE_IDLE;
			}
			updatePID();
			updateHeater();
			if (temp > REFLOW_TEMP_MAX - REFLOW_TEMP_MAX_OFFSET) {
				pid.SetTunings(PID_KP_REFLOW, PID_KI_REFLOW, PID_KD_REFLOW);
				setpoint = REFLOW_TEMP_COOL;
				elapsedSeconds = 0;
				state = STATE_REFLOW_COOL;
			}
			break;
		case STATE_REFLOW_COOL:
			updatePID();
			updateHeater();
			if (buttonRed.fallingEdge()) {
				heater(false);
				state = STATE_IDLE;
			}
			if (temp < REFLOW_TEMP_COOL) {
				heater(false);
				state = STATE_COOL;
			}
			break;
		case STATE_COOL:
			fan(true);
			if (buttonRed.fallingEdge()) {
				fan(false);
				state = STATE_IDLE;
			}
			if (temp < REFLOW_TEMP_END) {
				fan(false);
				state = STATE_IDLE;
			}
			break;
		default:
			break;
	}
}

/**
 * The infinite controller loop
 */
void loop()
{
	buttonRed.update();
	buttonGreen.update();
	buttonYellowLeft.update();
	buttonYellowRight.update();

	if (millis() > nextThermoRead) {
		do {
			temp = thermo.readCelsius();
		} while (temp == NAN);
		temp = temp * TEMP_CORRECTION_FACTOR;
		nextThermoRead = millis() + THERMO_READ_INTERVAL;
	}

	if (millis() > nextElapsedSecondsUpdate) {
		++elapsedSeconds;
		nextElapsedSecondsUpdate = millis() + 1000;
	}

	updateDisplay();

	if (tempError) {
		heater(false);
		state = STATE_ERROR;
		return;
	} else if (state == STATE_ERROR) {
		state = STATE_IDLE;
		return;
	}

	switch (mode) {
		case MODE_OFF:
			modeOff();
			break;

		case MODE_REFLOW:
			modeReflow();
			break;

		case MODE_MANUAL:
			modeManual();
			break;

		case MODE_COOL:
			modeCool();
			break;
	}
}
