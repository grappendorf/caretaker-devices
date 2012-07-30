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

require 'settings/settings'
require 'auth/user'

class LoginWindow < Rubydin::Window

	inject :settings

	def init
		self.caption = T('view.login.title')
		self.content = Rubydin::VerticalLayout.new
		self.content.full_size

		panel = Rubydin::Panel.new T('welcome_to_coyoho')
		self.content.add panel
		self.content.align panel, Rubydin::Alignment::MIDDLE_CENTER

		loginForm = Rubydin::LoginForm.new
		panel.add loginForm
		loginForm.when_logged_in {|e| login e.username, e.password}
	end

	def login(username, password)
		if username == 'admin' && password == settings.admin_password
			application.user = User.new 'admin', 'admin', [:admin, :user]
		elsif username == 'user' and password == settings.user_password
			application.user = User.new 'user', 'user', [:user]
		else
			show_notification T('wrong_username_or_password')
		end
	end
end
