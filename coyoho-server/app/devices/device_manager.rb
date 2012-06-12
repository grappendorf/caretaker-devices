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
require 'devices/device_connection_state'
require 'devices/camera/camera_device'
require 'devices/dimmer/dimmer_device'
require 'devices/dimmer/dimmer_rgb_device'
require 'devices/camera/ip_camera_device'
require 'devices/remote_control/remote_control_device'
require 'devices/robot/robot_device'
require 'devices/switch/switch_device'
require 'devices/xbee_device'
require 'devices/xbee_master'

class DeviceManager

	register_as :device_manager

	inject :xbee

	attr_reader :devices

	def initialize 
		@logger = Logging.logger[DeviceManager]
		@devices = []
		@devices_by_address = {}
	end

	def start
		@logger.info 'Device Manager starting'
		xbee.when_message_received {|m| xbee_message_received m}
		xbee.start
		# TODO: DataMapper doesn't load the attributes of the Device descendants
		# when using all() and it even doesn't load them lazy (they are nil not <not loaded>).
		# Only get() loads the attributes correctly, so as a workaround we first load the
		# device ids and then load the devices with get().
		device_ids = Device.all fields:[:id]
		device_ids.each do |id|
			device = Device.get id.id
			@devices << device
			begin
				@devices_by_address[device.address] = device
			rescue
			end
			device.start_device_connection_state
		end
	end

	def xbee_message_received message
		address = "%02x"*8 % message.remote_address64.address
		device = @devices_by_address[address]
		if not device
			return
		end
		message_code = message.data[0]
		if message_code == (CoYoHoMessages::COYOHO_MESSAGE_RESPONSE | CoYoHoMessages::COYOHO_ADD_LISTENER)
			device.connection_state_fire :connect_response
			return
		end
		message_type = message.data[0] & CoYoHoMessages::COYOHO_MESSAGE_TYPE_MASK
		if message_type == CoYoHoMessages::COYOHO_MESSAGE_RESPONSE or
			message_type == CoYoHoMessages::COYOHO_MESSAGE_NOTIFY
			message.data[0] = message.data[0] & CoYoHoMessages::COYOHO_MESSAGE_COMMAND_MASK
			device.message_received message.data
		end
	end
	
	def device_by_address address
		@devices_by_address[address]
	end

end