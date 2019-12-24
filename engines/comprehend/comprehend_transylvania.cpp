#include "common/array.h"
#include "common/debug.h"

#include "comprehend/comprehend.h"
#include "comprehend/game_tr.h"

namespace Comprehend {

// Flags
static const int kFlagVampireDead = 5;
static const int kFlagWerewolfDead = 7;

// Rooms
static const int kRoomHutInterior = 0x7;
static const int kRoomGoblin = 0x1a;
static const int kRoomFlagForest = (1 << 6);
static const int kRoomFlagCastle = (1 << 7);

// Objects
static const int kObjectGoblin = 0x0a;
static const int kObjectCat = 0x18;
static const int kObjectGarlic = 0x20;
static const int kObjectWerewolf = 0x22;
static const int kObjectVampire = 0x27;

// Strings
static const int kStringMeow = 0x6d;
static const int kStringForestMessageBase = 0x62;
static const int kStringEagle = 0x6b;
static const int kStringGoblinMessageBase = 0x5e;
static const int kStringGoblinMessageEnd = 0x61;

// Variables
static const int kVarTimeout = 0x0f;

static const char *roomImageFiles[]	= {"RA.MS1", "RB.MS1", "RC.MS1"};
static const char *objectImageFiles[]	= {"OA.MS1", "OB.MS1", "OC.MS1"};

static const struct StringFile stringFiles[] = {
	{"MA.MS1", 0x88, 0x0},
	{"MB.MS1", 0x88, 0x0},
	{"MC.MS1", 0x88, 0x0},
	{"MD.MS1", 0x88, 0x0},
	{"ME.MS1", 0x88, 0x0},
};

const char *ComprehendEngineTransylvania::getMainDataFile() const {
	return "TR.GDA";
}

Common::Array<struct StringFile> ComprehendEngineTransylvania::getStringFiles() const {
	return Common::Array<struct StringFile>(stringFiles, ARRAYSIZE(stringFiles));
}

Common::Array<const char *> ComprehendEngineTransylvania::getRoomImageFiles() const {
	return Common::Array<const char *>(roomImageFiles, ARRAYSIZE(roomImageFiles));
}

Common::Array<const char *> ComprehendEngineTransylvania::getObjectImageFiles() const {
	return Common::Array<const char *>(objectImageFiles, ARRAYSIZE(objectImageFiles));
}

int ComprehendEngineTransylvania::roomType(unsigned roomIndex) {
	if (roomIndex == 0x28)
		return kRoomDark;
	return kRoomNormal;
}

void ComprehendEngineTransylvania::handleSpecialOpcode(struct functionState *state, struct instruction *instr, struct wordIndex *verb, struct wordIndex *noun) {
	switch (instr->operand[0]) {
	case 0x01:
		// FIXME - called when the mice are dropped and the cat chases them.
		break;

	case 0x02:
		// FIXME - called whe the gun is fired
		break;

	case 0x06:
		// FIXME - save game
		break;

	case 0x07:
		// FIXME - restore game
		break;

	case 0x03:
		// Game over - lose
	case 0x05:
		// Game over - win
		_console->waitKey();
		quitGame();
		break;

	case 0x09:
		// Zin splash screen
		_renderer->drawRoomImage(41);
		_console->waitKey();
		_updateFlags = kUpdateGraphics;
		break;
	}
}

void ComprehendEngineTransylvania::beforeTurn(void) {
	struct room *curRoom = &_gameData->_rooms[_currentRoom];
	int stringIndex, roomIndex, turnCount;

	turnCount = _gameData->_variables[kVarTurnCounter];

	if (getObject(kObjectVampire)->room != _currentRoom)
		moveObject(kObjectVampire, kRoomNowhere);
	if (getObject(kObjectWerewolf)->room != _currentRoom)
		moveObject(kObjectWerewolf, kRoomNowhere);

	//
	// 50% chance for the cat to meow at the player in the hut
	//
	if (playerInRoom(kRoomHutInterior)) {
		if (objectInRoom(kObjectCat, kRoomHutInterior) && randomly(128))
			_console->writeWrappedText(_gameData->getString(kStringMeow));
		return;
	}

	//
	// The Goblin pulls a random prank if he is in the room
	//
	if (getObject(kObjectGoblin)->room == _currentRoom) {
		stringIndex = _rnd->getRandomNumberRng(kStringGoblinMessageBase, kStringGoblinMessageEnd);
		_console->writeWrappedText(_gameData->getString(stringIndex));
		return;
	}

	//
	// In the castle there is a ~20% chance for the vampire to appear
	// if you are not holding the garlic.
	//
	if (curRoom->flags & kRoomFlagCastle) {
		if (getObject(kObjectGarlic)->room != kRoomInventory && !_gameData->_flags[kFlagVampireDead] && randomly(200)) {
			moveObject(kObjectVampire, _currentRoom);
			_gameData->_variables[kVarTimeout] = _gameData->_variables[kVarTurnCounter] + 1;
			return;
		}
	}

	//
	// In the forest there is a small chance to either get a random
	// message or get picked up by the eagle and taken elsewhere in
	// the forest.
	//
	if ((curRoom->flags & kRoomFlagForest) && turnCount >= 4 && randomly(255 - 39)) {
		stringIndex = _rnd->getRandomNumberRng(kStringForestMessageBase,kStringEagle);
		_console->writeWrappedText(_gameData->getString(stringIndex));
		if (stringIndex == kStringEagle) {
			// Pick a random room in the forest to drop the player
			roomIndex = _rnd->getRandomNumberRng(1, 4);
			if (roomIndex == _currentRoom)
				roomIndex += 0xf;

			moveToRoom(roomIndex);
			return;
		}
	}

	//
	// After turn 10 the werewolf has ~33% to appear in the forest
	//
	if ((curRoom->flags & kRoomFlagForest) && !_gameData->_flags[kFlagWerewolfDead] && turnCount >= 10 && randomly(172)) {
		moveObject(kObjectWerewolf, _currentRoom);
		_gameData->_variables[kVarTimeout] = _gameData->_variables[kVarTurnCounter] + 1;
		return;
	}
}

} // End of namespace Comprehend
