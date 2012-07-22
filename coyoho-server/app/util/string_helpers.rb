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

class String

	def to_b
		return true if self == true || self =~ (/(true|t|yes|y|1|on)$/i)
		return false if self == false || self.nil? || self =~ (/(false|f|no|n|0|off)$/i)
		raise ArgumentError.new "invalid value for Boolean: '#{self}'"
	end
	
	def is_binary_data?
		(self.count( "^ -~", "^\r\n" ).fdiv(self.size) > 0.3 || self.index( "\x00" )) unless empty?
	end

	def to_underscore!
		self.gsub!(/(.)([A-Z])/,'\1_\2').downcase!
	end
   
	def to_underscore
		self.clone.to_underscore!
	end
	
end