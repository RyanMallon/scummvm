#include "common/array.h"

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

} // End of namespace Comprehend
