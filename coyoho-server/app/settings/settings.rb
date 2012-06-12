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

require 'yaml'

class Settings

	inject :logger

	SETTINGS_FILE_NAME = $CONFIG['settings_file']

	enum :BaudRate do
		B300(300)
		B1200(1200)
		B2400(2400)
		B4800(4800)
		B9600(9600)
		B19200(19200)
		B38400(38400)
		B57600(57600)
		B115200(152000)
		attr_reader :rate

		def init rate
			@rate = rate
		end

		def to_s
			rate.to_s
		end
	end

	attr_accessor :admin_password
	attr_accessor :user_password
	attr_accessor :serial_device
	attr_accessor :baud_rate
	attr_accessor :timeout

	def save
		File.open(SETTINGS_FILE_NAME, 'w') {|f| f.puts to_yaml}
	end

	register :settings do
		if File.exists?(SETTINGS_FILE_NAME)
			YAML::load(File.read(SETTINGS_FILE_NAME))
		else
			Settings.new.tap do |s|
				s.admin_password = 'admin'
				s.user_password = 'user'
				s.serial_device = '/dev/ttyUSB0'
				s.baud_rate = Settings::BaudRate::B57600
				s.timeout = 1000
			end
		end
	end
end
