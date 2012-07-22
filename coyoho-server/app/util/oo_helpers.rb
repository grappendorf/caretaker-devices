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

class Module

	attr_accessor :_oo_helpers_is_interface
	
	def _oo_helpers_abstract_methods
		@_oo_helpers_abstract_methods ||= []
	end

	def _oo_helpers_ensure_that_all_interface_methods_are_implemented interface_module		
		missing = interface_module._oo_helpers_abstract_methods - self.instance_methods(false)
		if missing != []
			raise "Unimplemented #{interface_module} methods in #{self}: #{missing}"
		end
	end

	def interface
		@_oo_helpers_is_interface = true
	end
	
	def abstract name
		if not _oo_helpers_is_interface
			raise "#{self} module: To use abstract methods, please tag this module with 'interface'"
		end
		_oo_helpers_abstract_methods << name
		define_method(name){raise NotImplementedError}
	end

	def implements interface_module
		if not interface_module._oo_helpers_is_interface
			raise "#{self} module: Please tag interface module #{interface_module} with 'interface'"
		end
		@_oo_helpers_interface_module = interface_module
		instance_eval do
			def included othermod
				_oo_helpers_ensure_that_all_interface_methods_are_implemented @_oo_helpers_interface_module
			end
		end
		include interface_module
	end

end