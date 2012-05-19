Feature: Interactive console

	Administrator users can directly enter commands into a terminal in the console view.
	
	Background:
	
		Given I log in as administrator
		And I select the Console view
	
	Scenario Outline: Enter some commands
	
		When I enter "<input>" into the terminal
		Then I should get the response "<output>"

		Examples:
		
		| input					| output					|
		| 1+1					| 2							|
		| 'string'.class		| String					|
		| lookup :application	| .*vaadin.Application.*	|
