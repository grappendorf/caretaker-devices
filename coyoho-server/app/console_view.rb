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

class ConsoleView < View

	include Securable

	register_as :console_view, scope: :session

	secure :view, [:admin]

	def initialize
		super T('view.console.name'), T('view.console.title'), 'icons/48/console.png', 6
	end

	class ConsoleHandler < Rubydin::Console::Handler

		def input_received console, input
			begin
				console.print eval(input).to_s
			rescue Exception => x
				console.print 'Error: ' + x.message
			end
			console.prompt
			true
		end

	end

	def create_content
		Rubydin::Console.new.tap do |console|
			console.width = '100%'
			console.height = '100%'
			console.ps = "#{application.user.name} >"
			console.cols = 120
			console.rows = 40
			console.max_buffer_size = 20
			console.greeting = 'CoYoHo Console'
			console.reset
			console.focus
			console.handler = ConsoleHandler.new
		end
	end

	def puts message
		console.print message
		push
	end
	
end