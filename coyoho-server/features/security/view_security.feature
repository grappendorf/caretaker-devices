Feature: 

	Some views are only accessible by administrator user.

	Scenario: 
		
		Administrators can activate secured views
		
		Given I log in as admin 
		Then I should see "Console"

	Scenario: 
	
		Normal users don't see secured view navigation buttons
		
		Given I log in as user 
		Then I should not see "Console"
