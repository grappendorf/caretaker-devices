require 'spec_helper'
require 'settings/settings'

describe Settings do
	it 'initializes the settings service with defualt values' do		
		s = lookup :settings
		s.admin_password.should == 'admin'
		s.user_password.should == 'user'
		s.serial_device.should == '/dev/ttyUSB0'
	end
end