require 'capybara'
require 'capybara/dsl'
require 'capybara/cucumber'
require 'selenium-webdriver'


Capybara.register_driver :selenium do |app|
	profile = Selenium::WebDriver::Firefox::Profile.new
	profile["intl.accept_languages"] = "en"
 	Capybara::Selenium::Driver.new(app, browser: :firefox, profile: profile)
end

Capybara.default_driver = :selenium
Capybara.default_wait_time = 10
Capybara.app_host = 'http://localhost:8080'
World(Capybara)
