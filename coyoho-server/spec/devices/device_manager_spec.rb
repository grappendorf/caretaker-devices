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

require 'devices/device_manager'

describe DeviceManager, use: :database do
	
	before :each do
		@device_manager = DeviceManager.new
		@device_manager.stub :xbee => double(:xbee).as_null_object
	end
	
	after :each do
		DatabaseCleaner.clean				
	end
	
	it 'loads all devices on startup' do
		Device.any_instance.stub :start
		3.times {FactoryGirl.create :device}
		@device_manager.start
		@device_manager.device_by_address('00000001').should_not be_nil
		@device_manager.device_by_address('00000002').should_not be_nil
		@device_manager.device_by_address('00000003').should_not be_nil
	end

	it 'start all devices on startup' do
		Device.any_instance.should_receive :start
		FactoryGirl.create :device
		@device_manager.start
	end
	
end
