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

require 'devices/abstract_device'
require 'devices/xbee_device'

class DimmerRgbDevice < AbstractDevice

	include DeviceConnectionState
	include XbeeDevice

	acts_as_heir_of :device

	def rgb
		@rgb ||= [0, 0, 0]
	end
	
	def red
		rgb[0]
	end
	
	def green
		rgb[1]
	end
	
	def blue
		rgb[2]
	end

	def rgb= rgb
		if @rgb != rgb
			send_message COYOHO_RGB_WRITE, 0, COYOHO_WRITE_ABSOLUTE, rgb[0], rgb[1], rgb[2]
		end
		@rgb = rgb
	end
	
	def red= red
		self.rgb = [red, rgb[1], rgb[2]]
	end
	
	def green= green
		self.rgb = [rgb[0], green, rgb[2]] 
	end

	def blue= blue
		self.rgb = [rgb[0], rgb[1], blue]
	end

	def message_received message
		if message[0] == CoYoHoMessages::COYOHO_RGB_READ
			@rgb = [message[2], message[3], message[4]] 
			notify_change_listeners
		end	
	end

end