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

class DimmerController < DeviceController

	def create_control_component
		
		layout = Rubydin::VerticalLayout.new
		
		slider = Rubydin::Slider.new 0, 255
		layout.add slider
		slider.width = '100%'
		slider.immediate = true
		slider.when_value_changed {@device.value = slider.value.to_i}
		
		@device.when_changed do
			slider.value = @device.value.to_f
			push
		end
		
		layout
	end
	
end