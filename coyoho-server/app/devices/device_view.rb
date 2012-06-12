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

require 'devices/device_controller'
require 'devices/camera/camera_controller'
require 'devices/dimmer/dimmer_controller'
require 'devices/dimmer/dimmer_rgb_controller'
require 'devices/camera/ip_camera_controller'
require 'devices/remote_control/remote_control_controller'
require 'devices/robot/robot_controller'
require 'devices/switch/switch_controller'

class DeviceView < View

	include Securable

	register_as :device_view, scope: :session

	inject :device_manager

	DISCONNECTED_ICON = Rubydin::ThemeResource.new 'icons/16/connect-no.png'
	CONNECTED_ICON = Rubydin::ThemeResource.new 'icons/16/connect-established.png'

	def initialize
		super 'Devices', 'Devices', 'icons/48/processor.png', 2
	end

	def create_content
		gui = Rubydin::Builder.new
		gui.VerticalLayout do
			gui.HorizontalLayout do |h|
				h.margin = false, false, true, false
				gui.Button 'New Device' do |b|
					b.setIcon Rubydin::ThemeResource.new 'icons/16/new.png'
				end
			end
			gui.Table do |t|
				@device_table = t
				t.full_size
				t.container_property :check, Rubydin::CheckBox
				t.container_property :device, Rubydin::Component
				t.container_property :connection, Rubydin::Embedded
				t.container_property :control, Rubydin::Component
				t.column_header :check, ''
				t.column_header :device, ''
				t.column_header :connection, ''
				t.column_header :control, ''
				t.column_expand_ratio :control, 1.0

				device_manager.devices.each do |device|
					controller = device.type.to_s.sub('Device', 'Controller').constantize.new device
					t.add_item device.id, Rubydin::CheckBox.new, create_device_info(device),
						Rubydin::Embedded.new(device.connected? ? CONNECTED_ICON : DISCONNECTED_ICON), controller
					device.when_connection_changed {|d| device_connection_changed d}
				end
			end
		end
	end

	def create_device_info device
		comp = Rubydin::VerticalLayout.new
		comp.add Rubydin::HTMLLabel.new "<img src=\"\" /><b>#{device.name}</b>"
		comp.add Rubydin::Label.new device.address
		comp
	end

	def device_connection_changed device
		item = @device_table.item(device.predecessor.id)
		icon = Rubydin::Embedded.new(device.connected? ? CONNECTED_ICON : DISCONNECTED_ICON)
		item.get_item_property('connection').value = icon
		push
	end

end