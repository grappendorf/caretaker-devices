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

class RemoteControlDevice < Device

	include DeviceConnectionState
	include XbeeDevice

	property :num_buttons, Integer
	property :buttons_per_row, Integer
	
	validates_presence_of :num_buttons
	validates_presence_of :buttons_per_row

	PRESSED = 1
	RELEASED = 0

	def self.icon
		@@icon ||= Rubydin::ThemeResource.new 'icons/32/gamepad.png'
	end

	def states
		@states ||= Array.new(num_buttons, RELEASED)
	end

	def pressed? button_num
		states[button_num] == PRESSED
	end

	def message_received message
		if message[0] == CoYoHoMessages::COYOHO_SWITCH_READ
			button_num = message[1]
			value = message[2]
			states[button_num] = value
			notify_change_listeners
		end
	end

end