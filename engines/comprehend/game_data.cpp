
#include "comprehend/comprehend.h"
#include "comprehend/game_data.h"
#include "comprehend/opcodes.h"

#include "common/debug.h"

namespace Comprehend {

static char kCharset[]            = "..abcdefghijklmnopqrstuvwxyz .";
static char kCharsetPunctuation[] = "[]\n!\"#$%&'(),-/0123456789:;?<>";

#define __readArray(type, offset, array, member, base, size)		\
	do {								\
		size_t __i;						\
		_mainFile.seek(offset, SEEK_SET);			\
		for (__i = (base); __i < (base) + (size); __i++)	\
			(array)[__i].member = _mainFile.read##type();	\
	} while (0)

#define readArray8(offset, array, member, base, size)			\
	__readArray(Byte, offset, array, member, base ,size)

#define readArray16(offset, array, member, base, size)			\
	__readArray(Uint16LE, offset, array, member, base, size)

GameData::GameData() {
}

GameData::~GameData() {
}

void GameData::readHeaderAddress(uint16 *addr) {
	*addr = _mainFile.readUint16LE() - 0x5a00 + 4;
}

bool GameData::readActionTableHeader(uint8 *word, uint8 *count) {
	*word = _mainFile.readByte();
	if (*word == 0)
		return false;
	*count = _mainFile.readByte();
	return true;
}

void GameData::loadActionTable(ActionType type, size_t numWords, WordType word1, WordType word2, WordType word3, WordType word4, size_t headerWordIndex) {
	struct action action;
	uint8 headerWord, count;
	size_t i, j;

	while (1) {
		if (!readActionTableHeader(&headerWord, &count))
			break;

		for (i = 0; i < count; i++) {
			action.type = type,
			action.numWords = numWords;
			action.word[0].type = word1;
			action.word[1].type = word2;
			action.word[2].type = word3;
			action.word[3].type = word4;

			for (j = 0; j < numWords; j++) {
				if (j == headerWordIndex)
					action.word[j].index = headerWord;
				else
					action.word[j].index = _mainFile.readByte();
			}

			action.function = _mainFile.readUint16LE();

			_actions.push_back(action);
		}
	}
}

void GameData::loadActionTableV(void) {
	struct action action;
	uint8 verb, numFuncs;
	uint16 func;
	size_t i;

	while (1) {
		verb = _mainFile.readByte();
		if (verb == 0)
			break;

		action.type = kActionV;
		action.numWords = 1;
		action.word[0].type = kWordVerb;
		action.word[0].index = verb;

		// Count is the number of functions, but only the first appears
		// to be used.
		numFuncs = _mainFile.readByte();
		for (i = 0; i < numFuncs; i++) {
			func = _mainFile.readUint16LE();
			if (i == 0)
				action.function = func;
		}

		_actions.push_back(action);
	}
}

void GameData::loadActions(void) {
	ActionType type;
	size_t i;

	for (i = kActionVVNN; i < kNumActionTypes; i++) {
		type = static_cast<ActionType>(i);

		_mainFile.seek(_header.actions[type], SEEK_SET);
		switch (type) {
		case kActionVVNN:
			loadActionTable(type, 4, kWordVerb, kWordVerb, kWordNoun, kWordNoun, 0);
			break;

		case kActionVNJN:
			loadActionTable(type, 4, kWordVerb, kWordNoun, kWordJoin, kWordNoun, 2);
			break;

		case kActionVJN:
			loadActionTable(type, 3, kWordVerb, kWordJoin, kWordNoun, kWordNone, 1);
			break;

		case kActionVDN:
			loadActionTable(type, 3, kWordVerb, kWordVerb, kWordNoun, kWordNone, 0);
			break;

		case kActionVNN:
			loadActionTable(type, 3, kWordVerb, kWordNoun, kWordNoun, kWordNone, 0);
			break;

		case kActionVN:
			loadActionTable(type, 2, kWordVerb, kWordNoun, kWordNone, kWordNone, 0);
			break;

		case kActionV:
			loadActionTableV();
			break;

		default:
			break;
		}
	}

#if 0
	debug("%u actions", (unsigned)_actions.size());
	for (i = 0; i < _actions.size(); i++) {
		debugN("[%.4x] (", (unsigned)i);
		for (j = 0; j < 4; j++) {
			if (j < _actions[i].numWords) {
				switch (_actions[i].word[j].type) {
				case kWordVerb: debugN("V"); break;
				case kWordNoun: debugN("N"); break;
				case kWordJoin: debugN("J"); break;
				default: debugN("?"); break;
				}
			} else {
				debugN(" ");
			}
		}
		debugN(") ");

		for (j = 0; j < _actions[i].numWords; j++)
			debugN("%.2x:%.2x ",
			       _actions[i].word[j].index,
			       _actions[i].word[j].type);

		debug(" -> %.4x", _actions[i].function);
	}
#endif
}

uint64 GameData::getEncodedStringChunk(uint8 *encoded) {
	uint64 c, val = 0;
	size_t i;

	for (i = 0; i < 5; i++) {
		c = encoded[i] & 0xff;
		val |= (c << ((4 - i) * 8));
	}

	return val;
}

char GameData::decodeStringCharacter(uint8 c, bool capital, bool punctuation)
{
	if (punctuation) {
		if (c < sizeof(kCharsetPunctuation) - 1)
			return kCharsetPunctuation[c];
	} else {
		if (c < sizeof(kCharset) - 1) {
			c = kCharset[c];
			if (capital) {
				//
				// A capital space means that the character
				// is dynamically replaced by at runtime.
				// We use the character '@' since it cannot
				// otherwise appear in strings.
				//
				if (c == ' ')
					return '@';
				return c - 0x20;
			} else {
				return c;
			}
		}
	}

	/* Unknown character */
	debug("Unknown char %d, capital=%d, punctuation=%d", c, capital, punctuation);
	return '*';
}


char *GameData::decodeString(Common::File &file) {
	bool nextIsCapital = false, nextIsPunctuation = false;
	char *string, c;
	unsigned i, j, k = 0;
	uint64 chunk;
	uint8 encoded[1024], elem;
	size_t encodedLen;

	// Get the encoded string
	for (encodedLen = 0; encodedLen < sizeof(encoded) - 1; encodedLen++) {
		encoded[encodedLen] = file.readByte();
		if (encoded[encodedLen] == '\0')
			break;
	}

	if (encodedLen == 0)
		return NULL;

	string = (char *)malloc(encodedLen * 2);

	for (i = 0; i < encodedLen; i += 5) {
		chunk = getEncodedStringChunk(&encoded[i]);

		for (j = 0; j < 8; j++) {
			elem = (chunk >> (35 - (5 * j))) & 0x1f;

			if (elem == 0)
				goto done;
			if (elem == 0x1e) {
				nextIsCapital = true;
			} else if (elem == 0x1f) {
				nextIsPunctuation = true;
			} else {
				c = decodeStringCharacter(elem, nextIsCapital, nextIsPunctuation);
				nextIsCapital = false;
				nextIsPunctuation = false;
				string[k++] = c;
			}
		}
	}

done:
	string[k] = '\0';

	return string;
}

unsigned GameData::loadStrings(Common::File &file, unsigned index, int32 startOffset, int32 endOffset) {
	char *string;
	unsigned count = 0;

	file.seek(startOffset, SEEK_SET);
	while (1) {
		string = decodeString(file);
		if (!string || file.pos() >= endOffset)
			break;

		_strings.insert_at(index + count, string);
		count++;
	}

	return count;
}

void GameData::loadExtraStrings(Common::Array<struct StringFile> stringFiles) {
	Common::File file;
	unsigned i, index;

	for (i = 0; i < stringFiles.size(); i++) {
		if (!file.open(stringFiles[i].name))
			error("String file %s not found", stringFiles[i].name);

		index = 0x200 + (i * 0x40) + (i == 0 ? 1 : 0);
		_strings.resize(index);
		loadStrings(file, index, stringFiles[i].offset, file.size());
		file.close();
	}
}

const char *GameData::getString(uint16 index) {
	index &= 0x7fff;

	if (index >= _strings.size() || !_strings[index]) {
		debug("Bad string index %.4x", (unsigned)index);
		return "BAD_STRING";
	}

	return _strings[index];
}

void GameData::loadFunctions(void) {
	struct function func;
	struct instruction instr;
	bool done = false;
	size_t i;

	_mainFile.seek(_header.functions, SEEK_SET);

	while (!done) {
		// Load instructions for this function
		memset(&func, 0, sizeof(func));

		while (1) {
			instr.opcode = _mainFile.readByte();
			if (instr.opcode == 0) {
				if (func.instructions.size() == 0)
					done = true;
				break;
			}

			for (i = 0; i < instr.numOperands(); i++)
				instr.operand[i] = _mainFile.readByte();

			func.instructions.push_back(instr);
		}

		if (!done)
			_functions.push_back(func);
	}

#if 0
	// Dump
	int j, k;
	debug("%d functions", _functions.size());
	for (i = 0; i < _functions.size(); i++) {
		debug("[%.4x]", i);
		for (j = 0; j < _functions[i].instructions.size(); j++) {
			instr = _functions[i].instructions[j];

			debugN("    [%.2x] ", instr.opcode);
			for (k = 0; k < instr.numOperands(); k++)
				debugN("%.2x ", instr.operand[k]);
			debug("");
		}
	}
#endif

}

void GameData::loadRooms(void) {
	size_t i;

	//
	// Rooms are indexed from 1 because room 0 is a special room for
	// player inventory.
	//
	_numRooms = _header.roomDirections[kDirectionSouth] - _header.roomDirections[kDirectionNorth];
	_rooms = new struct room[_numRooms + 1];

	for (i = 0; i < kNumDirections; i++)
		readArray8(_header.roomDirections[i], _rooms, direction[i], 1, _numRooms);
	readArray16(_header.roomDescriptions, _rooms, description, 1, _numRooms);
	readArray8(_header.roomFlags, _rooms, flags, 1, _numRooms);
	readArray8(_header.roomGraphics, _rooms, graphic, 1, _numRooms);

#if 0
	debug("%u rooms", (unsigned)_numRooms);
	for (i = 1; i <= _numRooms; i++)
		debug("[%.2x] dir=%.2x,%.2x,%.2x,%.2x,%.2x,%.2x,%.2x,%.2x desc=%.4x, flags=%.2x, gfx=%.2x",
		      (unsigned)i,
		      _rooms[i].direction[kDirectionNorth],
		      _rooms[i].direction[kDirectionSouth],
		      _rooms[i].direction[kDirectionEast],
		      _rooms[i].direction[kDirectionWest],
		      _rooms[i].direction[kDirectionUp],
		      _rooms[i].direction[kDirectionDown],
		      _rooms[i].direction[kDirectionIn],
		      _rooms[i].direction[kDirectionOut],
		      _rooms[i].description,
		      _rooms[i].flags,
		      _rooms[i].graphic);
#endif
}

void GameData::loadObjects(void) {
	// FIXME - use vectors?
	_numObjects = _header.objectWords - _header.objectFlags;
	_objects = new struct object[_numObjects];

	readArray16(_header.objectDescriptions, _objects, description, 0, _numObjects);
	readArray8(_header.objectFlags, _objects, flags, 0, _numObjects);
	readArray8(_header.objectWords, _objects, word, 0, _numObjects);
	readArray8(_header.objectRooms, _objects, room, 0, _numObjects);
	readArray8(_header.objectGraphics, _objects, graphic, 0, _numObjects);

#if 0
	debug("%u objects", (unsigned)_numObjects);
	for (size_t i = 0; i < _numObjects; i++)
		debug("[%.2x] desc=%.4x flags=%.2x word=%.2x room=%.2x gfx=%.2x",
		      (unsigned)i,
		      _objects[i].description,
		      _objects[i].flags,
		      _objects[i].word,
		      _objects[i].room,
		      _objects[i].graphic);
#endif
}

void GameData::loadDictionaryWords(void) {
	size_t i, j;

	_numWords = (_header.wordMap - _header.dictionary) / 8;
	_words = new struct word[_numWords];

	_mainFile.seek(_header.dictionary, SEEK_SET);
	for (i = 0; i < _numWords; i++) {
		// The word string is 6 characters and xor'ed with 0x8a.
		_mainFile.read(_words[i].word, 6);
		for (j = 0; j < 6; j++) {
			_words[i].word[j] ^= 0x8a;
			_words[i].word[j] = toupper(_words[i].word[j]);
		}
		_words[i].word[6] = '\0';

		_words[i].index.index = _mainFile.readByte();
		_words[i].index.type = _mainFile.readByte();
	}

#if 0
	debug("%u words", (unsigned)_numWords);
	for (i = 0; i < _numWords; i++)
		debug("[%.4x] %.2x:%.2x word=%s",
		      (unsigned)i,
		      _words[i].index.index,
		      _words[i].index.type,
		      _words[i].word);
#endif
}

void GameData::loadVariables(void) {
	int i;

	for (i = 0; i < ARRAYSIZE(_variables); i++)
		_variables[i] = _mainFile.readUint16LE();
}

void GameData::loadFlags(void) {
	int i, bit, index = 0;
	uint8 bitmask;

	for (i = 0; i < ARRAYSIZE(_flags) / 8; i++) {
		bitmask = _mainFile.readByte();
		for (bit = 7; bit >= 0; bit--) {
			_flags[index] = !!(bitmask & (1 << bit));
			index++;
		}
	}
}

void GameData::loadGameData(const char *mainDataFile, Common::Array<struct StringFile> stringFiles) {
	uint16 dummy, stringsEnd;

	if (!_mainFile.open(mainDataFile))
		error("Main data file %s not found", mainDataFile);

	// Read header
	_header.magic = _mainFile.readUint16LE();

	// Second word is unknown
	_mainFile.readUint16LE();

	// Action tables
	readHeaderAddress(&_header.actions[kActionVVNN]);
	readHeaderAddress(&_header.actions[kActionVVN]);
	readHeaderAddress(&_header.actions[kActionVNJN]);
	readHeaderAddress(&_header.actions[kActionVJN]);
	readHeaderAddress(&_header.actions[kActionVDN]);
	//readHeaderAddress(&_header.actionsVNN);
	readHeaderAddress(&_header.actions[kActionVN]);
	readHeaderAddress(&_header.actions[kActionV]);

	readHeaderAddress(&_header.functions);
	readHeaderAddress(&_header.dictionary);
	readHeaderAddress(&_header.wordMap);

	// Unknown
	_mainFile.readUint16LE();

	// Rooms
	readHeaderAddress(&_header.roomDescriptions);
	readHeaderAddress(&_header.roomDirections[kDirectionNorth]);
	readHeaderAddress(&_header.roomDirections[kDirectionSouth]);
	readHeaderAddress(&_header.roomDirections[kDirectionEast]);
	readHeaderAddress(&_header.roomDirections[kDirectionWest]);
	readHeaderAddress(&_header.roomDirections[kDirectionUp]);
	readHeaderAddress(&_header.roomDirections[kDirectionDown]);
	readHeaderAddress(&_header.roomDirections[kDirectionIn]);
	readHeaderAddress(&_header.roomDirections[kDirectionOut]);
	readHeaderAddress(&_header.roomFlags);
	readHeaderAddress(&_header.roomGraphics);

	// Objects
	readHeaderAddress(&_header.objectRooms);
	readHeaderAddress(&_header.objectFlags);
	readHeaderAddress(&_header.objectWords);
	readHeaderAddress(&_header.objectDescriptions);
	readHeaderAddress(&_header.objectGraphics);

	readHeaderAddress(&_header.strings);

	// FIXME
	readHeaderAddress(&dummy);
	readHeaderAddress(&stringsEnd);

	_mainFile.readByte(); // Unknown
	_startRoom = _mainFile.readByte();
	_mainFile.readByte(); // Unknown

        loadVariables();
        loadFlags();

	loadActions();
	loadRooms();
	loadObjects();
	loadDictionaryWords();

	loadStrings(_mainFile, 0, _header.strings, stringsEnd);
	loadExtraStrings(stringFiles);

#if 0
	size_t i;
	debug("%u strings", _strings.size());
	for (i = 0; i < _strings.size(); i++)
		debug("[%.4x] %s", i, _strings[i]);
#endif

	loadFunctions();
}

bool GameData::dictionaryWordMatch(struct word *word, const char *string) {
	// Words less than 6 characters must match exactly
	if (strlen(word->word) < 6 && strlen(string) != strlen(word->word))
		return false;

	return strncmp(word->word, string, strlen(word->word)) == 0;
}

struct wordIndex *GameData::lookupDictionaryWord(const char *string) {
	size_t i;

	for (i = 0; i < _numWords; i++) {
		if (dictionaryWordMatch(&_words[i], string))
			return &_words[i].index;
	}

	// Not found
	return NULL;
}

} // End of namespace Comprehend
