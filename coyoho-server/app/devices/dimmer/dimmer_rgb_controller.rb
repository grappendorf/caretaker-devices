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

class DimmerRgbController < DeviceController

	def create_control_component
		
		layout = Rubydin::GridLayout.new 3, 3
		layout.width = '100%'
		layout.spacing = true
		layout.expand_column 0, 0.0
		layout.expand_column 1, 0.0
		layout.expand_column 2, 1.0
				
		color_picker = Rubydin::ColorPicker.new
		color_picker.button_style = Rubydin::ColorPicker::ButtonStyle::BUTTON_AREA
		layout.add_from_to color_picker, 0, 0, 0, 2
		layout.align color_picker, Rubydin::Alignment::MIDDLE_CENTER
		color_picker.when_color_picked do
			@device.rgb = [color_picker.color.red, color_picker.color.green, color_picker.color.blue]
		end
						
		img_red = Rubydin::Embedded.new(Rubydin::ThemeResource.new 'icons/22/ledred.png')
		layout.add_at img_red, 1, 0
		layout.align img_red, Rubydin::Alignment::BOTTOM_LEFT
		
		slider_red = Rubydin::Slider.new 0, 255
		layout.add_at slider_red, 2, 0
		slider_red.width = '100%'
		slider_red.immediate = true
		slider_red.when_value_changed {@device.red = slider_red.value.to_i}
		
		img_green = Rubydin::Embedded.new(Rubydin::ThemeResource.new 'icons/22/ledgreen.png')
		layout.add_at img_green, 1, 1
		layout.align img_green, Rubydin::Alignment::BOTTOM_LEFT
		
		slider_green = Rubydin::Slider.new 0, 255
		layout.add_at slider_green, 2, 1
		slider_green.width = '100%'
		slider_green.immediate = true
		slider_green.when_value_changed {@device.green = slider_green.value.to_i}

		img_blue = Rubydin::Embedded.new(Rubydin::ThemeResource.new 'icons/22/ledblue.png')
		layout.add_at img_blue, 1, 2
		layout.align img_blue, Rubydin::Alignment::BOTTOM_LEFT
		 
		slider_blue = Rubydin::Slider.new 0, 255
		layout.add_at slider_blue, 2, 2
		slider_blue.width = '100%'
		slider_blue.immediate = true
		slider_blue.when_value_changed {@device.blue = slider_blue.value.to_i}

		@device.when_changed do
			slider_red.value = @device.red.to_f
			slider_green.value = @device.green.to_f
			slider_blue.value = @device.blue.to_f
			color_picker.color = [@device.red, @device.green, @device.blue]
			push
		end
		
		layout
	end
	
end