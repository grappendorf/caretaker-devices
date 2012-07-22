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

class Fixnum
	
	def to_xml
		"<integer>#{self.to_s}</integer>"
	end
	
end

class Float
	
	def to_xml
		"<float>#{self.to_s}</float>"
	end
	
end

class String

	def to_xml
		"<string>#{self.to_s}</string>"
	end
	
end
