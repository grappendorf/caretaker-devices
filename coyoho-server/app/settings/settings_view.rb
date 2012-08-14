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

class SettingsView < View

	include Securable

	register_as :settings_view, scope: :session

	inject :settings

	def initialize
		super T('view.settings.name'), T('view.settings.title_user'), 'icons/48/settings.png', 6
	end

	def create_content
		if application.user.roles.include? :admin
			self.title = T('view.settings.title_admin')
		end
		item = Rubydin::DataItem.new settings
		gui = Rubydin::Builder.new
		gui.VerticalLayout do |v|
			v.spacing = true
			gui.Form 'User accounts' do |f|
				f.item_data_source = item, [:admin_password, :user_password]
				@account_form = f
			end
			gui.Form 'Serial device parameters' do |f|
				f.item_data_source = item, [:serial_device, :baud_rate, :timeout]
				@serial_form = f
			end
			gui.HorizontalLayout do |h|
				h.margin = true, false, false, false
				h.spacing = true
				gui.Button 'Save' do |b|
					b.icon = Rubydin::ThemeResource.new 'icons/16/ok.png'
					b.when_clicked {save_settings}
				end
				gui.Button 'Discard' do |b|
					b.icon = Rubydin::ThemeResource.new 'icons/16/cancel.png'
					b.when_clicked {discard_settings}
				end
			end
		end
	end

	def save_settings
		begin
			@account_form.commit
			@serial_form.commit
			settings.save
			show_notification 'Settings saved'
		rescue InvalidValueError
			show_notification 'Your input contains errors',
				Rubydin::Notification::TYPE_WARNING_MESSAGE
		end
	end

	def discard_settings
		@account_form.discard
		@serial_form.discard
		show_notification 'Settings reverted'
	end
end