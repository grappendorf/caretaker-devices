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
require 'util/oo_helpers'

def load_yaml path
	yaml = File.read path	
	yaml = yaml.gsub(/(?:<%=)(.+?)(?:%>)/) {eval $1}
	YAML::load yaml
end

#require 'simplecov'
# SimpleCov.start do
	# root '/home/grappendorf/workspace-grapelabs/net.grappendorf.coyoho.server'
	# command_name 'cucumber'
# end

environment_name = ENV['COYOHO_ENV'] || 'development'

Logging.appenders.stdout.layout = Logging.layouts.pattern pattern: '[%d] %-5l: (%c) %m\n' 
Logging.logger.root.appenders = Logging.appenders.stdout
Logging.logger.root.level = :info

dbconfig = load_yaml File.dirname(File.expand_path(__FILE__)) + '/database.yml'
DataMapper.setup :default, dbconfig[environment_name]

$CONFIG = {}
config_yml = File.dirname(File.expand_path(__FILE__)) + "/environments/#{environment_name}.yml"
begin
	$CONFIG = load_yaml config_yml
rescue Exception => x
	puts "Unable to load configuration file '#{config_yml}' : #{x.message}"
end

config_rb = File.dirname(File.expand_path(__FILE__)) + "/environments/#{environment_name}.rb"
begin
	eval File.read config_rb
rescue Exception => x
 	puts "Unable to load configuration file '#{config_rb}' : #{x.message}"
end

($CONFIG['logger'] || {}).each do |logger, config|
	config.each {|option, value| Logging.logger[logger].send(option + '=', value)}
end  

require 'java'
require 'java/log4j-properties.jar'
require 'java/rxtx-2.2pre2.jar'
require 'java/xbee-api-0.9.jar'
require 'java/log4j-1.2.16.jar'
require 'java/vaadin-chameleon-theme-1.0.2.jar'
require 'java/chameleon-dark.jar'
require 'util/oo_helpers'
require 'util/dm_rest_adapter_helpers'
require 'util/string_helpers'
require 'util/range_helpers'
require 'util/to_xml_helpers'
require 'rubydin'
require 'rubydin-optional-di'
require 'rubydin-optional-datamapper'
require 'rubydin_addons'
require 'rufus/scheduler'
require 'thread_storm'
require 'rack'
require 'auth/securable'
require 'auth/login_window'
require 'settings/settings'
require 'main_window'
require 'dashboard_view'
require 'sysinfo_view'
require 'console_view'
require 'help_view'
require 'settings/settings_view'
require 'devices/device_manager'
require 'devices/device_script_manager'
require 'devices/device_view'
require 'devices/device_script_view'
require 'application'
require 'rubydin-optional-standalone/server'

def exit
	Java::java.lang.System.exit 0
end

console = false

register(:scheduler) {Rufus::Scheduler.start_new}
register(:async) {ThreadStorm.new size:2}

lookup(:device_manager).start
lookup(:device_script_manager).start

rack = Thread.new do
	Rack::Server.start config:'config/config.ru', Port:$CONFIG['api_port']
end

Rubydin::Server.start application_class: CoYoHoApplication, application_paths: ['app', 'config'],
	port:$CONFIG['gui_port']

if console
	puts 'Welcome to the CoYoHo backend console!'
	loop do
		print '> '
		line = gets
		break if not line
		begin
			puts eval(line).to_s
		rescue Exception => x
			puts 'Error: ' + x.message
		end
	end
	puts
	puts 'See you on the other side...'
else
	rack.join
end

exit
