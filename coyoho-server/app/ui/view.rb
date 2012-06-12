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

class View
		
	attr_reader :name
	attr_reader :title
	attr_reader :image
	attr_reader :position

	def initialize(name, title, image, position)
		@name = name
		@title = title
		@image = image
		@position = position
	end

	def create_content
		Rubydin::Label.new "Create content for the '#{name}' view in the create_content method"
	end

	def content
		@content ||= create_content
	end

	def show_notification caption, type = Rubydin::Notification::TYPE_HUMANIZED_MESSAGE
		content.application.main_window.show_notification caption, type
	end

	def window
		content.application.main_window
	end

	def push
		application.push		
	end
	
end