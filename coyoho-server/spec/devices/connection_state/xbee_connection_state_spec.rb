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

require 'devices/connection_state/xbee_connection_state'

describe XBeeConnectionState do

	class XbeeConnectionStateDevice < Device
		include XBeeConnectionState
		def name; 'under test'; end
	end

	before :each do
		@connection_state = XbeeConnectionStateDevice.new
		@scheduler = ManualScheduler.new
		@random = DeterministicRandom.new
		@connection_state.stub :scheduler => @scheduler
		@connection_state.stub :random => @random
		@connection_state.stub :send_message
	end

	it 'starts in state :DISCONNECTED' do
		@connection_state.state.should be ConnectionState::State::DISCONNECTED
	end

	it 'starts in internal state :DISCONNECTED' do
		@connection_state.xbee_connection_state.should be :DISCONNECTED
	end

	it 'stays in internal state :DISCONNECTED when receiving a connection response in :DISCONNECTED' do
		@connection_state.xbee_connect_response
		@connection_state.xbee_connection_state.should be :DISCONNECTED
	end

	it 'stays in internal state :DISCONNECTED when receiving :timeout in :DISCONNECTED' do
		@connection_state.xbee_timeout
		@connection_state.xbee_connection_state.should be :DISCONNECTED
	end

	it 'stays in internal state :DISCONNECTED when disconnecting in :DISCONNECTED' do
		@connection_state.disconnect
		@connection_state.xbee_connection_state.should be :DISCONNECTED
	end

	it 'enters internal state :WAIT_FOR_CONNECTION when connecting' do
		@connection_state.connect
		@connection_state.xbee_connection_state.should be :WAIT_FOR_CONNECTION
	end

	it 'tries to register with the device after a random delay when connecting' do
		@random.next_number XBeeConnectionState::REGISTER_FIRST_ATTEMPT_DELAY_RANDOM
		@connection_state.should_receive(:send_message).with CoYoHoMessages::COYOHO_ADD_LISTENER
		@connection_state.connect
		@scheduler.step XBeeConnectionState::REGISTER_FIRST_ATTEMPT_DELAY
		@scheduler.step XBeeConnectionState::REGISTER_FIRST_ATTEMPT_DELAY_RANDOM
	end

	it 'retries to connect when a timeout occured when connecting' do
		@random.next_number XBeeConnectionState::REGISTER_FIRST_ATTEMPT_DELAY_RANDOM
		@connection_state.should_receive(:send_message).with(CoYoHoMessages::COYOHO_ADD_LISTENER).twice
		@connection_state.connect
		@scheduler.step XBeeConnectionState::REGISTER_FIRST_ATTEMPT_DELAY
		@scheduler.step XBeeConnectionState::REGISTER_FIRST_ATTEMPT_DELAY_RANDOM
		@scheduler.step	XBeeConnectionState::REGISTER_TIMEOUT
		@scheduler.step XBeeConnectionState::REGISTER_ATTEMPT_DELAY
		@connection_state.xbee_connection_state.should be :WAIT_FOR_CONNECTION
	end

	it 'enters internal state :DISCONNECTED when disconnecting during a connect attempt' do
		@connection_state.connect
		@scheduler.step XBeeConnectionState::REGISTER_FIRST_ATTEMPT_DELAY
		@connection_state.disconnect
		@connection_state.xbee_connection_state.should be :DISCONNECTED
	end

	it 'enters internal state :CONNECTED when receiving a connection response in :WAIT_FOR_CONNECTION' do
		@connection_state.connect
		@scheduler.step XBeeConnectionState::REGISTER_FIRST_ATTEMPT_DELAY
		@connection_state.xbee_connect_response
		@connection_state.xbee_connection_state.should be :CONNECTED
	end
	
	it 'enters state :CONNECTED when connected with a device' do
		@connection_state.connect
		@scheduler.step XBeeConnectionState::REGISTER_FIRST_ATTEMPT_DELAY
		@connection_state.xbee_connect_response
		@connection_state.state.should be ConnectionState::State::CONNECTED
	end
	
	it 'notifies the listeners when it enters internal state :CONNECTED' do
		expect do |listener|
			@connection_state.when_connection_changed(&listener)
			@connection_state.connect
			@scheduler.step XBeeConnectionState::REGISTER_FIRST_ATTEMPT_DELAY
			@connection_state.xbee_connect_response
		end.to yield_with_args @connection_state 
	end

	it 'will not timeout when it enters internal state :CONNECTED' do
		@connection_state.connect
		@scheduler.step XBeeConnectionState::REGISTER_FIRST_ATTEMPT_DELAY
		@connection_state.xbee_connect_response
		@scheduler.step	XBeeConnectionState::REGISTER_TIMEOUT
		@connection_state.xbee_connection_state.should be :CONNECTED
	end

	it 'renews the registration after a successful registration' do
		@connection_state.should_receive(:send_message).with(CoYoHoMessages::COYOHO_ADD_LISTENER).twice		
		@connection_state.connect
		@scheduler.step XBeeConnectionState::REGISTER_FIRST_ATTEMPT_DELAY
		@connection_state.xbee_connect_response
		@scheduler.step XBeeConnectionState::REGISTER_LEASE
	end

	it 'doubles the connection delay after each unsuccessful connect attempt' do
		pending
	end
	
	it 'enters internal state :UNCONNECTED when disconnecting in :CONNECTED' do
		pending
	end
	
	it 'unregisters from the device when disconnecting' do
		pending
	end

end
