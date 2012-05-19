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

class CameraController < DeviceController

	inject :scheduler

	def initialize device
		@image_url = "http://#{device.user}:#{device.password}@#{device.host}:#{device.port}/img/snapshot.cgi"
		super
	end

	def create_control_component
		layout = Rubydin::HorizontalLayout.new

		image_panel = Rubydin::HorizontalLayout.new
		layout.add image_panel
		image_panel.margin = false, true, false, false
		@image = Rubydin::Embedded.new Rubydin::ExternalResource.new @image_url
		image_panel.add @image
		@image.type = Rubydin::Embedded::TYPE_IMAGE
		@image.width = '320px'
		@image.height = '200px'
		@image.style_name = 'v-embedded-image-with-border'

		button_grid = Rubydin::GridLayout.new 3, 3
		button_grid.spacing = true
		layout.add button_grid
		layout.align button_grid, Rubydin::Alignment::MIDDLE_CENTER

		button_up = Rubydin::Button.new
		button_up.icon = Rubydin::ThemeResource.new 'icons/32/1uparrow.png'
		button_grid.add_at button_up, 1, 0
		button_up.when_clicked {@device.up}

		button_down = Rubydin::Button.new
		button_down.icon = Rubydin::ThemeResource.new 'icons/32/1downarrow.png'
		button_grid.add_at button_down, 1, 2
		button_down.when_clicked {@device.down}

		button_left = Rubydin::Button.new
		button_left.icon = Rubydin::ThemeResource.new 'icons/32/1leftarrow.png'
		button_grid.add_at button_left, 0, 1
		button_left.when_clicked {@device.left}

		button_right = Rubydin::Button.new
		button_right.icon = Rubydin::ThemeResource.new 'icons/32/1rightarrow.png'
		button_grid.add_at button_right, 2, 1
		button_right.when_clicked {@device.right}

		button_center = Rubydin::Button.new
		button_center.icon = Rubydin::ThemeResource.new 'icons/32/stop.png'
		button_grid.add_at button_center, 1, 1
		button_center.when_clicked {@device.center}

		button_reload = Rubydin::Button.new
		button_reload.icon = Rubydin::ThemeResource.new 'icons/32/reload.png'
		button_grid.add_at button_reload, 2, 0
		button_reload.when_clicked do
			@image.request_repaint
			push
		end

		button00 = Rubydin::Button.new
		button00.icon = Rubydin::ThemeResource.new 'icons/32/empty.png'
		button_grid.add_at button00, 0, 0

		button02 = Rubydin::Button.new
		button02.icon = Rubydin::ThemeResource.new 'icons/32/empty.png'
		button_grid.add_at button02, 0, 2

		button22 = Rubydin::Button.new
		button22.icon = Rubydin::ThemeResource.new 'icons/32/empty.png'
		button_grid.add_at button22, 2, 2

		scheduler.every '1s' do
			@image.request_repaint
			push
		end

		layout
	end

	def create_config_component
		layout = Rubydin::GridLayout.new 2, 3
		layout.spacing = true
		refresh_interval = Rubydin::TextField.new 'Refresh interval'
		refresh_interval.value = @device.refresh_interval
		layout.add_at refresh_interval, 0, 0
		host = Rubydin::TextField.new 'Camera Host'
		host.value = @device.host
		layout.add_at host, 0, 1
		port = Rubydin::TextField.new 'Camera Port'
		port.value = @device.port
		layout.add_at port, 1, 1
		user = Rubydin::TextField.new 'Camera User'
		user.value = @device.user
		layout.add_at user, 0, 2
		password = Rubydin::TextField.new 'Camera Password'
		password.value = @device.password
		layout.add_at password, 1, 2
		layout
	end

end