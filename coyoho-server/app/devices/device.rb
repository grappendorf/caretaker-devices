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

require 'data_mapper'
require 'util/datamapper_helpers'
require 'devices/connection_state/connection_state'

class Device
	
	include DomainObject

	property :id, Serial
	property :type, Discriminator
	property :name, String, length:255, required:true, default:''
	property :address, String, length:255, default:''
	property :created_at, DateTime
	property :updated_at, DateTime	

	def self.handle_connection_state_with connection_state_handler
		include connection_state_handler
	end 
	
	def self.create_base_only from_other_device
		d = Device.new
		Device.properties.each{|p| d.send(p.name.to_s + '=', from_other_device.send(p.name))}
		d
	end
	
	def self.icon
		@@icon ||= Rubydin::ThemeResource.new 'icons/32/joystick.png'
	end

	def change_listeners
		@change_listeners ||= []
	end

	def when_changed &block
		change_listeners << block
	end

	def notify_change_listeners
		change_listeners.each {|l| l.call self}
	end

	def update
	end

	def load_descendant_attributes
		attributes.each{|a| attribute_get a[0]}
		p attributes
	end
	
	# We define to_xml to always return a root tag with the name of the Device base class,
	# instead of the name of the specific device subclass.
	def to_xml
		class_name = self.class.to_s.to_underscore
		xml = super
		'<device>' + xml[(class_name.length + 2)..(-class_name.length - 4)] + '</device>'
	end
	
	def start
		connect
	end
	
	def stop
		disconnect	
	end
	
	def reset
		disconnect
		connect
	end
	
end
