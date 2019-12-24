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
	kGameTypeTr,
	kGameTypeOo
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

struct StringFile {
	const char	*name;
	off_t		base_offset;
	off_t		end_offset;
};

class ComprehendEngine : public Engine {
public:
	ComprehendEngine(OSystem *syst, const ComprehendGameDescription *gd);
	~ComprehendEngine();

	virtual bool hasFeature(EngineFeature f) const;

	void initGame(const ComprehendGameDescription *gd);

	virtual Common::Error run();
	void handleSentence(struct sentence *sentence);

	void describeObjectsInCurrentRoom(void);
	void update(void);

	virtual void beforeTurn(void) { }

	bool randomly(uint8 value);

	struct object *getObject(int index);
	bool playerInRoom(uint8 room);
	bool objectInRoom(uint8 obj, uint8 room);
	void moveToRoom(uint8 room);
	void moveObject(struct object *obj, uint8 newRoom);
	void moveObject(int objIndex, uint8 newRoom);

	struct object *nounToObject(struct wordIndex *noun);

	size_t numObjectsInRoom(uint8 room);
	void showInventory(void);

	void evalFunction(struct function *func, struct wordIndex *verb, struct wordIndex *noun);
	void evalInstruction(struct functionState *state, struct instruction *instr, struct wordIndex *verb, struct wordIndex *noun);

	const ComprehendGameDescription *_gameDescription;
	Common::RandomSource *_rnd;

	// Methods provided by game specific engines
	virtual void handleSpecialOpcode(struct functionState *state, struct instruction *instr, struct wordIndex *verb, struct wordIndex *noun) { }

	virtual const char *getMainDataFile() const {
		return nullptr;
	}

	virtual Common::Array<struct StringFile> getStringFiles() const {
		return Common::Array<struct StringFile>();
	};

	virtual Common::Array<const char *> getRoomImageFiles() const {
		return Common::Array<const char *>();
	}

	virtual Common::Array<const char *> getObjectImageFiles() const {
		return Common::Array<const char *>();
	}

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

} // End of namespace Comprehend

#endif // COMPREHEND_H
