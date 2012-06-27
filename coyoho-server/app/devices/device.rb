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

require 'devices/device_connection_state'

class Device
	
	include DataMapper::Resource

	property :id, Serial
	property :type, Discriminator
	property :name, String, length:255, required:true, default:''
	property :address, String, length:255, default:''
	property :created_at, DateTime
	property :updated_at, DateTime	

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
end
