#include "comprehend/comprehend.h"

namespace Comprehend {

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
