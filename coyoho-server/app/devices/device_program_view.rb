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

require 'devices/device_program'

class DeviceProgramView < View

	include Securable

	register_as :device_program_view, scope: :session

	TABLE_COLUMNS = [:id, :name, :description, :created_at, :updated_at]
	FORM_FIELDS = [:name, :description, :program]

	def initialize
		super 'Device Programs', 'Device Programs', 'icons/48/execute.png', 3
	end

	def create_content

		@items = Rubydin::ActiveRecordContainer.new DeviceProgram

		gui = Rubydin::Builder.new
		content = gui.VerticalLayout do
			@table = gui.Table 'Remote Control Programs', @items do |t|
				t.selectable = true
				t.size_full
				t.visible_columns TABLE_COLUMNS
				t.column_expand_ratio :description, 1.0
				t.column_header :name, 'Name'
				t.column_header :description, 'Description'
				t.column_header :created_at, 'Created at'
				t.column_header :updated_at, 'Updated at'
				t.column_header :actions, ''
				t.generated_column :actions, do |source, item_id, column_id|
					create_action_buttons item_id
				end
				t.when_item_clicked {|e| edit e.item_id}
			end
			@form = gui.Form
			@form.form_field_factory = FormFieldFactory.new
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
		item = Rubydin::ActiveRecordItem.new DeviceProgram.new
		@form.item_data_source = item, FORM_FIELDS
		@table.value = nil
		@form.focus
	end

	def edit item_id
		@form.item_data_source = Rubydin::ActiveRecordItem.new(@items.item(item_id).item), FORM_FIELDS
		@form.focus
	end

	def save
		@form.commit
		data = @form.item_data_source.data
		if data.save
			show_notification 'Device program saved'
		else
			show_notification 'Device program validation failed'
		end
		# @table.value = data.id
		@table.container_data_source = @items
		@table.visible_columns TABLE_COLUMNS
		@form.focus
	end

	def discard
		@form.discard
		@form.focus
	end

	def delete id = nil
		if id
			table.value = id
			edit
		end
		data = @form.item_data_source.data
		if data.id
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

	class FormFieldFactory < Rubydin::ActiveRecordFormFieldFactory

		def create_field item, property_id, ui_context
			if property_id == :program
				Rubydin::CodeMirror.new create_caption(property_id),
					Rubydin::CodeMirror::MODE_RUBY, Rubydin::CodeMirror::THEME_COBALT
			else
				super
			end
		end
	end
end