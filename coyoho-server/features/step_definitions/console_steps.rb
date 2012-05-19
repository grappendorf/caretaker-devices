When /^I enter "([^"]+)" into the terminal$/ do |text|
	find(:xpath, '//div[@class="term"]//input[@class="i"]').set text + "\n"
end

Then /^I should get the response "([^"]+)"$/ do |response|
	output = find(:xpath, '//div[@class="term"]//pre[@class="b"]')
	output.text.should match /.*\n#{response}$/
end
