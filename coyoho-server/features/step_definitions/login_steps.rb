When /^I log in as "([^"]*)" with password "([^"]*)"$/ do |username,password|
	visit '/'
	find :xpath, '//iframe[@name="PID5"]'
	within_frame('PID5') do
		fill_in 'username', with: username
		fill_in 'password', with: password
		find(:xpath, '//form/div/div').click
	end
end

When /^I log in as (\w+)$/ do |role|
	user = {
		'admin' => ['admin', 'admin'],
		'administrator' => ['admin', 'admin'],
		'user' => ['user', 'user']
	}[role]
	step %Q{I log in as "#{user[0]}" with password "#{user[1]}"}
end

Then /^I should see the login dialog$/ do
	page.should have_content 'Welcome to CoYoHo!'
	find :xpath, '//iframe[@name="PID5"]'
	within_frame('PID5') do
		page.should have_content 'Login'
	end
end
