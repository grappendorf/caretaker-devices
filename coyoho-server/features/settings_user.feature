Feature: 

	View and edit personal settings

	As a normal user
	I want to view and change my personal settings

	Background:
	
		Given I log in as user
	
	Scenario: 

		Access the settings view
	
		When I select the Settings view
		Then I should see "Edit personal settings"
