=begin

This file is part of the CoYoHo Control Your Home System.

Copyright 2011-2012 Dirk Grappendorf, www.grappendorf.net

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=end

module CoYoHoMessages

	COYOHO_VERSION = 1
	
	# Message types bits 6..7

	COYOHO_MESSAGE_REQUEST = 0 << 6
	COYOHO_MESSAGE_RESPONSE = 2 << 6
	COYOHO_MESSAGE_NOTIFY = 3 << 6
	COYOHO_MESSAGE_TYPE_MASK = 3 << 6
	COYOHO_MESSAGE_COMMAND_MASK = 0x3f

	# Commands bits 0..5

	COYOHO_RESET = 0
	COYOHO_ADD_LISTENER = 1
	COYOHO_REMOVE_LISTENER = 2
	COYOHO_PROGRAM_WRITE = 3
	COYOHO_PROGRAM_READ = 4
	COYOHO_SWITCH_WRITE = 5
	COYOHO_SWITCH_READ = 6
	COYOHO_SENSOR_READ = 7
	COYOHO_SERVO_WRITE = 8
	COYOHO_SERVO_READ = 9
	COYOHO_PWM_WRITE = 10
	COYOHO_PWM_READ = 11
	COYOHO_RGB_WRITE = 12
	COYOHO_RGB_READ = 13
	COYOHO_DUMP = 14

	# Value write modes

	COYOHO_WRITE_DEFAULT = 0
	COYOHO_WRITE_ABSOLUTE = 1
	COYOHO_WRITE_INCREMENT = 2
	COYOHO_WRITE_INCREMENT_DEFAULT = 3
	COYOHO_WRITE_DECREMENT = 4
	COYOHO_WRITE_DECREMENT_DEFAULT = 5
	COYOHO_WRITE_TOGGLE = 6

	# Sensor types

	COYOHO_SENSOR_TEMPERATURE = 0
	COYOHO_SENSOR_BRIGHTNESS = 1
	COYOHO_SENSOR_SERVO = 2
	COYOHO_SENSOR_POWER_CONSUMPTION = 3
	COYOHO_SENSOR_ALL = 255

	# Servo types

	COYOHO_SERVO_AZIMUTH = 0
	COYOHO_SERVO_ALTITUDE = 1
	COYOHO_SERVO_ALL = 255
	
	# Dump values
	
	COYOHO_DUMP_VERSION	= 0
	COYOHO_DUMP_LISTENER = 1

end
