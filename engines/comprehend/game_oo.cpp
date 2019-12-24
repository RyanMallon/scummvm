#include "common/array.h"
#include "common/debug.h"

#include "comprehend/comprehend.h"
#include "comprehend/game_oo.h"

namespace Comprehend {

static const int kFlagRoomDark = 0x02;
static const int kFlagRoomBright = 0x19;
static const int kFlagWearingGoggles = 0x1b;
static const int kFlagFlashlightOn = 0x27;

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

#if 0
void ComprehendEngineOOTopos::beforeTurn(void) {

}
#endif

} // End of namespace Comprehend
