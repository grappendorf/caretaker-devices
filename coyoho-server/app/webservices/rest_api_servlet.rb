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

require 'devices/switch/switch_webservice'
require 'devices/dimmer/dimmer_webservice'

class RESTApiServlet < Java::javax.servlet.http.HttpServlet

	def init config
		@logger = Logging.logger[self]
		@switch_handler = SwitchWebService.new
		@dimmer_handler = DimmerWebService.new
	end

	def service request, response
		res = nil
		if params = (request.path_info.match %r{^/device/(.+)/switch/(.+)/toggle$})
			res = @switch_handler.toggle params[1], params[2]
		elsif params = (request.path_info.match %r{^/device/(.+)/switch/(.+)/(.+)$})
			res = @switch_handler.set_state params[1], params[2], params[3]
		elsif params = (request.path_info.match %r{^/device/(.+)/switch/(.+)$})
			res = @switch_handler.get_state params[1], params[2]
		elsif params = (request.path_info.match %r{^/device/(.+)/dimmer/(.+)$})
			res = @dimmer_handler.set_value params[1], params[2]
		elsif params = (request.path_info.match %r{^/device/(.+)/dimmer$})
			res = @dimmer_handler.get_value params[1]
		else
			res = 'No handler for this request path'
		end
		response.content_type = 'text/html'
		response.writer.print res.to_s if res != nil
		response.writer.close
	end

end