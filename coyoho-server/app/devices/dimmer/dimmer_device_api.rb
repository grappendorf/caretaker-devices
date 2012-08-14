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
require 'devices/abstract_device_api'
require 'devices/dimmer/dimmer_device'

module API

	class DimmerDeviceAPI < Grape::API

		version '1', using: :header
		default_format :json

		helpers do
			
			inject :device_manager
			
			def device address
				device_manager.device_by_address address	
			end
			
		end

		resource :device do
		
			resource :dimmer do

				desc 'Set the value of a dimmer',
				params:{
					address:{desc:'The address of the dimmer device to set', type:'string'},
					value:{desc:'The new dimmer value', type:'integer'}
				}
				post ':address/set/:value' do
					device(params[:address]).value = params[:value].to_i
					[]
				end

				desc 'Get the value of a dimmer',
				params:{
					address:{desc:'The address of the dimmer device to get', type:'string'},
				}
				get ':address/get' do
					{'value' => device(params[:address]).value}
				end

			end
			
		end

	end

end