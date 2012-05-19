Given /^I (?:go to|am at) the home screen$/ do
	visit '/'
end

Then /^I should see "([^"]*)"$/ do |text|
	page.should have_content text
end

Then /^I should not see "([^"]*)"$/ do |text|
	page.should_not have_content text
end

When /^I (?:select|activate|switch to) the (\w+) view$/ do |view|
	find(:xpath, "//span[@class='v-button-caption' and text() = '#{view}']").click
end

	