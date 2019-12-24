#ifndef COMPREHEND_GAME_TR_H
#define COMPREHEND_GAME_TR_H

#include "comprehend/comprehend.h"

namespace Comprehend {

class ComprehendEngineTransylvania : public ComprehendEngine {
public:
	ComprehendEngineTransylvania(OSystem *syst, const ComprehendGameDescription *gd) : ComprehendEngine(syst, gd) { }

	const char *getMainDataFile() const;
	Common::Array<struct StringFile> getStringFiles() const;

	Common::Array<const char *> getRoomImageFiles() const;
	Common::Array<const char *> getObjectImageFiles() const;

	int roomType(unsigned roomIndex);
	void handleSpecialOpcode(struct functionState *state, struct instruction *instr, struct wordIndex *verb, struct wordIndex *noun);
	void beforeTurn(void);
};

} // End of namespace Comprehend

#endif // COMPREHEND_GAME_TR_H
