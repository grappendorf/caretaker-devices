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

class XbeeMaster

	java_import com.rapplogic.xbee.api.PacketListener
	java_import com.rapplogic.xbee.api.XBee
	java_import com.rapplogic.xbee.api.XBeeAddress64
	java_import com.rapplogic.xbee.api.zigbee.ZNetTxRequest
	java_import com.rapplogic.xbee.api.ApiId

	include PacketListener

	register_as :xbee

	inject :settings

	def initialize
		@logger = Logging.logger[XbeeMaster]
		@xbee = XBee.new
		@message_listeners = []
		@xbee_mutex = Mutex.new
	end

	def start
		begin
			@xbee.open settings.serial_device, settings.baud_rate.rate
			@xbee.add_packet_listener self
		rescue
			@logger.error "Unable to open serial device '#{settings.serial_device}'"
		end
	end

	def stop
		@xbee.remove_packet_listener self
		@xbee.close
	end

	def process_response response
		if response.api_id == ApiId::ZNET_RX_RESPONSE
			@message_listeners.each {|l| l.call response}
		end
	end

	def send_message address, *data
		addr64 = XBeeAddress64.new address.to_java(:int)
		request = ZNetTxRequest.new addr64, data.to_java(:int)
		@xbee_mutex.synchronize do
			@xbee.send_asynchronous request if @xbee.is_connected
		end
	end

	def when_message_received &block
		@message_listeners << block
	end

end