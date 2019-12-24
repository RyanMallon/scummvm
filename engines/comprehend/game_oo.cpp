#include "common/array.h"
#include "common/debug.h"

#include "comprehend/comprehend.h"
#include "comprehend/game_oo.h"

namespace Comprehend {

// Flags
static const int kFlagRoomDark = 0x02;
static const int kFlagRoomBright = 0x19;
static const int kFlagWearingGoggles = 0x1b;
static const int kFlagFlashlightOn = 0x27;

// Rooms
static const int kRoomBrightRoom = 0x19;

static const char *roomImageFiles[]	= {"RA", "RB", "RC", "RD", "RE"};
static const char *objectImageFiles[]	= {"OA", "OB", "OC", "OD"};
static const struct StringFile stringFiles[] = {
	// Strings are stored in the game binary
	{"NOVEL.EXE", 0x16564, 0x17640},
	{"NOVEL.EXE", 0x17702, 0x18600},
	{"NOVEL.EXE", 0x186b2, 0x19b80},
	{"NOVEL.EXE", 0x19c62, 0x1a590},
	{"NOVEL.EXE", 0x1a634, 0x1b080}
};

const char *ComprehendEngineOOTopos::getMainDataFile() const {
	return "G0";
}

Common::Array<struct StringFile> ComprehendEngineOOTopos::getStringFiles() const {
	return Common::Array<struct StringFile>(stringFiles, ARRAYSIZE(stringFiles));
}

Common::Array<const char *> ComprehendEngineOOTopos::getRoomImageFiles() const {
	return Common::Array<const char *>(roomImageFiles, ARRAYSIZE(roomImageFiles));
}

Common::Array<const char *> ComprehendEngineOOTopos::getObjectImageFiles() const {
	return Common::Array<const char *>(objectImageFiles, ARRAYSIZE(objectImageFiles));
}

int ComprehendEngineOOTopos::roomType(unsigned roomIndex) {
	struct room *curRoom = &_gameData->_rooms[_currentRoom];

	if ((curRoom->flags & kRoomDark) && !_gameData->_flags[kFlagFlashlightOn]) {
		// FIXME - roomDesc = 0xb3;
		return kRoomDark;
	}

	if (roomIndex == kRoomBrightRoom && !_gameData->_flags[kFlagWearingGoggles]) {
		// FIXME - roomDesc = 0x1c;
		return kRoomBright;
	}

	return kRoomNormal;
}

void ComprehendEngineOOTopos::handleSpecialOpcode(struct functionState *state, struct instruction *instr, struct wordIndex *verb, struct wordIndex *noun) {
	switch (instr->operand[0]) {
	case 0x03:
		// Game over
		break;
	case 0x05:
		// Game complete
		break;
	case 0x04:
		// Restart game
		break;
	case 0x06:
		// Save game
		break;
	case 0x07:
		// Restore game
		break;
	}
}

void ComprehendEngineOOTopos::beforeTurn(void) {
	struct room *curRoom = &_gameData->_rooms[_currentRoom];

	// Check if the room needs to be redrawn because the flashlight was switched on or off.
	if (_gameData->_flags[kFlagFlashlightOn] != _flashlightOn && curRoom->flags & kFlagRoomDark) {
		_flashlightOn = _gameData->_flags[kFlagFlashlightOn];
		_updateFlags |= kUpdateGraphics | kUpdateRoomDesc;
	}

	// Check if the room needs to be redrawn because the goggles were worn or removed.
	if (_gameData->_flags[kFlagWearingGoggles] != _wearingGoggles && curRoom->flags & kRoomBright) {
		_wearingGoggles = _gameData->_flags[kFlagWearingGoggles];
		_updateFlags |= kUpdateGraphics | kUpdateRoomDesc;
	}
}

} // End of namespace Comprehend
