require 'capybara'
require 'capybara/dsl'
require 'capybara/cucumber'
require 'selenium-webdriver'

Capybara.default_driver = :selenium
Capybara.app_host = 'http://localhost:8080/coyoho'
World(Capybara)
