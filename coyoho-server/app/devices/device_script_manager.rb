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

require 'devices/device_script'

# Device script are stored in the database and should define two functions: start() and and 
# stop(). When the system starts up, all enabled scripts are loaded, evaluated and their start()
# function is called. If a script is modified, its stop() function is called, it is 
# re-evaluated and then its start() method is called again.
 
class DeviceScriptManager
	
	register_as :device_script_manager

	def initialize 
		@logger = Logging.logger[DeviceScriptManager]
	end
	
	def start
		@logger.info 'Device Script Manager starting'
		DeviceScript.all.each do |script|
			if script.enabled?
				@logger.info "Starting script #{script.name}"
				script_class_name = "DeviceScript#{script.name.capitalize}"
				code = <<-CODE
					class #{script_class_name}
						#{script.script}
					end
					script = #{script_class_name}.new
					script.init
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