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

class CameraDevice < Device
	
	include DeviceConnectionState
	include XbeeDevice

	property :host, String, length:255
	property :port, Integer
	property :user, String, length:255
	property :password, String, length:255
	property :refresh_interval, Integer

	def self.icon
		@@icon ||= Rubydin::ThemeResource.new 'icons/32/camera.png'
	end
	
	def message_received message
	end
	
	def left
		send_message COYOHO_SERVO_WRITE, COYOHO_SERVO_AZIMUTH, COYOHO_WRITE_INCREMENT_DEFAULT
	end
	
	def right			
		send_message COYOHO_SERVO_WRITE, COYOHO_SERVO_AZIMUTH, COYOHO_WRITE_DECREMENT_DEFAULT
	end
	
	def up
		send_message COYOHO_SERVO_WRITE, COYOHO_SERVO_ALTITUDE, COYOHO_WRITE_INCREMENT_DEFAULT				
	end
	
	def down
		send_message COYOHO_SERVO_WRITE, COYOHO_SERVO_ALTITUDE, COYOHO_WRITE_DECREMENT_DEFAULT		
	end
	
	def center
		send_message COYOHO_SERVO_WRITE, COYOHO_SERVO_ALL, COYOHO_WRITE_DEFAULT
	end
	
end