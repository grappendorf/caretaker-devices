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

require 'renum'

module ConnectionState

	interface

	# A device is always in one of the following connection states:
	# - UNKNOWN
	#	We really don't know the current connection state
	# - DISCONNECTED
	#	We are unable to access the device
	# - CONNECTED
	#   A successfull connection is established and data can be transfered
	#	to and from the device
	enum :State, [:UNKNOWN, :DISCONNECTED, :CONNECTED]

	# Returns the current connection state of the device
	abstract :state

	# Call the specified block, when the device connection state has changed
	abstract :when_connection_changed

	# Start the device connection state management
	abstract :start_device_connection_state

	# Connect to the device
	abstract :connect

	# Disconnect from the device
	abstract :disconnect

	# Check if the device is connected (i.e. has the state :CONNECTED)
	def connected?
		state == :CONNECTED
	end

end