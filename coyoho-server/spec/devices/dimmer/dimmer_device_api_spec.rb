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

describe API::DimmerDeviceAPI do
	
	include Rack::Test::Methods
	
	def app
		API::DimmerDeviceAPI
	end

	inject :device_manager

	before :each do
		@dimmer_device = DimmerDevice.new
	end		
	
	describe 'POST /device/dimmer/:address/set/:value' do
		it 'sets the dimmer device with the :address to the new :value' do
			device_manager.should_receive(:device_by_address).with('01234567').and_return @dimmer_device
			expect do
				post '/device/dimmer/01234567/set/100'
			end.to change{@dimmer_device.value}.from(0).to(100)
			last_response.status.should eq 201
			JSON.parse(last_response.body).should eq [] 
		end
	end

	describe 'GET /device/dimmer/:address/get' do
		it 'gets the value of the dimmer device with the :address' do
			@dimmer_device.value = 100
			device_manager.should_receive(:device_by_address).with('01234567').and_return @dimmer_device
			get '/device/dimmer/01234567/get'
			last_response.status.should eq 200
			JSON.parse(last_response.body).should eq({'value' => 100})
		end
	end

end