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

require 'devices/device'
require 'devices/xbee_device'

class SwitchDevice < Device
	
	include DeviceConnectionState
	include XbeeDevice
	
	property :num_switches, Integer
	property :switches_per_row, Integer

	validates_presence_of :num_switches
	validates_presence_of :switches_per_row

	ON = 1
	OFF = 0
	
	def self.icon
		@@icon ||= Rubydin::ThemeResource.new 'icons/32/joystick.png'
	end

	def states
		@states ||= Array.new(num_switches, OFF)
	end

	def toggle switch_num
		states[switch_num] = (states[switch_num] == ON ? OFF : ON)
		send_message COYOHO_SWITCH_WRITE, switch_num, COYOHO_WRITE_TOGGLE
	end
	
	def get_state switch_num
		states[switch_num]
	end

	def set_state switch_num, state
		states[switch_num] = state
		send_message COYOHO_SWITCH_WRITE, switch_num, COYOHO_WRITE_ABSOLUTE, state
	end
	
	def switch switch_num, on_or_off
		set_state switch_num, on_or_off ? ON : OFF
	end
	
	def on? switch_num
		get_state(switch_num) == ON
	end
	
	def off? switch_num
		get_state(switch_num) == OFF
	end

	def message_received message
		if message[0] == CoYoHoMessages::COYOHO_SWITCH_READ
			switch_num = message[1]
			value = message[2]
			states[switch_num] = value
			notify_change_listeners
		end
	end

	def update
		(0...num_switches).each {|i| send_message COYOHO_SWITCH_READ, i}
	end
	
end