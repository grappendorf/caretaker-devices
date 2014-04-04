/**
 * This file is part of the CoYoHo Control Your Home System.
 *
 * Copyright 2011-2012 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COYOHO_MESSAGES_H_
#define COYOHO_MESSAGES_H_

/** COYOHO messages */
#define COYOHO_VERSION	1

/** Message types bits 6..7 */
#define COYOHO_MESSAGE_REQUEST  	0 << 6
#define COYOHO_MESSAGE_RESPONSE 	2 << 6
#define COYOHO_MESSAGE_NOTIFY   	3 << 6
#define COYOHO_MESSAGE_TYPE_MASK	3 << 6
#define COYOHO_MESSAGE_COMMAND_MASK	0x3f

/** Commands bits 0..5 */
#define COYOHO_RESET				0
#define COYOHO_ADD_LISTENER			1
#define COYOHO_REMOVE_LISTENER		2
#define COYOHO_PROGRAM_WRITE		3
#define COYOHO_PROGRAM_READ			4
#define COYOHO_SWITCH_WRITE 		5
#define COYOHO_SWITCH_READ  		6
#define COYOHO_SENSOR_READ  		7
#define COYOHO_SERVO_WRITE  		8
#define COYOHO_SERVO_READ   		9
#define COYOHO_PWM_WRITE			10
#define COYOHO_PWM_READ				11
#define COYOHO_RGB_WRITE			12
#define COYOHO_RGB_READ				13
#define COYOHO_DUMP					14
#define COYOHO_REFLOW_OVEN_ACTION	15
#define COYOHO_REFLOW_OVEN_STATUS	16

/** Value write modes */
#define COYOHO_WRITE_DEFAULT			0
#define COYOHO_WRITE_ABSOLUTE			1
#define COYOHO_WRITE_INCREMENT			2
#define COYOHO_WRITE_INCREMENT_DEFAULT	3
#define COYOHO_WRITE_DECREMENT			4
#define COYOHO_WRITE_DECREMENT_DEFAULT	5
#define COYOHO_WRITE_TOGGLE				6

/** Sensor types */
#define COYOHO_SENSOR_TEMPERATURE 		0
#define COYOHO_SENSOR_BRIGHTNESS  		1
#define COYOHO_SENSOR_SERVO       		2
#define COYOHO_SENSOR_POWER_CONSUMPTION	3
#define COYOHO_SENSOR_ALL         		255

/** Servo types */
#define COYOHO_SERVO_AZIMUTH  0
#define COYOHO_SERVO_ALTITUDE 1
#define COYOHO_SERVO_ALL      255

/** Dump values */
#define COYOHO_DUMP_VERSION		0
#define COYOHO_DUMP_LISTENER	1

/** Reflow oven actions */
#define COYOHO_REFLOW_OVEN_OFF		0
#define COYOHO_REFLOW_OVEN_START	1
#define COYOHO_REFLOW_OVEN_COOL		2

/** Reflow oven modes */
#define COYOHO_REFLOW_OVEN_MODE_OFF		0
#define COYOHO_REFLOW_OVEN_MODE_REFLOW	1
#define COYOHO_REFLOW_OVEN_MODE_MANUAL	2
#define COYOHO_REFLOW_OVEN_MODE_COOL	3

/** Reflow oven states */
#define COYOHO_REFLOW_OVEN_STATE_IDLE			0
#define COYOHO_REFLOW_OVEN_STATE_ERROR			1
#define COYOHO_REFLOW_OVEN_STATE_SET			2
#define COYOHO_REFLOW_OVEN_STATE_HEAT			3
#define COYOHO_REFLOW_OVEN_STATE_PRECOOL		4
#define COYOHO_REFLOW_OVEN_STATE_PREHEAT		5
#define COYOHO_REFLOW_OVEN_STATE_SOAK			6
#define COYOHO_REFLOW_OVEN_STATE_REFLOW			7
#define COYOHO_REFLOW_OVEN_STATE_REFLOW_COOL	8
#define COYOHO_REFLOW_OVEN_STATE_COOL			9
#define COYOHO_REFLOW_OVEN_STATE_COMPLETE		10

#endif /* COYOHO_MESSAGES_H_ */
