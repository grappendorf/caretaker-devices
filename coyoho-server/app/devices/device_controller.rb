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

class DeviceController < Rubydin::HorizontalLayout
		
	CONFIG_VISIBLE_ICON = Rubydin::ThemeResource.new 'icons/16/2downarrow.png'
	CONFIG_HIDDEN_ICON = Rubydin::ThemeResource.new 'icons/16/2rightarrow.png'

	def initialize device
		super()
		
		@device = device
		width 100, Rubydin::Units::PERCENTAGE
		
		show_config_button = Rubydin::Button.new
		add show_config_button
		show_config_button.icon = CONFIG_HIDDEN_ICON
		align show_config_button, Rubydin::Alignment::TOP_LEFT
		show_config_button.when_clicked do
			@config_component.visible = (not @config_component.visible?)
			show_config_button.icon = @config_component.visible ? CONFIG_VISIBLE_ICON : CONFIG_HIDDEN_ICON
		end		
		
		vlayout = Rubydin::VerticalLayout.new
		add vlayout
		expand_ratio vlayout, 1.0
		
		@control_component = create_control_component
		@control_component.margin = false, true, true, true
		vlayout.add @control_component		
		
		@config_component = create_config_component
		if @config_component
			@config_component.margin = true
			vlayout.add @config_component
			@config_component.visible = false
		
		end
		show_config_button.enabled = @config_component != nil
		
		device.when_connection_changed {device.update if device.connected?}
		device.update if device.connected?
	end
	
	def push
		application.push if application
	end
	
	def create_control_component
		Rubydin::HorizontalLayout.new		
	end
	
	def create_config_component		
		nil		
	end
	
end