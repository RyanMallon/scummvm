#ifndef COMPREHEND_GAME_DATA_H
#define COMPREHEND_GAME_DATA_H

#include "common/file.h"

#include "comprehend/comprehend.h"

namespace Comprehend {

enum SpecialRooms {
	kRoomInventory	= 0x00,
	kRoomNowhere	= 0xff
};

enum ObjectFlags {
	kObjectTakeable	= (1 << 3)
};

enum Direction {
	kDirectionNorth,
	kDirectionSouth,
	kDirectionEast,
	kDirectionWest,
	kDirectionUp,
	kDirectionDown,
	kDirectionIn,
	kDirectionOut,

	kNumDirections
};

enum ActionType {
	kActionVVNN,
	kActionVVN,
	kActionVNJN,
	kActionVJN,
	kActionVDN,
	kActionVNN,
	kActionVN,
	kActionV,

	kNumActionTypes
};

enum WordType {
	kWordNone       = 0x00,
	kWordVerb       = 0x01,
	kWordJoin       = 0x02,
	kWordNounFemale = 0x10,
	kWordNounMale   = 0x20,
	kWordNounNeuter = 0x40,
	kWordNounPlural = 0x80,
	kWordNoun       = (kWordNounFemale | kWordNounMale | kWordNounNeuter | kWordNounPlural)
};

enum DefaultStrings {
	kStringCantGo         = 0,
	kStringDontUnderstand = 1,
	kStringYouSee         = 2,
	kStringInventory      = 3,
	kStringInventoryEmpty = 4,
	kStringBeforeContinue = 5,
	kStringSaveGame       = 6,
	kStringRestoreGame    = 7
};

enum VariableNames {
	kVarTurnCounter  = 2
};

struct room {
	uint8  direction[Comprehend::kNumDirections];
	uint8  flags;
	uint8  graphic;
	uint16 description;
};

struct object {
	uint16 description;
	uint16 longDescription; // Only used by version 2
	uint8  room;
	uint8  flags;
	uint8  word;
	uint8  graphic;
};

struct wordIndex {
	uint8  index;
	uint8  type;
};

struct word {
	char             word[7];
	struct wordIndex index;
};

struct action {
	ActionType       type;
	size_t           numWords;
	struct wordIndex word[4];
	uint16           function;
};

struct instruction {
	uint8  opcode;
	uint8  operand[3];

	size_t numOperands(void) {
		return opcode & 0x3;
	}

	bool isCommand(void) {
		return opcode & 0x80;
	}
};

struct function {
	Common::Array<struct instruction> instructions;
};

class GameData {
public:
	struct Header {
		uint16	magic;

#if 0
		uint16	actionsVVNN;
		uint16	actionsVVN;
		uint16	actionsVNJN;
		uint16	actionsVJN;
		uint16	actionsVDN;
		uint16	actionsVNN;
		uint16	actionsVN;
		uint16	actionsV;
#endif
		uint16  actions[kNumActionTypes];

		uint16	functions;
		uint16	dictionary;
		uint16	wordMap;

		uint16	roomDescriptions;
		uint16	roomDirections[kNumDirections];
		uint16	roomFlags;
		uint16	roomGraphics;

		uint16	objectRooms;
		uint16	objectFlags;
		uint16	objectWords;
		uint16	objectDescriptions;
		uint16	objectGraphics;

		uint16	strings;
		uint16  stringsEnd;
	} _header;

	Common::Array<struct action> _actions;
	Common::Array<char *> _strings;
	Common::Array<struct function> _functions;

	unsigned int _comprehendVersion;
	off_t _headerOffset;
	uint8 _startRoom;

	bool _flags[64];
	uint16 _variables[128];
	Common::Array<char *> _variableWords;
	unsigned int _currentVariableWord;

	size_t _numRooms;
	struct room *_rooms;

	size_t _numObjects;
	struct object *_objects;

	size_t _numWords;
	struct word *_words;

	Common::File _mainFile;

	GameData();
	~GameData();
	void loadGameData(const char *mainDataFile, Common::Array<struct StringFile> stringFiles);

	bool readActionTableHeader(uint8 *word, uint8 *count);
	void loadActionTable(ActionType type, size_t numWords, WordType word1, WordType word2, WordType word3, WordType word4, size_t headerWordIndex);
	void loadActionTableV(void);
	void loadActions(void);

	uint64 getEncodedStringChunk(uint8 *encoded);
	char decodeStringCharacter(uint8 c, bool capital, bool punctuation);
	char *decodeString(Common::File &file);
	unsigned loadStrings(Common::File &file, unsigned index, int32 startOffset, int32 endOffset);
	void loadExtraStrings(Common::Array<struct StringFile> stringFiles);

	void loadVariables(void);
	void loadVariableWords(void);
	void loadFlags(void);
	void loadRooms();
	void loadObjects();
	void loadDictionaryWords();
	void loadFunctions();

	void setVariableWord(unsigned int index, const char *string);
	const char *getVariableWord(unsigned int index);
	const char *getCurrentVariableWord(void);

	bool dictionaryWordMatch(struct word *word, const char *string);
	struct wordIndex *lookupDictionaryWord(const char *string);

	const char *getString(uint16 index);

private:
	void readHeaderAddress(uint16 *addr);
};

} // End of namespace Comprehend

#endif // COMPREHEND_GAME_DATA_H
