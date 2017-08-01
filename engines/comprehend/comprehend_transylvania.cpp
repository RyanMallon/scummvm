#include "common/array.h"
#include "common/debug.h"

#include "comprehend/comprehend.h"

namespace Comprehend {

static const char *roomImageFiles[]	= {"RA.MS1", "RB.MS1", "RC.MS1"};
static const char *objectImageFiles[]	= {"OA.MS1", "OB.MS1", "OC.MS1"};

static const struct StringFile stringFiles[] = {
	{"MA.MS1", 0x88},
	{"MB.MS1", 0x88},
	{"MC.MS1", 0x88},
	{"MD.MS1", 0x88},
	{"ME.MS1", 0x88},
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
		break;

	case 0x09:
		// Zin splash screen
		_renderer->drawRoomImage(41);
		_console->waitKey();
		_updateFlags = kUpdateGraphics;
		break;
	}
}

/*

if (!in_current_room(vampire)) {
    moveObject(vampire, nowhere);
}

..

if (inRoom(hut)) {
    if (inCurrentRoom(cat)) {
        if (random(0.5)) {
            showString(0x6d); // loud hissing meow
        }
    }
}

*/

// Rooms
static const int kRoomHutInterior = 0x7;
static const int kRoomFlagForest = (1 << 6);

// Objects
static const int kObjectCat = 0x18;

// Strings
static const int kStringMeow = 0x6d;
static const int kStringForestMessageBase = 0x62;
static const int kStringEagle = 0x6b;

void ComprehendEngineTransylvania::beforeTurn(void) {
	struct room *curRoom = &_gameData->_rooms[_currentRoom];
	int stringIndex, roomIndex;

	// 50% chance for the cat to meow at the player in the hut
	if (playerInRoom(kRoomHutInterior) && objectInRoom(kObjectCat, kRoomHutInterior) && randomly(128))
		_console->writeWrappedText(_gameData->getString(kStringMeow));

	//
	// In the forest there is a small chance to either get a random
	// message or get picked up by the eagle and taken elsewhere in
	// the forest.
	//
	if ((curRoom->flags & kRoomFlagForest) && _gameData->_variables[kVarTurnCounter] >= 4 && randomly(255 - 39)) {
		stringIndex = _rnd->getRandomNumberRng(kStringForestMessageBase,kStringEagle);
		_console->writeWrappedText(_gameData->getString(stringIndex));
		if (stringIndex == kStringEagle) {
			// Pick a random room in the forest to drop the player
			roomIndex = _rnd->getRandomNumberRng(1, 4);
			if (roomIndex == _currentRoom)
				roomIndex += 0xf;

			// FIXME - zero the vampire/werewolf
			moveToRoom(roomIndex);
		}
	}
}

} // End of namespace Comprehend
