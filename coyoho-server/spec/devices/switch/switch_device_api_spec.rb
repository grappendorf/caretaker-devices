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

require 'api'
require 'rack/test'

describe API::SwitchDeviceAPI do
	
	include Rack::Test::Methods
	
	def app
		API::SwitchDeviceAPI
	end

	inject :device_manager
		
	before :each do
		@switch_device = SwitchDevice.new
		@switch_device.num_switches = 2		
	end
	
	describe 'POST /device/switch/:address/toggle/:num' do
		it 'toggles the state of the switch :num of the device with the :address' do
			device_manager.should_receive(:device_by_address).with('01234567').twice.and_return @switch_device
			expect do
				post '/device/switch/01234567/toggle/1'
			end.to change{@switch_device.on? 1}.from(false).to(true)
			last_response.status.should eq 201
			JSON.parse(last_response.body).should eq [] 
			expect do
				post '/device/switch/01234567/toggle/1'
			end.to change{@switch_device.on? 1}.from(true).to(false)
			last_response.status.should eq 201
			JSON.parse(last_response.body).should eq [] 
		end
	end
	
	describe 'POST /device/switch/:address/set/:num/:state' do
		it 'sets the switch :num of the device with the :address to the new :state' do
			device_manager.should_receive(:device_by_address).with('01234567').and_return @switch_device
			expect do
				post '/device/switch/01234567/set/1/on'
			end.to change{@switch_device.on? 1}.from(false).to(true)
			last_response.status.should eq 201
			JSON.parse(last_response.body).should eq [] 
		end
	end

	describe 'GET /device/switch/:address/get/:num' do
		it 'gets the state of switch :num of the device with the :address' do
			@switch_device.switch 1, SwitchDevice::ON
			device_manager.should_receive(:device_by_address).with('01234567').twice.and_return @switch_device
			get '/device/switch/01234567/get/0'
			last_response.status.should eq 200
			JSON.parse(last_response.body).should eq({'state' => false})
			get '/device/switch/01234567/get/1'
			last_response.status.should eq 200
			JSON.parse(last_response.body).should eq({'state' => true})
		end
	end

end