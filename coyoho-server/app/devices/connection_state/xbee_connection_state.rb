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

require 'devices/connection_state/connection_state'
require 'statemachine'
require 'scheduler'

module XBeeConnectionState

	implements ConnectionState

	inject :scheduler
	inject :random

	REGISTER_FIRST_ATTEMPT_DELAY = 5
	REGISTER_FIRST_ATTEMPT_DELAY_RANDOM = 5
	REGISTER_ATTEMPT_DELAY = 10
	REGISTER_MAX_ATTEMPT_DELAY = 60 * 60
	REGISTER_TIMEOUT = 5
	REGISTER_LEASE = 5 * 60

	def initialize
		@logger = Logging.logger[XBeeConnectionState]
		@connection_listeners = []
		@connected = false
		device = self

		@machine = Statemachine.build do
			context device
			state :DISCONNECTED do
				event :connect, :WAIT_FOR_CONNECTION, :try_to_register
				event :connect_response, :DISCONNECTED, proc {
					@logger.debug "Connect response from #{device.name} while in state UNCONNECTED"}
				event :timeout, :DISCONNECTED, proc {
					@logger.debug "Timeout from #{device.name} while in state UNCONNECTED"}
				event :disconnect, :DISCONNECTED
			end
			state :WAIT_FOR_CONNECTION do
				event :connect_response, :CONNECTED, :device_connected
				event :timeout, :WAIT_FOR_CONNECTION, :retry_to_register
				event :disconnect, :DISCONNECTED
			end
			state :CONNECTED do
				event :connect_response, :CONNECTED, :registration_renewed
				event :timeout, :DISCONNECTED, :device_disconnected
				event :disconnect, :DISCONNECTED, :device_disconnect
			end
		end
	end

	def connect
		@register_attempt_delay = REGISTER_FIRST_ATTEMPT_DELAY + random.rand(REGISTER_FIRST_ATTEMPT_DELAY_RANDOM)
		@register_next_attempt_delay = REGISTER_ATTEMPT_DELAY
		@machine.connect
	end

	def disconnect
		@machine.disconnect
	end

	def state
		return @connected ? ConnectionState::State::CONNECTED : ConnectionState::State::DISCONNECTED
	end

	def connected?
		@connected
	end

	def when_connection_changed &block
		@connection_listeners << block
	end

	def xbee_connect_response
		@machine.connect_response
	end

	def xbee_timeout
		@machine.timeout
	end

	def xbee_connection_state
		@machine.state
	end

	private

	def device_disconnect
		send_message CoYoHoMessages::COYOHO_REMOVE_LISTENER
		device_disconnected
	end

	def try_to_register
		@logger.debug "Try to register with device #{name}"
		scheduler.in(@register_attempt_delay) do
			@logger.debug "Sending registration message to device #{name}"
			send_message CoYoHoMessages::COYOHO_ADD_LISTENER
			@timeout_job = scheduler.in(REGISTER_TIMEOUT) do
				@timeout_job = nil
				@logger.debug "Registration with device #{name} timed out"
				@machine.timeout
			end
		end
	end

	def retry_to_register
		@logger.debug "Retrying to register with device #{name}"
		@register_attempt_delay = @register_next_attempt_delay
		@register_next_attempt_delay = [2 * @register_next_attempt_delay, REGISTER_MAX_ATTEMPT_DELAY].min
		try_to_register
	end

	def device_connected
		@logger.debug "Device #{name} responded to registration request"
		@connected = true
		@register_attempt_delay = REGISTER_LEASE
		@register_next_attempt_delay = REGISTER_ATTEMPT_DELAY
		registration_renewed
		notify_connection_listeners
	end

	def device_disconnected
		@logger.debug "Lost registration with device #{name}"
		@connected = false
		notify_connection_listeners
	end

	def registration_renewed
		@logger.debug "Registration with device #{name} renewed"
		@timeout_job.unschedule if @timeout_job
		@timeout_job = nil
		try_to_register
	end

	def notify_connection_listeners
		@connection_listeners.each {|l| l.call self}
	end

end
