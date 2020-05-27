#include "common/array.h"
#include "common/debug.h"

#include "comprehend/comprehend.h"
#include "comprehend/game_cc.h"

namespace Comprehend {

// Flags
static const int kFlagSabrinaAction = 0xa;
static const int kFlagErikAction = 0xb;

// Functions
static const int kFuncVampireRoom = 0xe;

static const char *roomImageFiles[]	= {"RA.MS1", "RB.MS1", "RC.MS1"};
static const char *objectImageFiles[]	= {"OA.MS1", "OB.MS1"};

static const struct StringFile stringFiles[] = {
	{"MA.MS1", 0x89, 0x0},
};

const char *ComprehendEngineCrimsonCrown::getMainDataFile() const {
	return "CC1.GDA";
}

Common::Array<struct StringFile> ComprehendEngineCrimsonCrown::getStringFiles() const {
	return Common::Array<struct StringFile>(stringFiles, ARRAYSIZE(stringFiles));
}

Common::Array<const char *> ComprehendEngineCrimsonCrown::getRoomImageFiles() const {
	return Common::Array<const char *>(roomImageFiles, ARRAYSIZE(roomImageFiles));
}

Common::Array<const char *> ComprehendEngineCrimsonCrown::getObjectImageFiles() const {
	return Common::Array<const char *>(objectImageFiles, ARRAYSIZE(objectImageFiles));
}

void ComprehendEngineCrimsonCrown::handleSpecialOpcode(struct functionState *state, struct instruction *instr, struct wordIndex *verb, struct wordIndex *noun) {
	debug("SPECIAL(%02x)", instr->operand[0]);
	switch (instr->operand[0]) {
	case 0x01:
		// Disk 2: Enter the vampire's throne room
		evalFunction(&_gameData->_functions[kFuncVampireRoom], nullptr, nullptr);
		break;

	case 0x03:
		// Game over
		break;

	case 0x05:
		// Win
		break;

	case 0x06:
		// Save game
		break;

	case 0x07:
		// Restore game
		break;
	}
}

void ComprehendEngineCrimsonCrown::beforeTurn(void) {
	_gameData->_flags[kFlagSabrinaAction] = 0;
	_gameData->_flags[kFlagErikAction] = 0;
}

} // End of namespace Comprehend
