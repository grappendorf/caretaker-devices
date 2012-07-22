=begin

This file is part of the CoYoHo - Control Your Home System.

Copyright (C) 2012 Dirk Grappendorf (www.grappendorf.net)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software i
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

=end

class CoYoHoApplication < Rubydin::Application

	inject :dashboard_view
	inject :console_view
	inject :help_view
	inject :settings_view
	inject :sysinfo_view
	inject :device_script_view
	inject :device_view

	attr_reader :views

	def init
		super

		when_transaction_start {|application,request| Thread.current[':rubydin_session_id'] = request.session.id}

		@push = Rubydin::ServerPush.new

		@views = [
			dashboard_view, 
			console_view, 
			help_view, 
			settings_view,
			sysinfo_view, 
			device_script_view, 
			device_view
		]

		set_theme 'coyoho'

		@login_window = LoginWindow.new
		@login_window.init
		@login_window.when_closed {|e| close}
		set_main_window @login_window

		when_user_changed do |e|
			if e.new_user
				@login_window.when_closed_do_nothing
				get_main_window.open Rubydin::ExternalResource.new url
				@main_window = MainWindow.new
				@main_window.when_closed {close}
				set_main_window @main_window
				@main_window.init
				@main_window.content.add_component @push
			else
				close
			end
		end

		register(:application, :scope => :session) {self}

		if $CONFIG.key? 'auto_login'
			@login_window.login $CONFIG['auto_login']['username'], $CONFIG['auto_login']['password']
		end

	end

	def close
		micon.custom_scopes[:session].clear_session Thread.current[':rubydin_session_id']
		super
	end

	def push
		@push.push
	end

	def foo
		puts 'Foo!'
	end

end
