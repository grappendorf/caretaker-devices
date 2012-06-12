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

# Device programs are stored in the database and should define two functions: start() and and 
# stop(). When the system starts up, all enabled programs are loaded, evaluated and their start()
# function is called. If a program is modified, its stop() function is called, it is 
# re-evaluated and then its start() method is called again.
 
class DeviceProgramManager
	
	register_as :device_program_manager

	def initialize 
		@logger = Logging.logger[DeviceProgramManager]
	end
	
	def start
		@logger.info 'Device Program Manager starting'
		DeviceProgram.all.each do |program|
			if program.enabled?
				@logger.info "Starting program #{program.name}"
				program_class_name = "DeviceProgram#{program.name.capitalize}"
				code = <<-CODE
					class #{program_class_name}
						#{program.program}
					end
					program = #{program_class_name}.new
					program.init
				CODE
				begin
					eval code
				rescue Exception => x
					@logger.error 'Error: ' + x.message
				end		
			end
		end

	end
	
end