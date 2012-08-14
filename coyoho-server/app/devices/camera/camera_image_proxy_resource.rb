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

class CameraImageProxyResource < Rubydin::StreamResource
	
	include_class 'java.io.ByteArrayOutputStream'
	include_class 'java.io.ByteArrayInputStream'
	include_class 'java.awt.image.BufferedImage'
	include_class 'java.awt.Color'
	include_class 'javax.imageio.ImageIO'
	
	class Source
		
		include Rubydin::StreamSource
		
		def initialize
			super
		end
		
		def stream
			image = BufferedImage.new 200, 200, BufferedImage::TYPE_INT_RGB
			drawable = image.get_graphics
			drawable.setColor(Color.lightGray);
			drawable.fillRect(0,0,200,200);
			drawable.setColor(Color.yellow);
			drawable.fillOval(25,25,150,150);
			drawable.setColor(Color.blue);
			drawable.drawRect(0,0,199,199);
			buffer = ByteArrayOutputStream.new
			ImageIO.write image, 'png', buffer
			ByteArrayInputStream.new buffer.to_byte_array
		end
		
	end
	
	def initialize application
		super Source.new, 'abc.png', application
	end
	
end