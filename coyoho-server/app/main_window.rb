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

require 'ui/view'

class MainWindow < Rubydin::Window

	inject :dashboard_view

	attr_reader :content_panel

	def init
		set_caption 'CoYoHo - Control Your Home!'

		hbox = Rubydin::HorizontalLayout.new
		hbox.margin = true
		hbox.size_full
		set_content hbox

		navigation_bar = create_navigation_bar
		hbox.add_component navigation_bar

		content_box = Rubydin::VerticalLayout.new
		hbox.add_component content_box
		hbox.expand content_box, 1.0
		content_box.set_size_full

		@content_panel = Rubydin::Panel.new
		content_box.add @content_panel
		content_box.expand @content_panel, 1.0
		@content_panel.set_size_full

		footer = Rubydin::Label.new(((application != nil and application.user != nil) ? 'Logged in as <b>' +
		application.user.name + '</b>' : '') +
		'&nbsp;'*4 + 'CoYoHo - Control Your Home' +
		'&nbsp;'*4 + '(C)2012 Dirk Grappendorf' +
		'&nbsp;'*4 + '<a href="www.grappendorf.net">www.grappendorf.net</a>',
		Rubydin::Label::CONTENT_XHTML)
		content_box.add footer

		activate_view dashboard_view
	end

	def create_navigation_bar
		navigation_bar = Rubydin::VerticalLayout.new
		navigation_bar.set_margin false, true, false, false
		navigation_bar.set_spacing true
		navigation_bar.set_width '15em'

		views = application.views.select{|v|v.allowed? :view}.sort{|v1,v2| v1.position <=> v2.position}
		views.each do |v|
			button = Rubydin::Button.new v.name
			navigation_bar.add button
			button.set_width '100%'
			button.set_icon Rubydin::ThemeResource.new v.image
			button.when_clicked { aactivate_view v }
		end

		button_logout = Rubydin::Button.new 'Logout'
		navigation_bar.add button_logout
		button_logout.set_width '100%'
		button_logout.set_icon Rubydin::ThemeResource.new 'icons/48/exit.png'
		button_logout.when_clicked { application.set_user nil }

		if application.user.roles.include? :admin
			button_terminate = Rubydin::Button.new 'Terminate'
			navigation_bar.add button_terminate
			button_terminate.set_width '100%'
			button_terminate.set_icon Rubydin::ThemeResource.new 'icons/48/bomb.png'
			button_terminate.when_clicked { exit }
		end

		navigation_bar
	end

	def activate_view view
		@content_panel.remove_all_components
		@content_panel.add view.content
		@content_panel.caption = view.title
	end

end
