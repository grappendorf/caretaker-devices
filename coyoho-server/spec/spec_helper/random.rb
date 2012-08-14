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

# This is a 'random' number generator that returns deterministic numbers.
# Use this in your test code instead of the normal random number service.

class DeterministicRandom
	
	def initialize default = 0
		@default = default
		@numbers = []
	end
	
	def next_number number
		next_numbers number		
	end
	
	def next_numbers *numbers
		@numbers = numbers
	end
		
	def rand limit = nil
		@numbers.shift || @default
	end
	
end
