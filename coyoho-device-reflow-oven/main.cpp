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
 * Reflow profile for Sn63Pb37 or Sn62Pb36Ag02:
 *
 * Temperature [°C]
 *
 * 225-|                                               x  x <---------- Peak 210-225
 *     |                                            x        x
 *     |                                         x              x
 *     |                                      x                    x
 * 180-|-----------------------------------x                          x
 *     |                              x    |                          |   x
 *     |                         x         |                          |       x
 *     |                    x              |                          |
 * 150-|---------------x                   |                          |
 *     |             x |                   |                          |
 *     |           x   |                   |                          |
 *     |         x     |                   |                          |
 *     |       x       |                   |                          |
 *     |     x         |                   |                          |
 *     |   x           |                   |                          |
 * 30 -| x             |                   |                          |
 *     |   < 1.8°C/s   | 30-60 typ 120 max |        90 - 120 s        |
 *     |<Preheat Stage>|<--Soaking Stage-->|<------Reflow Stage------>|<--Cool-
 *  0  |_ _ _ _ _ _ _ _|_ _ _ _ _ _ _ _ _ _|_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
 *                     90                 180                        270    Time [s]
 */

/**
 * If you want to use this code inside the Arduino IDE, remove the Arduino.h
 * include and delete the path prefixes from the other includes.
 */

/**
 * If you don't want to compile in support for CoYoHo, comment out the following line
 */
#define COYOHO 1

#include <Arduino.h>
#include <LiquidCrystal/LiquidCrystal.h>
#include <Bounce/Bounce.h>
#include <PID/PID.h>
#include <Adafruit_MAX31855/Adafruit_MAX31855.h>
#ifdef COYOHO
#include <CoYoHoMessages.h>
#include <CoYoHoListenerManager.h>
#endif

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

const double PID_KP_PREHEAT = 120;
const double PID_KI_PREHEAT = 0.03;
const double PID_KD_PREHEAT = 20;

const double PID_KP_SOAK = 180;
const double PID_KI_SOAK = 0.5;
const double PID_KD_SOAK = 60;

const double PID_KP_REFLOW = 150;
const double PID_KI_REFLOW = 0.1;
const double PID_KD_REFLOW = 25;

const double REFLOW_TEMP_PREHEAT_START = 50;
const double REFLOW_TEMP_PREHEAT_END = 155;
const double REFLOW_TEMP_SOAK_START = 150;
const double REFLOW_TEMP_SOAK_END = 185;
const double REFLOW_TEMP_REFLOW_START = 180;
const double REFLOW_TEMP_REFLOW_MAX = 235;
const double REFLOW_TEMP_REFLOW_PEAK = 225;
const double REFLOW_TEMP_REFLOW_END = 195;
const double REFLOW_TEMP_END = 50;
const double SOAK_TEMP_STEP = 5;

const double COOLING_TEMP = 45.0;

const unsigned long SOAK_INTERVAL = 15000;

const double MIN_TEMP = 23.0;
const double MAX_TEMP = 350.0;

double setpoint, input, output;

PID pid(&input, &output, &setpoint, PID_KP_PREHEAT, PID_KI_PREHEAT, PID_KD_PREHEAT, DIRECT);

unsigned long nextSoakUpdate = 0;

unsigned long windowSize = 2500;

unsigned long windowStartTime;

double offCoolingTempTarget = COOLING_TEMP;

bool cooling = false;

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

// CoYoHo

#ifdef COYOHO
const long XBEE_BAUD_RATE = 57600;
XXBee<8> xbee;
ListenerManager<4> listenerManager(&xbee);
ZBRxResponse rxResponse;
unsigned long nextNotifyListeners = 0;
#endif

void heater(boolean on);
void fan(boolean on);
void updatePID();
void updateHeater();
void updateDisplay();
void offCooling();
void setup();
void enterMode(Mode newMode, State newState);
void enterState(State newState);
void modeOff();
void modeCool();
void modeManual();
void modeReflow();
void loop();

#ifdef COYOHO
void notifyTemperatureToListeners();
void notifyStatusToListeners();
void processXBeeMessages();
#endif

/**
 * Switch the heater on or off.
 * @param on if true, the heater is switched on
 */
void heater(boolean on)
{
#ifdef COYOHO
	boolean oldOn = digitalRead(RELAY) == HIGH;
#endif
	digitalWrite(RELAY, on ? HIGH : LOW);
#ifdef COYOHO
	if (on != oldOn) {
		notifyStatusToListeners();
	}
#endif
}

/**
 * Switch the fan on or off.
 * @param on if true, the fan is switched on
 */
void fan(boolean on)
{
#ifdef COYOHO
	boolean oldOn = digitalRead(FAN) == HIGH;
#endif
	digitalWrite(FAN, on ? HIGH : LOW);
#ifdef COYOHO
	if (on != oldOn) {
		notifyStatusToListeners();
	}
#endif
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
	lcd.print("           ");

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
	if (temp > COOLING_TEMP + 2 && ! cooling) {
		cooling = true;
	}
	if (cooling && temp < COOLING_TEMP - 2) {
		cooling = false;
	}
	fan(cooling);
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

#ifdef COYOHO
	Serial.begin(XBEE_BAUD_RATE);
	xbee.begin(Serial);
#endif

	pid.SetOutputLimits(0, windowSize);
	pid.SetSampleTime(PID_SAMPLE_INTERVAL);
	pid.SetMode(AUTOMATIC);

	setpoint = 23;
	windowStartTime = millis();
}

/**
 * Enter a new mode and state.
 * @param newMode The new mode
 * @param newState The new state
 */
void enterMode(Mode newMode, State newState)
{
#ifdef COYOHO
	Mode oldMode = mode;
	State oldState = state;
#endif
	mode = newMode;
	state = newState;
#ifdef COYOHO
	if (newMode != oldMode || newState != oldState) {
		notifyStatusToListeners();
	}
#endif
}

/**
 * Enter a new state.
 * @param newState The new state
 */
void enterState(State newState)
{
#ifdef COYOHO
	State oldState = state;
#endif
	state = newState;
#ifdef COYOHO
	if (newState != oldState) {
		notifyStatusToListeners();
	}
#endif
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
		enterMode(MODE_REFLOW, STATE_IDLE);
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
		enterMode(MODE_MANUAL, STATE_IDLE);
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
			fan(false);
			heater(false);
			if (buttonRed.fallingEdge()) {
				enterMode(MODE_OFF, STATE_IDLE);
			}
			if (buttonGreen.fallingEdge()) {
				enterState(STATE_SET);
			}
			break;
		case STATE_SET:
			if (buttonRed.fallingEdge()) {
				enterState(STATE_IDLE);
			}
			if (buttonGreen.fallingEdge()) {
				pid.SetTunings(PID_KP_REFLOW, PID_KI_REFLOW, PID_KD_REFLOW);
				enterState(STATE_HEAT);
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
				enterState(STATE_IDLE);
			}
			if (buttonGreen.fallingEdge()) {
				heater(false);
				enterState(STATE_SET);
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
				enterMode(MODE_COOL, STATE_IDLE);
			}
			if (buttonGreen.fallingEdge()) {
				enterState(STATE_PRECOOL);
			}
			break;
		case STATE_PRECOOL:
			setpoint = REFLOW_TEMP_PREHEAT_START;
			if (buttonRed.fallingEdge()) {
				heater(false);
				fan(false);
				enterState(STATE_IDLE);
			}
			fan(temp > REFLOW_TEMP_PREHEAT_START);
			if (temp < REFLOW_TEMP_PREHEAT_START) {
				setpoint = REFLOW_TEMP_PREHEAT_END;
				pid.SetTunings(PID_KP_PREHEAT, PID_KI_PREHEAT, PID_KD_PREHEAT);
				elapsedSeconds = 0;
				windowStartTime = millis();
				enterState(STATE_PREHEAT);
			}
			break;
		case STATE_PREHEAT:
			if (buttonRed.fallingEdge()) {
				heater(false);
				enterState(STATE_IDLE);
			}
			updatePID();
			updateHeater();
			if (temp > REFLOW_TEMP_SOAK_START) {
				pid.SetTunings(PID_KP_SOAK, PID_KI_SOAK, PID_KD_SOAK);
				setpoint = REFLOW_TEMP_SOAK_START + SOAK_TEMP_STEP;
				elapsedSeconds = 0;
				nextSoakUpdate = millis() + SOAK_INTERVAL;
				enterState(STATE_SOAK);
			}
			break;
		case STATE_SOAK:
			if (buttonRed.fallingEdge()) {
				heater(false);
				enterState(STATE_IDLE);
			}
			updatePID();
			updateHeater();
			if (temp > REFLOW_TEMP_REFLOW_START) {
				pid.SetTunings(PID_KP_REFLOW, PID_KI_REFLOW, PID_KD_REFLOW);
				setpoint = REFLOW_TEMP_REFLOW_MAX;
				elapsedSeconds = 0;
				enterState(STATE_REFLOW);
			} else if (millis() > nextSoakUpdate) {
				if (setpoint < REFLOW_TEMP_SOAK_END) {
					setpoint += SOAK_TEMP_STEP;
				}
				nextSoakUpdate = millis() + SOAK_INTERVAL;
			}
			break;
		case STATE_REFLOW:
			if (buttonRed.fallingEdge()) {
				heater(false);
				enterState(STATE_IDLE);
			}
			updatePID();
			updateHeater();
			if (temp > REFLOW_TEMP_REFLOW_PEAK) {
				pid.SetTunings(PID_KP_REFLOW, PID_KI_REFLOW, PID_KD_REFLOW);
				setpoint = REFLOW_TEMP_REFLOW_END;
				elapsedSeconds = 0;
				enterState(STATE_REFLOW_COOL);
			}
			break;
		case STATE_REFLOW_COOL:
			updatePID();
			updateHeater();
			if (buttonRed.fallingEdge()) {
				heater(false);
				enterState(STATE_IDLE);
			}
			if (temp < REFLOW_TEMP_REFLOW_END) {
				heater(false);
				enterState(STATE_COOL);
			}
			break;
		case STATE_COOL:
			fan(true);
			if (buttonRed.fallingEdge()) {
				fan(false);
				enterState(STATE_IDLE);
			}
			if (temp < REFLOW_TEMP_END) {
				fan(false);
				enterState(STATE_IDLE);
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
#ifdef COYOHO
	listenerManager.checkListenerLeases();
	processXBeeMessages();
	if (millis() > nextNotifyListeners) {
		notifyTemperatureToListeners();
		nextNotifyListeners = millis() + 1000;
	}
#endif

	buttonRed.update();
	buttonGreen.update();
	buttonYellowLeft.update();
	buttonYellowRight.update();

	if (millis() > nextThermoRead) {
		do {
			temp = thermo.readCelsius();
		} while (temp == NAN || thermo.readError());
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
		enterState(STATE_ERROR);
		return;
	} else if (state == STATE_ERROR) {
		enterState(STATE_IDLE);
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

#ifdef COYOHO

/**
 * Notify any CoYoHo listeners about the current temperature.
 */
void notifyTemperatureToListeners()
{
	uint16_t temp16 = temp;
	uint8_t tempMessage[] = { COYOHO_SENSOR_TEMPERATURE | COYOHO_MESSAGE_NOTIFY,
			0, (uint8_t) (temp16 >> 8), (uint8_t) (temp16 & 255) };
	listenerManager.notifyListeners(tempMessage, sizeof(tempMessage));
}

/**
 * Notify any CoYoHo listeners about a mode or state change.
 */
void notifyStatusToListeners()
{
	uint8_t statusMessage[] = { COYOHO_REFLOW_OVEN_STATUS | COYOHO_MESSAGE_NOTIFY,
			mode, state, (uint8_t) digitalRead(RELAY), (uint8_t) digitalRead(FAN) };
	listenerManager.notifyListeners(statusMessage, sizeof(statusMessage));
}

/**
 * Process XBee messages.
 */
void processXBeeMessages()
{
	xbee.readPacket();
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
			xbee.getResponse().getZBRxResponse(rxResponse);
			xbee.resetData(rxResponse.getData(), rxResponse.getDataLength());
			while (xbee.dataAvailable()) {
				uint8_t command = xbee.getData();

				if (listenerManager.processXBeeMessage(command, xbee, rxResponse)) {
					continue;
				}

				switch (command) {
					case COYOHO_REFLOW_OVEN_ACTION:
						if (xbee.dataAvailable()) {
							uint8_t ovenCommand = xbee.getData();
							switch (ovenCommand) {
								case COYOHO_REFLOW_OVEN_OFF:
									enterMode(MODE_OFF, STATE_IDLE);
									break;

								case COYOHO_REFLOW_OVEN_START:
									enterMode(MODE_REFLOW, STATE_PRECOOL);
									break;

								case COYOHO_REFLOW_OVEN_COOL:
									enterMode(MODE_COOL, STATE_IDLE);
									break;
							}
						}
						break;

					case COYOHO_SENSOR_READ:
						if (xbee.dataAvailable(1))
						{
							uint8_t sensorNum = xbee.getData();
							xbee.resetPayload();
							xbee.putPayload(COYOHO_SENSOR_READ | COYOHO_MESSAGE_RESPONSE);
							xbee.putPayload(sensorNum);
							xbee.putPayload(temp);
							ZBTxRequest txRequest(rxResponse.getRemoteAddress64(), xbee.payload(),
									xbee.payloadLenght());
							txRequest.setAddress16(rxResponse.getRemoteAddress16());
							xbee.send(txRequest);
						}
						break;

					case COYOHO_REFLOW_OVEN_STATUS:
						xbee.resetPayload();
						xbee.putPayload(COYOHO_REFLOW_OVEN_STATUS | COYOHO_MESSAGE_RESPONSE);
						xbee.putPayload(mode);
						xbee.putPayload(state);
						xbee.putPayload(digitalRead(RELAY));
						xbee.putPayload(digitalRead(FAN));
						ZBTxRequest txRequest(rxResponse.getRemoteAddress64(), xbee.payload(),
								xbee.payloadLenght());
						txRequest.setAddress16(rxResponse.getRemoteAddress16());
						xbee.send(txRequest);
						break;
				}
			}
		}
	}
}

#endif
