#include "ScreenState.h"
#include "ScreenActor.h"
#include "Command.h"
#include <iostream>
#include "MainMenuState.h"

ScreenState* ScreenState::mainMenu = new MainMenuState();

ScreenState::ScreenState()
{
}

ScreenState::~ScreenState() {
}

ScreenState* ScreenState::update(ScreenActor& screen)
{
	return stateToSwitchTo;
}

void ScreenState::enter(ScreenActor& screen)
{
	// Reset variables add this actor to the screen
	_screen = &screen;
	if (this->getParent() != &screen) {
		screen.addChild(this);
	}
	screen.clearText();
	this->stateToSwitchTo = NULL;

	//knowCommands["mail"] = Command::mail;
	//knowCommands["Mail"] = Command::mail;
	//knowCommands["Music"] = Command::music;
	//knowCommands["music"] = Command::music;
}

void ScreenState::switchStates(ScreenState* newState)
{
	stateToSwitchTo = newState;
}

Command* ScreenState::parseVerb(const string& command) {
	auto w = knowCommands.find(command);
	if (w == knowCommands.end()) {
		// Command not found
		return Command::invalid;
	}

	return w->second;
}

void ScreenState::handleInput(const string & command, vector<string> options)
{
	Command* com = parseVerb(command);
	_screen->addText(com->parseNoun(options, this));
}


