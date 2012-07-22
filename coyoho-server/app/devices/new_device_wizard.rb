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

class NewDeviceWizard < Rubydin::Wizard

	attr_accessor :device_type
	attr_accessor :device

	inject :application
	inject :device_manager
		
	def initialize
		super
		step SelectDeviceTypeStep.new(self)
		@device_data_step = DeviceDataStep.new(self) 
		step @device_data_step
	end
	
	class SelectDeviceTypeStep < Rubydin::WizardStep

		def initialize wizard
			@wizard = wizard
		end
		
		def caption
			'Type'
		end
		
		def content
			@table = Rubydin::Table.new
			@table.full_size
			@table.container_property :icon, Rubydin::Embedded
			@table.container_property :name, String
			@table.container_property :description, String
			@table.column_header :icon, '&nbsp'*5
			@table.column_header :name, 'Device type'
			@table.column_header :description, 'Description'
			@table.column_expand_ratio :description, 1.0			
			@table.selectable = true
			@types = ObjectSpace.each_object(Class).select {|klass| klass < Device}
			@types.sort{|a,b| a.name <=> b.name}
			@types.each_with_index do |type, i| 
				@table.add_item i, Rubydin::Embedded.new(type.icon), type.name, ''
			end
			@table.when_selection_changed {@wizard.update}
			@table
		end
				
		def can_next
			@table.value != nil
		end
		
		def on_next
			@wizard.device_type = @types[@table.value]
			true
		end
	end

	class DeviceDataStep < Rubydin::WizardStep
		
		def initialize wizard
			@wizard = wizard
		end
		
		def caption
			'Data'
		end
		
		def content
			@form = Rubydin::Form.new
			@form.form_field_factory = Rubydin::DataMapperFormFieldFactory.new
			fields = @wizard.device_type.properties.map(&:name)
			fields = fields - [:id, :type, :created_at, :updated_at]
			@item = Rubydin::DataMapperItem.new @wizard.device_type.new
			@form.item_data_source = @item, fields
			@form
		end
		
		def on_next
			begin 
				@form.commit
				if @item.data.valid?
					@wizard.device = @item.data
					return true
				else
					raise
				end
			rescue Exception
				@wizard.application.main_window.show_notification 'New device contains errors'
			end
			false
		end
	end

end