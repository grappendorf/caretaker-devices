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
require 'devices/connection_state/xbee_connection_state'
require 'devices/xbee_device'
require 'devices/easyvr/easyvr_listener'

class EasyvrDevice < Device

	handle_connection_state_with XBeeConnectionState
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

	def button_listeners
		@button_listeners ||= []
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
			old_value = states[button_num]
			states[button_num] = value
			notify_change_listeners
			button_listeners.each do |listener|
				if value ==  PRESSED
					listener.button_pressed button_num
				else
					listener.button_released button_num
				end
			end			
		end
	end

	def add_easyvr_listener listener
		assert listener.is_a EasyvrListener
		button_listeners.push listener
	end	

	def remove_easyvr_listener listener
		assert listener.is_a EasyvrListener
		button_listeners.delete listener
	end
	
end