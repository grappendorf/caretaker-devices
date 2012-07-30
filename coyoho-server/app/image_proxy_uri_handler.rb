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

require 'net/http'
require 'uri'

class ImageProxyUriHandler 
	
	include Rubydin::UriHandler
	include Rubydin::ParameterHandler
	
	include_class 'java.io.ByteArrayOutputStream'
	include_class 'java.io.ByteArrayInputStream'
	include_class 'java.awt.image.BufferedImage'
	include_class 'java.awt.Color'
	include_class 'javax.imageio.ImageIO'
	
	def handle_uri context, relative_uri
		if relative_uri == 'imageproxy'
			parameters = Thread.current[:ImageProxyUriHandler]
			url = URI.parse parameters['url'][0]
			Net::HTTP.start url.host, url.port do |http|
				req = Net::HTTP::Get.new url.path
				req.basic_auth parameters['user'][0], parameters['password'][0]
				res = http.request req
				stream = Rubydin::DownloadStream.new ByteArrayInputStream.new res.body.to_java_bytes
				stream.cache_time = 0
				stream.content_type = res['content-type']
				stream.file_name = 'xyz'
				stream
			end
		else
			nil
		end
	end 
	
	def handle_parameters parameters
		if parameters.contains_key 'handler' and parameters.get('handler')[0] == 'imageproxy'
			Thread.current[:ImageProxyUriHandler] = parameters
		end
	end
	
end