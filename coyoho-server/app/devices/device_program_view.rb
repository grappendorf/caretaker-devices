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

# TODO: Currently there is a bug with CodeMirror. If CodeMirror is used as a form field, the
# previous CodeMirror is not removed from the form, every second time when the item_data_source
# is changed.
# So currently the CodeMirror field is added as a seperate component and the value of the program
# field is transfered manually.

require 'devices/device_program'

class DeviceProgramView < View

	include Securable

	register_as :device_program_view, scope: :session

	TABLE_COLUMNS = [:id, :enabled, :name, :description, :created_at, :updated_at, :actions]
	FORM_FIELDS = [:enabled, :name, :description]

	def initialize
		super 'Device Programs', 'Device Programs', 'icons/48/execute.png', 3
	end 

	def create_content
 
		@items = Rubydin::DataMapperContainer.new DeviceProgram

		gui = Rubydin::Builder.new
		content = gui.VerticalLayout do
			@table = gui.Table 'Remote Control Programs', @items do |t|
				t.selectable = true
				t.immediate = true
				t.full_size
				t.generated_column :enabled, &Rubydin::Table.boolean_image_column
				t.generated_column :actions, do |source, item_id, column_id|
					create_action_buttons item_id
				end
				t.visible_columns TABLE_COLUMNS
				t.column_expand_ratio :description, 1.0
				t.column_header :enabled, 'Enabled'
				t.column_header :name, 'Name'
				t.column_header :description, 'Description'
				t.column_header :created_at, 'Created at'
				t.column_header :updated_at, 'Updated at'
				t.column_header :actions, ''
				t.when_selection_changed {|e| edit e.property.value}
			end
			@form = gui.Form
			@form.form_field_factory = Rubydin::DataMapperFormFieldFactory.new
			@program_editor = gui.CodeMirror nil,
				Rubydin::CodeMirror::MODE_RUBY, Rubydin::CodeMirror::THEME_MONOKAI
			@program_editor.width = '100%'
			gui.HorizontalLayout do |h|
				h.margin = true, false, false, false
				h.spacing = true
				gui.Button 'Save' do |b|
					b.setIcon Rubydin::ThemeResource.new 'icons/16/ok.png'
					b.when_clicked {save}
				end
				gui.Button 'Discard' do |b|
					b.setIcon Rubydin::ThemeResource.new 'icons/16/undo.png'
					b.when_clicked {discard}
				end
				gui.Button 'Create' do |b|
					b.setIcon Rubydin::ThemeResource.new 'icons/16/new.png'
					b.when_clicked {create}
				end
				gui.Button 'Delete' do |b|
					b.setIcon Rubydin::ThemeResource.new 'icons/16/trashcan.png'
					b.when_clicked {delete}
				end
			end
		end

		create

		content
	end

	def create_action_buttons item_id
		layout = Rubydin::HorizontalLayout.new
		button = Rubydin::Button.new
		button.icon = Rubydin::ThemeResource.new 'icons/16/trashcan.png'
		button.when_clicked {delete item_id}
		layout.add button
		layout
	end

	def create
		item = Rubydin::DataMapperItem.new DeviceProgram.new
		@form.item_data_source = item, FORM_FIELDS
		@program_editor.value = ''
		@table.value = nil
		@form.focus
	end

	def edit item_id
		if item_id
			item = @items.item(item_id).item
			@form.item_data_source = Rubydin::DataMapperItem.new(item), FORM_FIELDS
			@program_editor.value = item.program
			@form.focus
		else
			create
		end
	end

	def save
		@form.commit
		data = @form.item_data_source.data
		data.program = @program_editor.value
		if data.save
			show_notification 'Device program saved'
		else
			show_notification 'Device program validation failed'
		end
		@table.container_data_source = @items
		@table.visible_columns TABLE_COLUMNS
		@table.value = data.id
		@form.focus
	end

	def discard
		@form.discard
		data = @form.item_data_source.data
		@program_editor.value = data.program
		@form.focus
	end

	def delete id = nil
		id = @form.item_data_source.data.id unless id
		if id
			data = @items.item(id).item
			Rubydin::ConfirmDialog::show window, 'Confirm deletion',
			"Do you really want to<br />delete device program <b>#{data.name}</b>?",
			'Yes', 'No' do |dialog|
				if dialog.confirmed?
					data.destroy
					@table.container_data_source = @items
					@table.visible_columns TABLE_COLUMNS
					create
				end
			end
		end
	end

end