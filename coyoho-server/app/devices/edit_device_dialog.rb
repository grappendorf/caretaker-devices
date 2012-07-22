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

class EditDeviceDialog < Rubydin::Window
	
	def initialize device
		super 'Edit device "' + device.name + '"' 
		self.width = '50%'
		self.height = '50%'
		self.center
		content = Rubydin::VerticalLayout.new
		content.full_size
		content.spacing = true
		self.content = content
		form = Rubydin::Form.new
		form.form_field_factory = Rubydin::DataMapperFormFieldFactory.new
		fields = device.model.properties.map(&:name)
		fields = fields - [:id, :type, :created_at, :updated_at]
		item = Rubydin::DataMapperItem.new device
		form.item_data_source = item, fields
		content.add form
		content.add spacer = Rubydin::VerticalSpacer.new
		content.expand spacer, 1.0
		content.add panel = Rubydin::HorizontalLayout.new
		panel.width = '100%'
		panel.spacing = true
		panel.margin = true
		panel.add spacer = Rubydin::HorizontalSpacer.new
		panel.expand spacer, 1.0
		save_button = Rubydin::Button.new 'Save', Rubydin::ThemeResource.new('icons/16/ok.png')
		save_button.when_clicked do |e|
			form.commit
			@save_handler.call e if @save_handler
		end 
		panel.add save_button		
		cancel_button = Rubydin::Button.new 'Cancel', Rubydin::ThemeResource.new('icons/16/cancel.png')
		cancel_button.when_clicked {|e| @cancel_handler.call e if @cancel_handler} 
		panel.add cancel_button 
	end
	
	def when_cancelled &handler
		@cancel_handler = handler
	end

	def when_saved &handler
		@save_handler = handler
	end
	
end