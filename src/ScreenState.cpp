#include "ScreenState.h"
#include "ScreenActor.h"
#include "Command.h"
#include <iostream>

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
	_screen = &screen;
	screen.addChild(this);
	screen.clearText();

	knowCommands["mail"] = Command::mail;
	knowCommands["Mail"] = Command::mail;
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

void ScreenState::handleInput(const string & command, const string & options)
{
	Command* com = parseVerb(command);
	_screen->addText(com->parseNoun(options, this));
}


