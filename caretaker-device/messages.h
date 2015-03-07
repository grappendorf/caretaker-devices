/**
 * This file is part of the Caretaker Home Automation System
 *
 * Copyright 2011-2015 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the the MIT License
 * You find a copy of license in the root directory of this project
 */

#ifndef _MESSAGES_H
#define _MESSAGES_H

/** General messages */
#define MSG_INVALID             0
#define MSG_REGISTER_REQUEST    1
#define MSG_REGISTER_RESPONSE   2
#define MSG_PING_REQUEST        3
#define MSG_PING_RESPONSE       4
#define MSG_SWITCH_WRITE        5
#define MSG_SWITCH_READ         6
#define MSG_SWITCH_STATE        7
#define MSG_RGB_WRITE           8
#define MSG_RGB_READ            9
#define MSG_RGB_STATE           10
#define MSG_PWM_WRITE           11
#define MSG_PWM_READ            12
#define MSG_PWM_STATE           13
#define MSG_SENSOR_READ         14
#define MSG_SERVO_WRITE         15
#define MSG_SERVO_READ          16
#define MSG_REFLOW_OVEN_CMD     17
#define MSG_REFLOW_OVEN_READ    18

/** Value write modes */
#define WRITE_DEFAULT            0
#define WRITE_ABSOLUTE           1
#define WRITE_INCREMENT          2
#define WRITE_INCREMENT_DEFAULT  3
#define WRITE_DECREMENT          4
#define WRITE_DECREMENT_DEFAULT  5
#define WRITE_TOGGLE             6

/** Sensor types */
#define SENSOR_ALL                 0
#define SENSOR_TEMPERATURE         1
#define SENSOR_BRIGHTNESS          2
#define SENSOR_SERVO               3
#define SENSOR_POWER_CONSUMPTION   4

/** Servo types */
#define SERVO_ALL      0
#define SERVO_AZIMUTH  1
#define SERVO_ALTITUDE 2

/** Reflow oven commands */
#define REFLOW_OVEN_CMD_OFF    0
#define REFLOW_OVEN_CMD_START  1
#define REFLOW_OVEN_CMD_COOL   2

/** Reflow oven modes */
#define REFLOW_OVEN_MODE_OFF     0
#define REFLOW_OVEN_MODE_REFLOW  1
#define REFLOW_OVEN_MODE_MANUAL  2
#define REFLOW_OVEN_MODE_COOL    3

/** Reflow oven states */
#define REFLOW_OVEN_STATE_IDLE         0
#define REFLOW_OVEN_STATE_ERROR        1
#define REFLOW_OVEN_STATE_SET          2
#define REFLOW_OVEN_STATE_HEAT         3
#define REFLOW_OVEN_STATE_PRECOOL      4
#define REFLOW_OVEN_STATE_PREHEAT      5
#define REFLOW_OVEN_STATE_SOAK         6
#define REFLOW_OVEN_STATE_REFLOW       7
#define REFLOW_OVEN_STATE_REFLOW_COOL  8
#define REFLOW_OVEN_STATE_COOL         9
#define REFLOW_OVEN_STATE_COMPLETE     10

#endif /* _MESSAGES_H */
