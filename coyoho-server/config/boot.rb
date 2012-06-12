=begin

This file is part of the CoYoHo - Control Your Home System.

Copyright (C) 2012 Dirk Grappendorf (www.grappendorf.net)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software i 
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

=end

require 'rubygems'
require 'bundler/setup'
require 'logger'
require 'logging'
require 'yaml'
require 'data_mapper'

#require 'simplecov'
# SimpleCov.start do
	# root '/home/grappendorf/workspace-grapelabs/net.grappendorf.coyoho.server'
	# command_name 'cucumber'
# end

environment_name = 'development'

Logging.appenders.stdout.layout = Logging.layouts.pattern(pattern: '[%d] %-5l: (%c) %m\n') 
Logging.logger.root.appenders = Logging.appenders.stdout
Logging.logger.root.level = :info

dbconfig = YAML::load(File.open(File.dirname(File.expand_path(__FILE__)) + '/database.yml'))
DataMapper.setup(:default, dbconfig[environment_name])

$CONFIG = {}
config_yml = File.dirname(File.expand_path(__FILE__)) + "/environments/#{environment_name}.yml"
begin
	$CONFIG = YAML::load File.open config_yml
rescue Exception => x
	puts "Unable to load configuration file '#{config_yml}' : #{x.message}"
end

config_rb = File.dirname(File.expand_path(__FILE__)) + "/environments/#{environment_name}.rb"
begin
	eval File.open(config_rb).read
rescue Exception => x
 	puts "Unable to load configuration file '#{config_rb}' : #{x.message}"
end

($CONFIG['logger'] || {}).each do |logger, config|
	config.each {|option, value| Logging.logger[logger].send(option + '=', value)}
end  

require 'rubydin'
require 'rubydin-optional-di'
require 'rubydin-optional-datamapper'
require 'rubydin_addons'
require 'util/string_helpers'
require 'util/range_helpers'
require 'rufus/scheduler'
require 'thread_storm'
require 'auth/securable'
require 'auth/login_window'
require 'main_window'
require 'dashboard_view'
require 'sysinfo_view'
require 'console_view'
require 'help_view'
require 'settings/settings_view'
require 'devices/device_manager'
require 'devices/device_program_manager'
require 'devices/device_view'
require 'devices/device_program_view'
require 'webservices/rest_api_servlet'
require 'models'

def exit
	Java::java.lang.System.exit 0
end

register(:logger) {Logger.new STDERR}

register(:scheduler) {Rufus::Scheduler.start_new}

register(:async) {ThreadStorm.new size:2}

lookup(:device_manager).start
lookup(:device_program_manager).start
