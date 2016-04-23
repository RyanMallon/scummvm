#ifndef COMPREHEND_H
#define COMPREHEND_H

#include "engines/engine.h"
#include "common/random.h"
#include "gui/debugger.h"

#include "comprehend/image_manager.h"
#include "comprehend/game_data.h"
#include "comprehend/renderer.h"
#include "comprehend/console.h"
#include "comprehend/opcodes.h"
#include "comprehend/parser.h"

namespace Comprehend {
	class GameData;

struct ComprehendGameDescription;

enum GameType {
	kGameTypeNone	= 0,
	kGameTypeTr
};

// Debug channels
enum {
	kDebugGraphics = (1 << 0)
};

enum {
	kUpdateNone		= 0,
	kUpdateGraphics 	= (1 << 0),
	kUpdateGraphicsObjects	= (1 << 1),
	kUpdateRoomDesc 	= (1 << 2),
	kUpdateObjectList 	= (1 << 3),
	kUpdateAll		= ~0
};

// Room types
enum {
	kRoomNormal,
	kRoomDark
};

struct sentence {
	struct wordIndex *word[4];
	size_t           numWords;
};

class ComprehendEngine : public Engine {
public:
	ComprehendEngine(OSystem *syst, const ComprehendGameDescription *gd);
	~ComprehendEngine();

	virtual bool hasFeature(EngineFeature f) const;

	void initGame(const ComprehendGameDescription *gd);

	virtual Common::Error run();
	void handleSentence(struct sentence *sentence);

	void update(void);
	void moveToRoom(uint8 room);
	void moveObject(struct object *obj, uint8 newRoom);

	struct object *nounToObject(struct wordIndex *noun);

	size_t numObjectsInRoom(uint8 room);
	void showInventory(void);

	void evalFunction(struct function *func, struct wordIndex *verb, struct wordIndex *noun);
	void evalInstruction(struct functionState *state, struct instruction *instr, struct wordIndex *verb, struct wordIndex *noun);

	const ComprehendGameDescription *_gameDescription;
	Common::RandomSource *_rnd;

	// Methods provided by game specific engines
	virtual void handleSpecialOpcode(struct functionState *state, struct instruction *instr, struct wordIndex *verb, struct wordIndex *noun) { }

	virtual int roomType(unsigned roomIndex) {
		return kRoomNormal;
	}

protected:
	GameData *_gameData;
	OpcodeMap *_opcodeMap;
	ImageManager _imageManager;
	Renderer *_renderer;
	Console *_console;
	Parser *_parser;

	// Game state
	unsigned	_updateFlags;
	uint8		_currentRoom;
};

class ComprehendEngineTransylvania : public ComprehendEngine {
public:
	ComprehendEngineTransylvania(OSystem *syst, const ComprehendGameDescription *gd) : ComprehendEngine(syst, gd) { }

	int roomType(unsigned roomIndex);
	void handleSpecialOpcode(struct functionState *state, struct instruction *instr, struct wordIndex *verb, struct wordIndex *noun);
};

} // End of namespace Comprehend

#endif // COMPREHEND_H
