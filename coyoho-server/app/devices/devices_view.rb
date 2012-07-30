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
require 'devices/easyvr/easyvr_controller'
require 'devices/robot/robot_controller'
require 'devices/switch/switch_controller'
require 'devices/new_device_wizard'
require 'devices/edit_device_dialog'

class DeviceView < View

	include Securable

	register_as :devices_view, scope: :session

	inject :device_manager

	CONNECTION_UNKNOWN_ICON = Rubydin::ThemeResource.new 'icons/16/connect-unknown.png'
	CONNECTION_DISCONNECTED_ICON = Rubydin::ThemeResource.new 'icons/16/connect-no.png'
	CONNECTION_CONNECTED_ICON = Rubydin::ThemeResource.new 'icons/16/connect-established.png'

	def initialize
		super T('view.devices.name'), T('view.devices.title'), 'icons/48/processor.png', 2
	end

	def create_content
		gui = Rubydin::Builder.new
		gui.VerticalLayout do
			gui.HorizontalLayout do |h|
				h.margin = false, false, true, false
				gui.Button T('view.devices.new_device') do |b|
					b.icon = Rubydin::ThemeResource.new 'icons/16/new.png'
					b.when_clicked {new_device}
				end
				gui.Button T('view.devices.edit_device') do |b|
					b.icon = Rubydin::ThemeResource.new 'icons/16/configure.png'
					b.when_clicked {edit_device}
				end
				gui.Button T('view.devices.delete_device') do |b|
					b.icon = Rubydin::ThemeResource.new 'icons/16/trashcan.png'
					b.when_clicked {delete_device}
				end
			end
			gui.Table do |t|
				@device_table = t
				t.full_size
				t.container_property :check, Rubydin::CheckBox
				t.container_property :device, Rubydin::Component
				t.container_property :connection, Rubydin::Component
				t.container_property :controller, Rubydin::Component
				t.column_header :check, ''
				t.column_header :device, T('view.devices.device')
				t.column_header :connection, ''
				t.column_header :controller, T('view.devices.control_panel')
				t.column_expand_ratio :controller, 1.0
				t.when_item_clicked {|e| exclusively_select_item e.item_id}

				device_manager.devices.each do |device|
					add_device_table_item device
				end
			end
		end
	end

	def get_connection_state_icon device
		DeviceView.const_get "CONNECTION_#{device.state.name}_ICON" 
	end
	
	def add_device_table_item device
		controller = device.type.to_s.sub('Device', 'Controller').constantize.new device
		@device_table.add_item device.id, Rubydin::CheckBox.new, create_device_info(device),
			create_connection_state_info(device), controller
		device.when_connection_changed {|d| device_connection_changed d}		
	end

	def update_device_table_item device
		item = @device_table.item device.id
		controller = device.type.to_s.sub('Device', 'Controller').constantize.new device
		item.property(:device).value = create_device_info(device)
		item.property(:connection).value = create_connection_state_info device
		item.property(:controller).value = controller
		device.when_connection_changed {|d| device_connection_changed d}		
	end
	
	def delete_device_table_item device_id
		@device_table.remove_item device_id
	end
	
	def create_device_info device
		comp = Rubydin::VerticalLayout.new
		comp.margin = true, false, false, false
		comp.undefined_width
		comp.add Rubydin::HTMLLabel.new "<img src=\"\" /><b>#{device.name}</b>"
		comp.add Rubydin::Label.new device.address
		comp
	end

	def create_connection_state_info device
		comp = Rubydin::VerticalLayout.new
		comp.margin = true, false, false, false
		comp.undefined_size
		label = Rubydin::Label.new
		label.icon = get_connection_state_icon device
		label.width = '16px'
		comp.add label
		comp
	end
	
	def device_connection_changed device
		item = @device_table.item(device.id)
		item.get_item_property('connection').value = create_connection_state_info device
		push
	end

	def exclusively_select_item item_id
		@device_table.item_ids.each do |id|
			@device_table.item(id).property(:check).value.value = (id == item_id)
		end 
	end
	
	def new_device
		window = Rubydin::Window.new T('view.devices.create_new_device')
		window.width = '50%'
		window.height = '50%'
		window.center
		wizard = NewDeviceWizard.new
		wizard.when_completed do
			device_manager.create_device wizard.device
			add_device_table_item wizard.device
			application.main_window.remove_window window
		end
		wizard.when_cancelled {application.main_window.remove_window window}
		window.content = wizard
		application.main_window.add_window window
	end
	
	def delete_device
		ids = @device_table.item_ids.select{|id| @device_table.item(id).property(:check).value.value}
		if ids == []
			return
		end
		device_names = ids.map{|id| Device.first(id:id, fields:[:name]).name}.join '<br />'
		Rubydin::ConfirmDialog::show window, T('confirm_deletion'),
			T('view.devices.delete_confirm', devices:device_names), T('yes'), T('no') do |dialog|
				if dialog.confirmed?
					ids.each do |id|
						device_manager.delete_device id
						delete_device_table_item id
					end
				end
		end
	end
	
	def edit_device
		ids = @device_table.item_ids.select{|id| @device_table.item(id).property(:check).value.value}
		if ids == []
			return
		end
		device = Device.get ids[0]
		dialog = EditDeviceDialog.new device
		dialog.when_saved do
			application.main_window.remove_window dialog
			device_manager.update_device device
			update_device_table_item device
		end
		dialog.when_cancelled {application.main_window.remove_window dialog}
		application.main_window.add_window dialog
	end
	
end