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
require 'devices/switch/switch_device'

module API

	class SwitchDeviceAPI < Grape::API

		version '1', using: :header
		default_format :json

		helpers do
			
			inject :device_manager
			
			def device address
				device_manager.device_by_address address	
			end
			
		end

		resource :device do
		
			resource :switch do

				desc 'Toggle a switch',
				params:{
					address:{desc:'The address of the switch device to toggle', type:'string'},
					num:{desc:'The number of the switch to toggle', type:'integer'}
				}
				post ':address/toggle/:num' do
					device(params[:address]).toggle params[:num].to_i
					[]
				end
			
				desc 'Set the state of a switch',
				params:{
					address:{desc:'The address of the switch device to set', type:'string'},
					num:{desc:'The number of the switch to set', type:'integer'},
					state:{desc:'The new switch state', type:'boolean'}
				}
				post ':address/set/:num/:state' do
					device(params[:address]).switch params[:num].to_i, params[:state].to_b
					[]
				end

				desc 'Get the state of a switch',
				params:{
					address:{desc:'The address of the switch device to get', type:'string'},
					num:{desc:'The number of the switch to get', type:'integer'},
				}
				get ':address/get/:num' do
					{'state' => device(params[:address]).on?(params[:num].to_i)}
				end

			end
			
		end

	end

end