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

require 'net/http'
require 'uri'
require 'devices/device'
require 'util/executor'

class IpCameraDevice < Device

	include NullDeviceConnectionState
	include Executor

	property :host, String, length:255
	property :port, Integer
	property :user, String, length:255
	property :password, String, length:255
	property :refresh_interval, Integer

	def icon
		@icon ||= Rubydin::ThemeResource.new 'icons/32/camera.png'
	end

	def send_message *data
	end

	def message_received message
	end

	def up
		async do
			send_command '0'
			sleep 1
			send_command '1'
		end
	end

	def down
		async do
			send_command '2'
			sleep 1
			send_command '3'
		end
	end

	def left
		async do
			send_command '4'
			sleep 1
			send_command '5'
		end
	end

	def right
		async do
			send_command '6'
			sleep 1
			send_command '7'
		end
	end

	def center
		send_command '25'
	end

	def send_command command		
		uri ||= URI "http://#{host}:#{port}/decoder_control.cgi?command=#{command}"
		req = Net::HTTP::Get.new uri.request_uri
		req.basic_auth user, password
		Net::HTTP.start(uri.host, uri.port) {|http| http.request req}
	end

end