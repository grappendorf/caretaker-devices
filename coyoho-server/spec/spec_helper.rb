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

require 'rubygems'
require 'bundler/setup'
require 'data_mapper'

def load_yaml file_name
	yaml = File.read file_name
	yaml = yaml.gsub(/(?:<%=)(.+?)(?:%>)/) {eval($1)}
	YAML::load(yaml)
end

def run_once
  path = File.expand_path(caller.first)
  unless ($__already_run_block ||= []).include?(path)
    yield
    $__already_run_block << path
  end
end

environment_name = 'test'
$CONFIG = {}
$CONFIG['environment'] = environment_name

require 'logging'
Logging.appenders.stdout.layout = Logging.layouts.pattern pattern: '[%d] %-5l: (%c) %m\n', 
	color_scheme:'default'
Logging.logger.root.appenders = Logging.appenders.stdout
Logging.logger.root.level = :error

require 'util/assert_helpers'
require 'rubydin'
require 'rubydin-optional-di'
require 'spec_helper/scheduler'
require 'spec_helper/random'
require 'spec_helper/factory_girl'
#require 'spec_helper/machinist'

module FactoryGirl

	class Sequence

		def reset
			@value = 1
		end

	end

end

RSpec.configure do |config|

	config.before :each, :use => :database do
		run_once do
			p '1111'
			dbconfig = load_yaml 'config/database.yml'
			DataMapper::Logger.new $stdout, :error
			DataMapper.setup :default, dbconfig[environment_name]
			DataMapper.auto_migrate!
			DatabaseCleaner.strategy = :transaction
		end
		DatabaseCleaner.start
	end

	config.after :each, :use => :database do
		DatabaseCleaner.clean
		FactoryGirl.sequences.each do |sequence|
			sequence.reset
		end
	end

end
