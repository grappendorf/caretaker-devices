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

class EasyvrController < DeviceController

	ICON_ON = Rubydin::ThemeResource.new 'icons/32/ledgreen.png'
	ICON_OFF = Rubydin::ThemeResource.new 'icons/32/empty.png'

	def create_control_component
		
		columns = @device.buttons_per_row
		rows = @device.num_buttons / @device.buttons_per_row

		grid = Rubydin::GridLayout.new columns, rows
		grid.spacing = true

		@buttons = (0...rows).x(0...columns).map do |row,col|
			button_num = row * columns + col
			button = Rubydin::Button.new((button_num + 1).to_s)
			button.icon = ICON_OFF
			button.when_clicked do
				@device.toggle button_num
			end
			grid.add_at button, col, row
			button
		end
		
		@device.when_changed do 
			(0...(@device.num_buttons)).each do |i| 
				@buttons[i].icon = @device.pressed?(i) ? ICON_ON : ICON_OFF
			end
			push
		end
		
		grid		
	end

end