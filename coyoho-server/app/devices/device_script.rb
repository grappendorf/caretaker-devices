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

require 'data_mapper'
require 'util/datamapper_helpers'

class DeviceScript
	
	include DomainObject

	property :id, Serial
	property :name, String, length:20, required:true, format: /^[A-Z]\w*$/, default:''
	property :description, String, length:255, default:''
	property :script, Text, default:''
	property :enabled, Boolean
	property :created_at, DateTime
	property :updated_at, DateTime	

end