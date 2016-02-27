/**
 * This file is part of the Caretaker Home Automation System
 *
 * Copyright 2011-2015 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the the MIT License
 * You find a copy of license in the root directory of this project
 */

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
 * If you don't want to compile in support for Caretaker, comment out the following line
 */
#define CARETAKER

#include <Arduino.h>
#include <LiquidCrystal440.h>
#include <Bounce.h>
#include <PID.h>
#include <Adafruit_MAX31855.h>
#ifdef CARETAKER
#include <CaretakerDevice.h>
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
const uint8_t HEATER = 9;
const uint8_t THERMO_DO = 10;
const uint8_t THERMO_CLK = 15;
const uint8_t THERMO_CS = 14;
const uint8_t FAN = 8;

// LCD

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D1);

uint8_t emptySymbol[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t degreeSymbol[] = { 140, 146, 146, 140, 128, 128, 128, 128 };
uint8_t heaterSymbol[] = { 137, 146, 146, 145, 137, 137, 146, 128 };
uint8_t fanSymbol[] = { 132, 149, 142, 159, 142, 149, 132, 128 };

enum Symbols {
  SYM_DUMMY, SYM_DEGREE, SYM_HEATER, SYM_FAN, SYM_ARROW = 0x7E
};

char lcdbuf[17];

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

State state = STATE_IDLE;

const char* stateNames[] = { "Idle", "Error", "Set", "Heat", "Pre-cool", "Pre-heat", "Soak", "Reflow", "Cool",
    "Complete" };

enum Mode {
  MODE_OFF, MODE_REFLOW, MODE_MANUAL, MODE_COOL
};

Mode mode = MODE_OFF;

const char* modeNames[] = { "Off", "Reflow", "Manual", "Cool" };

// Time measurement

unsigned int elapsedSeconds = 0;
unsigned long nextElapsedSecondsUpdate = 0;

// Other state variables

boolean fanOn = false;
boolean heaterOn = false;

// Caretaker

#ifdef CARETAKER
DeviceDescriptor device;
#define SEND_TEMPERATURE_INTERVAL 1000
unsigned long nextSendTemperatureMillis = 0;
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

#ifdef CARETAKER
void register_message_handlers();
void sendTemperatureToServer();
void sendStatusToServer();
void onCommand();
void onRead();
#endif

/**
 * Switch the heater on or off.
 * @param on if true, the heater is switched on
 */
void heater(boolean on) {
  if (heaterOn != on) {
    heaterOn = on;
    digitalWrite(HEATER, heaterOn ? HIGH : LOW);
#ifdef CARETAKER
    sendStatusToServer();
#endif
  }
}

/**
 * Switch the fan on or off.
 * @param on if true, the fan is switched on
 */
void fan(boolean on) {
  if (fanOn != on) {
    fanOn = on;
    digitalWrite(FAN, fanOn ? HIGH : LOW);
#ifdef CARETAKER
    sendStatusToServer();
#endif
  }
}

/**
 * Update the state of the PID controller.
 */
void updatePID() {
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
void updateHeater() {
  heater(output > (millis() - windowStartTime));
}

/**
 * Update the LCD display with current information about the state, the tempeature,
 * and so on.
 */
void updateDisplay() {
  char *bufpos = lcdbuf;

  // Print the temperature
  if (!tempError) {
    bufpos += sprintf(bufpos, "%d%c", (int) temp, SYM_DEGREE);
  } else {
    bufpos += sprintf(bufpos, "N.C.");
  }

  // Print the heater and fan symbols
  if (digitalRead(HEATER) == HIGH) {
    bufpos += sprintf(bufpos, "%c", SYM_HEATER);
  } else if (digitalRead(FAN) == HIGH) {
    bufpos += sprintf(bufpos, "%c", SYM_FAN);
  } else {
    bufpos += sprintf(bufpos, " ");
  }

  // Print the temperature set point
  if (state == STATE_SET || state == STATE_HEAT || state == STATE_PRECOOL || state == STATE_PREHEAT
      || state == STATE_SOAK || state == STATE_REFLOW) {
    bufpos += sprintf(bufpos, " %c%d%c", SYM_ARROW, (int) setpoint, SYM_DEGREE);
  }

  // Print the elapsed seconds
  if (state == STATE_PREHEAT || state == STATE_SOAK || state == STATE_REFLOW || state == STATE_REFLOW_COOL) {
    bufpos += sprintf(bufpos, " %ds", elapsedSeconds);
  }

  lcd.setCursor(0, 0);
  memset(bufpos, ' ', 16 - (bufpos - lcdbuf));
  lcd.print(lcdbuf);
  bufpos = lcdbuf;

  // Print the current mode and state
  bufpos += sprintf(bufpos, "%s", modeNames[mode]);
  bufpos += sprintf(bufpos, "(%s)", stateNames[state]);

  lcd.setCursor(0, 1);
  memset(bufpos, ' ', 16 - (bufpos - lcdbuf));
  lcd.print(lcdbuf);
}

/**
 * Switch the fan on and cool until the OFF_COOLING_TEMP is reached.
 */
void offCooling() {
  heater(false);
  if (temp > COOLING_TEMP + 2 && !cooling) {
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
void setup() {
#ifdef CARETAKER
  device.type = "ReflowOven";
  device.description = "Reflow Oven";
  device.ledPin = 0;
  device.buttonPin = BUTTON_1;
  device.registerMessageHandlers = register_message_handlers;
  deviceInit(device);
#endif

  pinMode(BUTTON_1, INPUT);
  pinMode(BUTTON_2, INPUT);
  pinMode(BUTTON_3, INPUT);
  pinMode(BUTTON_4, INPUT);
  pinMode(HEATER, OUTPUT);
  pinMode(FAN, OUTPUT);

  digitalWrite(BUTTON_1, HIGH);
  digitalWrite(BUTTON_2, HIGH);
  digitalWrite(BUTTON_3, HIGH);
  digitalWrite(BUTTON_4, HIGH);

  lcd.begin(16, 2);
  lcd.createChar(SYM_DUMMY, emptySymbol);
  lcd.createChar(SYM_DEGREE, degreeSymbol);
  lcd.createChar(SYM_HEATER, heaterSymbol);
  lcd.createChar(SYM_FAN, fanSymbol);
  lcd.clear();

#ifdef CARETAKER
  lcd.setCursor(0, 0);
  lcd.println(F("Connecting to"));
  lcd.print(F("Caretaker..."));
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
void enterMode(Mode newMode, State newState) {
  if (mode != newMode || state != newState) {
    mode = newMode;
    state = newState;
#ifdef CARETAKER
    sendStatusToServer();
#endif
  }
}

/**
 * Enter a new state.
 * @param newState The new state
 */
void enterState(State newState) {
  if (state != newState) {
    state = newState;
#ifdef CARETAKER
    sendStatusToServer();
#endif
  }
}

/**
 * State machine mode: OFF
 * - Safe mode, do nothing
 * - Switch to next mode on mode button press
 */
void modeOff() {
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
void modeCool() {
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
void modeManual() {
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
void modeReflow() {
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
void loop() {
#ifdef CARETAKER
  deviceUpdate();
#endif

#ifdef CARETAKER
  if (deviceIsOperational()) {
    if (millis() > nextSendTemperatureMillis) {
      sendTemperatureToServer();
      nextSendTemperatureMillis = millis() + SEND_TEMPERATURE_INTERVAL;
    }
#endif

    updateDisplay();

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

    if (tempError) {
      enterMode(MODE_OFF, STATE_IDLE);
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

#ifdef CARETAKER
  }
#endif
}

#ifdef CARETAKER

/**
 * Register device specific message handlers.
 */
void register_message_handlers() {
  device.messenger->attach(MSG_REFLOW_OVEN_CMD, onCommand);
  device.messenger->attach(MSG_REFLOW_OVEN_READ, onRead);
}

/**
 * Notify the Caretaker server about the current temperature.
 */
void sendTemperatureToServer() {
  device.messenger->sendCmdStart(MSG_SENSOR_STATE);
  device.messenger->sendCmdArg(SENSOR_TEMPERATURE);
  device.messenger->sendCmdArg(temp);
  device.messenger->sendCmdEnd();
  deviceWiflyFlush();
}

/**
 * Notify the Caretaker server about a mode or state change.
 */
void sendStatusToServer() {
  device.messenger->sendCmdStart(MSG_REFLOW_OVEN_STATE);
  device.messenger->sendCmdArg(mode);
  device.messenger->sendCmdArg(state);
  device.messenger->sendCmdArg(heaterOn);
  device.messenger->sendCmdArg(fanOn);
  device.messenger->sendCmdEnd();
  deviceWiflyFlush();
}

/**
 * Called when a MSG_REFLOW_OVEN_CMD was received.
 */
void onCommand() {
  int command = device.messenger->readIntArg();
  switch (command) {
    case REFLOW_OVEN_CMD_OFF:
      enterMode(MODE_OFF, STATE_IDLE);
      break;

    case REFLOW_OVEN_CMD_START:
      enterMode(MODE_REFLOW, STATE_PRECOOL);
      break;

    case REFLOW_OVEN_CMD_COOL:
      enterMode(MODE_COOL, STATE_IDLE);
      break;
  }
}

/**
 * Called when a MSG_REFLOW_OVEN_READ was received.
 */
void onRead() {
  sendTemperatureToServer();
  sendStatusToServer();
}

#endif
