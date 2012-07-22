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

module DataMapperRest
	class Adapter < DataMapper::Adapters::AbstractAdapter
		def aggregate query
			model = query.model
			response = connection.http_get("#{resource_name(model)}/count")
			doc = REXML::Document::new response.body
			[doc.elements[1].text.to_i]
		end 	
	end
end
