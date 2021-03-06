#include "Room.h"
#include <stdlib.h>
#include "Enemy.h"
#include "Item.h"
#include "SoundManager.h"

using namespace std;

Room::Room(int floor)
{
	_floor = floor;
	_roomDescription = rand() % 4; // Random number for description
}

Room::~Room()
{
}

string Room::enteringRoom()
{
    SoundManager::playEvent("event:/Sfx/Footsteps");
	if (enemies.size() > 0) {
		for (auto enemy : enemies) {
			enemy->onEnter();
		}
	}
	return getDescritption() + " " +  getExits();
}

string Room::getExits()
{
	string exitDesc = "You see ";
    int numberOfExits = 0;
	//Check all exits if they exists
	auto w = exits.find("n");
	if (w != exits.end()) {
		exitDesc += exits["n"]->getShortDescription() + " to the north ";
        numberOfExits++;
	}
	w = exits.find("s");
	if (w != exits.end()) {
        if (numberOfExits > 0) {
            exitDesc += "and " + exits["s"]->getShortDescription() + " to the south ";
        } else {
            exitDesc += exits["s"]->getShortDescription() + " to the south ";
        }
        numberOfExits++;
	}
	w = exits.find("e");
	if (w != exits.end()) {
        if (numberOfExits > 0){
            exitDesc += "and " + exits["e"]->getShortDescription() +  " to the east ";
        } else {
            exitDesc += exits["e"]->getShortDescription() +  " to the east ";
        }
        numberOfExits++;
	}
	w = exits.find("w");
	if (w != exits.end()) {
        if (numberOfExits > 0){
            exitDesc += "and " + exits["w"]->getShortDescription() +  " to the west ";
        } else {
            exitDesc += exits["w"]->getShortDescription() +  " to the west ";
        }
        numberOfExits++;
	}
	w = exits.find("up");
	if (w != exits.end()) {
		exitDesc += "and staris up ";
	}
	w = exits.find("down");
	if (w != exits.end()) {
		exitDesc += "and stairs down ";
	}

	return exitDesc;
}


string Room::getShortDescription() {
    if ( _roomDescription < 0){
        _roomDescription = rand() % 4;
    }
    if (_roomDescription == 0) {
        return "a grimey room";
    } else if (_roomDescription == 1) {
        return "a cool looking room";
    } else if (_roomDescription == 2) {
        return "a boring looking room";
    } else if (_roomDescription == 3) {
        return "a room with the sound of flapping";
    }
    
    return "This room has no description.";
}

int Room::getFloor()
{
	return _floor;
}

string Room::getDescritption()
{
	string desc = "";
    if ( _roomDescription < 0){
        _roomDescription = rand() % 4;
    }
	if (_roomDescription == 0) {
		desc += "Peeling wallpaper is slowly falling off the walls. You can see the time on the walls based on the thin level of grime covering the floor.";
	}
	else if (_roomDescription == 1) {
		desc += "Broken tech lies in the corner of the room. Each corner of the brown carpet on the floor is starting to seperate from the floor underneath.";
	}
	else if (_roomDescription == 2) {
		desc += "Everything is so plain here that it physically hurts you from the bordom you're experiencing.";
	}
	else if (_roomDescription == 3) {
		desc += "Butterflies fill the room with humming that generates a white noise the penatrates your ears. The longer you stay in the room the louder it gets.";
	}
	else {
		desc += "This room has no description.";
	}

	if (enemies.size() > 0) {
        for (auto enemy : enemies){
           		desc += " " + enemy->getDescription();
        }
	}

	if (items.size() > 0) {
		int numItems = 0;
		for (auto item : items) {
			if (numItems > 0) {
				desc += " and a " + item->getDescription();
			}
			else {
				desc += " There is a " + item->getDescription();
			}
			numItems++;
		}
		desc += " in the room.";
	}
    
    return desc;
}

void Room::update(const UpdateState &us) {
    if (enemies.size() > 0) {
        for (auto enemy : enemies) {
            enemy->update(us);
        }
    }
}

void Room::generateEnemies(Level& level)
{
	int spawnEnemy = rand() % 3;
	if (spawnEnemy >= 2) {
		enemies.push_back( make_shared<Enemy>(level));
	}
}
