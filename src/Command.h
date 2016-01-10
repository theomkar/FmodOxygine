#pragma once
#include <vector>
#include <string>
#include <sstream>

using namespace std;
class ScreenState;
class InvalidCommand;
class Mail;
class Music;
class Look;
class Go;
class Clear;
class Quit;
class MainMenu;
class Reset;

class Command {
public:
	virtual ~Command() {};
	virtual string parseNoun(vector<string> options, ScreenState* state) = 0;

	// List all commands as static variables
	static Command* invalid;
	static Command* mail;
	static Command* music;
	static Command* look;
	static Command* go;
	static Command* clear;
	static Command* quit;
	static Command* mainMenu;
	static Command* reset;

	//TODO: Allow for questions and responses to commands (mini states?)
};