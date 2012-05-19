Feature: 

	Some views are only accessible by administrator user.

	Scenario: Administrators
		
		Given I log in as "admin" with password "admin" 
		Then I should see "Logged in as admin"
		Then I should see "Console"
		And I should see "Settings"

	Scenario: Normal users
		
		When I log in as "user" with password "user" 
		Then I should not see "Console"
		And I should not see "Settings"
