#!/bin/env ruby

require 'optparse'
require 'yaml'
require 'tempfile'
require 'ostruct'

class Array
  def pad length
    self + [0] * [length - self.length, 0].max
  end
end

def checksum address, bytes
  count = bytes.length +
      ((address & 0xff00) >> 8) +
      (address & 0x00ff) +
      bytes.reduce(:+)
  ((count & 0x00ff) ^ 0x00ff) + 1
end

def ihex_line address, bytes
  bytes_hex = bytes.map { |b| '%02x' % b }.join
  sprintf ":%02x%04x00%s%02x\n", bytes.length, address, bytes_hex, checksum(address, bytes)
end

def ihex start_address, chunk_size, bytes
  bytes.each_slice(chunk_size).each_with_index.map do |chunk, i|
    ihex_line (start_address + i * chunk_size), chunk
  end.join + ":00000001ff\n"
end

def avrdude_command options, ihex_file_path
  "avrdude -c #{options.programmer} -p #{options.part} -U eeprom:w:#{ihex_file_path}"
end

options = OpenStruct.new
options.pretend = false
options.programmer = 'dragon_isp'
options.part = 'm328p'

opt_parser = OptionParser.new do |opts|
  opts.banner = 'Usage: configurator.rb <config_file>'

  opts.on '-c <programmer>', 'Specify programmer type (default: dragon_isp)' do |arg|
    options.programmer = arg
  end

  opts.on '-p <part>', 'Specify AVR device (default: m328p)' do |arg|
    options.part = arg
  end

  opts.on '-n', '--dry-run', 'Don\'t program, only print ihex file and avrdude command' do
    options.pretend = true
  end

  opts.on_tail '-h', '--help', 'Show this message' do
    puts 'Caretaker Device Configurator'
    puts
    puts 'Programs the device EEPROM with the contents of a configuration YAML file'
    puts
    puts opts
    exit
  end
end

opt_parser.parse! ARGV
if ARGV.length == 0
  puts opt_parser.help
  exit
end

config = YAML.load_file ARGV.first
bytes = [0xFE, 0xCA] +
    config['guid'].bytes.pad(37) +
    config['name'].bytes.pad(33) +
    config['ssid'].bytes.pad(33) +
    config['phrase'].bytes.pad(65)

ihex_content = ihex 0, 32, bytes

unless options.pretend
  Tempfile.create 'caretaker-device-config-' do |f|
    bytes.each_slice(32).each_with_index do |chunk, i|
      f.write ihex_line i * 32, chunk
    end
    f.write ":00000001ff\n"
    f.flush
    system avrdude_command options, f.path
  end
else
  puts "EEPROM IHEX file:\n\n"
  puts ihex_content
  puts "\nAVRDUDE command:\n\n"
  puts avrdude_command options, '<ihex_file>'
end
