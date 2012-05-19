Feature: Log in
	
	In order to use the CoYoHo application, a user should be able to login with 
	his username and password.

	Scenario: 
	
		Accessing the application without being logged in should show the login dialog
		
		Given I go to the home screen
		Then I should see the login dialog		 

	Scenario: Log in as administrator
	
		When I log in as "admin" with password "admin"
		Then I should see "Logged in as admin"

	Scenario: Log in as a normal user
	
		When I log in as "user" with password "user"
		Then I should see "Logged in as user"

	Scenario: Log in with non existing username should give an error
	
		When I log in as "non_existing_user" with password "user"
		Then I should see "Wrong username or password"

	Scenario: Log in with wrong password should give an error
	
		When I log in as "user" with password "wrong_password"
		Then I should see "Wrong username or password"
