CoYoHo Control Your Home Server
===============================

Installation
------------

### Java

Since we are using JRuby, you need a working Java installation. Any Java 6 SDK or later should
work. If you are using Ubuntu and want to use the original Java 7 from Oracle, you can do the
following:

	sudo apt-get purge openjdk*
	sudo add-apt-repository ppa:webupd8team/java
	sudo apt-get update
	sudo apt-get install oracle-java7-installer
	
Verify the installation with

	java -v
	
### JRuby

We need at least JRuby 1.6.7. Download the .tar.gz file from the JRuby download page 
(http://jruby.org/download). Then do the following as root:

	cd /usr/lib
	tar xzf ~/jruby-bin-<version>.tar.gz
	
Add /usr/lib/jruby-<version>/bin to the PATH variable in /etc/environment. Also add

	JRUBY_HOME="/usr/lib/jruby-<version>"
	
to /etc/environment. Finally modify /usr/lib/jruby-<version>/bin/jruby and change the line
mode="" to mode="--1.9", so that JRuby always runs with Ruby Version 1.9. Re-login and enter

	jruby -v
	
You should see something like

	jruby 1.6.7.2 (ruby-1.9.2-p312)  (2012-05-01 26e08ba) 
		(Java HotSpot(TM) 64-Bit Server VM 1.6.0_32) [linux-amd64-java]

### Bundler

Bundler is a Ruby system for specifying all the gems an application needs. Install it with

	sudo jruby -S gem install bundler -n /usr/bin

Verify the installation with

	bundle -v

### Rubydin

Clone the Rubydin github repository and build and install Rubydin:

	git clone git://github.com/grappendorf/rubydin.git
	cd rubydin
	rake install

### Jetty

When you are using Ubuntu, Jetty can simply installed with

	sudo apt-get install jetty

In /etc/default/jetty change NO_START=1 to NO_START=0 and then add

	JETTY_HOME="/usr/share/jetty"
	
to /etc/environment.

### CoYoHo Server

Now we can build the CoYoHo web application, create a new empty database and deploy it to Jetty:

	bundle install
	rake widgetset
	rake createdb
	rake deploy
	
Finally start Jetty

	sudo service jetty start
	
And access CoYoHo with a web browser at

	http://localhost:8080/coyoho

Licenses
--------

### CoYoHo Server

The CoYoHo Server code is licensed under the the Apache License, Version 2.0
You find the license at http://www.apache.org/licenses/LICENSE-2.0

### Vaadin

The contained Vaadin version is licensed under the the Apache License, Version 2.0
You find the license at http://www.apache.org/licenses/LICENSE-2.0
You find the Vaadin source code at https://vaadin.com/src
You find the Vaadin project at https://vaadin.com

### JRuby

The contained JRuby version is licensed under a three-was CPL/GPL/LGPL license
You find the CPL license at http://www.opensource.org/licenses/cpl1.0.php
You find the GPL license at http://www.gnu.org/licenses/gpl-3.0.txt
You find the LGPL license at http://www.gnu.org/copyleft/lesser.html
You find the JRuby source code at https://github.com/jruby/jruby
You find the JRuby project at http://http://jruby.org

### GWT

The contained GWT version is licensed under the the Apache License, Version 2.0
You find the license at http://www.apache.org/licenses/LICENSE-2.0
You find the GWT source code at http://code.google.com/p/google-web-toolkit
You find the GWT project at https://developers.google.com/web-toolkit

### SLF4J

The contained SLF4J api version is licensed under the MIT license
You find a copy of the license at http://www.slf4j.org/license.html
You find the SLF4J source code at http://www.slf4j.org/download.html
You find the SLF4J project at http://http://www.slf4j.org

### ServerPush

The contained ServerPush version is licensed under the Apache License, Version 2.0
You find the license at http://www.apache.org/licenses/LICENSE-2.0
You find the ServerPush source code at https://github.com/markathomas/ServerPush
You find the ServerPush project at https://vaadin.com/directory#addon/serverpush

### Atmosphere

The contained Atmosphere version is licensed under the Apache License, Version 2.0
You find the license at http://www.apache.org/licenses/LICENSE-2.0
You find the Atmosphere source code at https://github.com/Atmosphere/atmosphere
You find the Atmosphere project at https://github.com/Atmosphere/atmosphere
