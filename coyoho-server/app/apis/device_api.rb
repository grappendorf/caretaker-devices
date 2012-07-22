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

require 'grape'
require 'devices/camera/camera_device'
require 'devices/dimmer/dimmer_device'
require 'devices/dimmer/dimmer_rgb_device'
require 'devices/camera/ip_camera_device'
require 'devices/remote_control/remote_control_device'
require 'devices/robot/robot_device'
require 'devices/switch/switch_device'

module API

	class DeviceAPI < Grape::API

		version '1', using: :header

		resource :devices do

			desc 'Retrieve a list of all devices (base class properties only)'
			get do
				Device.all.map!{|d| Device.create_base_only d}
			end

			desc 'Retrieve a single device',
			params:{
				id:{desc:'The device id', type:'integer'}
			}
			get ':id' do
				Device.get(params[:id])
			end

		end

	end

end