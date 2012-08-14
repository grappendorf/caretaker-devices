Feature: 

	View and edit the system settings

	As an administrative user
	I want to view and change the system settings

	Background:
	
		Given I log in as administrator
	
	Scenario: 

		Access the settings view
	
		When I select the Settings view
		Then I should see "Edit system and personal settings"
