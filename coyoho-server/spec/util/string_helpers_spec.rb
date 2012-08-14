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

require 'util/string_helpers'

describe String do
	
	describe '#to_undescore' do
		it 'converts a camel case string to underscore format' do
			'AaaBbbCcc'.to_underscore.should eq 'aaa_bbb_ccc'
			'Aaa'.to_underscore.should eq 'aaa'
			'ABC'.to_underscore.should eq 'abc'
			'A'.to_underscore.should eq 'a'
		end
	end
		
	describe '#strip_heredoc' do	
		it 'strips the left whitespace in a here document' do
			heredoc = <<-END
				abc
					def
				ghi
			END
			heredoc.strip_heredoc.should == "abc\n\tdef\nghi\n"
		end
	end
	
	describe '#to_b' do
		it 'converts boolean strings to boolean values' do
			'true'.to_b.should be_true
			't'.to_b.should be_true
			'yes'.to_b.should be_true
			'y'.to_b.should be_true
			'1'.to_b.should be_true
			'on'.to_b.should be_true
			'false'.to_b.should be_false
			'f'.to_b.should be_false
			'no'.to_b.should be_false
			'n'.to_b.should be_false
			'0'.to_b.should be_false
			'off'.to_b.should be_false
		end
		
		it 'throws an error for unconvertable strings' do
			expect {'xyz'.to_b}.to raise_error ArgumentError 
		end
	end
	
	describe '#is_binary_data?' do
		it 'detects if a string contains binary data' do
			"\x00\x01\x02\x03\x04\x05\x06\x07".is_binary_data?.should be_true
		end
		
		it 'detects if a string contains only readable text' do
			'binary_data'.is_binary_data?.should be_false
		end
	end
		
end