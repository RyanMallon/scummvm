#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"

#include "common/system.h"
#include "common/events.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/file.h"
#include "common/fs.h"

#include "engines/util.h"

#include "comprehend/comprehend.h"
#include "comprehend/image_manager.h"
#include "comprehend/game_data.h"
#include "comprehend/opcodes.h"
#include "comprehend/parser.h"

namespace Comprehend {

ComprehendEngine::ComprehendEngine(OSystem *syst, const ComprehendGameDescription *gd) : Engine(syst), _gameDescription(gd) {
	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from run().

	// Do not initialize graphics here
	// Do not initialize audio devices here

	// However this is the place to specify all default directories
	const Common::FSNode gameDataDir(ConfMan.get("path"));

	DebugMan.addDebugChannel(kDebugGraphics, "graphics", "Debug graphics");

	// Don't forget to register your random source
	_rnd = new Common::RandomSource("comprehend");

	debug("ComprehendEngine::ComprehendEngine");
}

ComprehendEngine::~ComprehendEngine() {
	debug("ComprehendEngine::~ComprehendEngine");

	// Dispose your resources here
	delete _rnd;

	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();
}

bool ComprehendEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
}

struct functionState {
	bool   testResult;
	bool   elseResult;
	size_t orCount;
	bool   isAnd;
	bool   inCommand;
	bool   executed;

	void setTestResult(bool value) {
		if (orCount == 0) {
			// and
			if (isAnd) {
				if (!value)
					testResult = false;
			} else {
				testResult = value;
				isAnd = true;
			}
		} else {
			// or
			if (value)
				testResult = value;
		}
	}
};

struct object *ComprehendEngine::getObject(int index) {
	if (index <= 0 || index > (int)_gameData->_numObjects)
		return NULL;

	return &_gameData->_objects[index - 1];
}

bool ComprehendEngine::randomly(uint8 value) {
	return _rnd->getRandomNumberRng(0, 255) >= value;
}

bool ComprehendEngine::playerInRoom(uint8 room) {
	return _currentRoom == room;
}

bool ComprehendEngine::objectInRoom(uint8 objIndex, uint8 room) {
	struct object *obj;

	obj = getObject(objIndex);
	if (obj)
		return obj->room == room;
	return false;
}

void ComprehendEngine::moveToRoom(uint8 room) {
	if (room != _currentRoom)
		_updateFlags = (kUpdateGraphics | kUpdateRoomDesc | kUpdateObjectList);
	_currentRoom = room;
}

void ComprehendEngine::moveObject(struct object *obj, uint8 newRoom) {
	//unsigned obj_weight = item->flags & ITEMF_WEIGHT_MASK;

	if (obj->room == newRoom)
		return;

	if (obj->room == kRoomInventory) {
		// Removed from player's inventory
		//game->info->variable[VAR_INVENTORY_WEIGHT] -= obj_weight;
	}
	if (newRoom == kRoomInventory) {
		// Moving to the player's inventory
		// game->info->variable[VAR_INVENTORY_WEIGHT] += obj_weight;
	}

	if (obj->room == _currentRoom) {
		// Object moved away from the current room. Need a full redraw.
		_updateFlags |= kUpdateGraphics;

	} else if (newRoom == _currentRoom) {
		// Object moved into the current room. Only the object needs a
		// redraw, not the whole room.
		_updateFlags |= (kUpdateGraphicsObjects | kUpdateObjectList);
	}

	obj->room = newRoom;
}

void ComprehendEngine::moveObject(int objIndex, uint8 newRoom) {
	struct object *obj;

	obj = getObject(objIndex);
	if (obj)
		moveObject(obj, newRoom);
}

struct object *ComprehendEngine::nounToObject(struct wordIndex *noun) {
	struct object *obj;
	size_t i;

	if (!noun || !(noun->type & kWordNoun))
		return NULL;

	//
	// FIXME - in oo-topos the word 'box' matches more than one object
	//         (the box and the snarl-in-a-box). The player is unable
	//         to drop the latter because this will match the former.
	//
	for (i = 0; i < _gameData->_numObjects; i++) {
		obj = &_gameData->_objects[i];
		if (obj->word == noun->index)
			return obj;
	}

	return NULL;
}

size_t ComprehendEngine::numObjectsInRoom(uint8 room) {
	size_t count, i;

	for (i = 0, count = 0; i < _gameData->_numObjects; i++)
		if (_gameData->_objects[i].room == room)
			count++;

	return count;
}

void ComprehendEngine::showInventory(void) {
	size_t i, numObjects;
	struct object *obj;

	numObjects = numObjectsInRoom(kRoomInventory);
	if (numObjects == 0) {
		_console->writeWrappedText(_gameData->_strings[kStringInventoryEmpty]);
		return;
	}

	_console->writeWrappedText(_gameData->_strings[kStringInventory]);
	for (i = 0; i < _gameData->_numObjects; i++) {
		obj = &_gameData->_objects[i];

		if (obj->room == kRoomInventory)
			_console->writeWrappedText(_gameData->getString(obj->description));
	}
}

void ComprehendEngine::evalInstruction(struct functionState *state, struct instruction *instr, struct wordIndex *verb, struct wordIndex *noun) {
	struct room *room;
	struct object *obj;
	uint16 index;

	if (state->orCount)
		state->orCount--;

	if (instr->isCommand()) {
		state->inCommand = true;
		state->orCount = 0;
		if (!state->testResult)
			return;

		state->elseResult = false;
		state->executed = true;

	} else {
		if (state->inCommand) {
			// Finished sequence of commands - clear the result
			state->inCommand = false;
			state->testResult = false;
			state->isAnd = false;
		}
	}

	switch (_opcodeMap->_map[instr->opcode]) {
	case OPCODE_VAR_ADD:
		_gameData->_variables[instr->operand[0]] += _gameData->_variables[instr->operand[1]];
		break;

	case OPCODE_VAR_SUB:
		_gameData->_variables[instr->operand[0]] -= _gameData->_variables[instr->operand[1]];
		break;

	case OPCODE_VAR_INC:
		_gameData->_variables[instr->operand[0]]++;
		break;

	case OPCODE_VAR_DEC:
		_gameData->_variables[instr->operand[0]]--;
		break;

	case OPCODE_VAR_EQ:
		state->setTestResult(_gameData->_variables[instr->operand[0]] == _gameData->_variables[instr->operand[1]]);
		break;

	case OPCODE_TURN_TICK:
		_gameData->_variables[kVarTurnCounter]++;
		break;

	case OPCODE_TEST_ROOM_FLAG:
		room = &_gameData->_rooms[_currentRoom];
		state->setTestResult(room->flags & instr->operand[0]);
		break;

	case OPCODE_TEST_NOT_ROOM_FLAG:
		room = &_gameData->_rooms[_currentRoom];
		state->setTestResult(!(room->flags & instr->operand[0]));
		break;

	case OPCODE_PRINT:
		index = (instr->operand[1] << 8 | instr->operand[0]);
		_console->writeWrappedText(_gameData->getString(index));
		break;

	case OPCODE_MOVE:
		room = &_gameData->_rooms[_currentRoom];
		if (room->direction[verb->index - 1])
			moveToRoom(room->direction[verb->index - 1]);
		else
			_console->writeWrappedText(_gameData->_strings[kStringCantGo]);
		break;

	case OPCODE_MOVE_TO_ROOM:
		if (instr->operand[0] == 0xff) {
			// This was used for copy protection hooks in the original games. Ignore it.
			break;
		}

		moveToRoom(instr->operand[0]);
		break;

	case OPCODE_IN_ROOM:
		state->setTestResult(_currentRoom == instr->operand[0]);
		break;

	case OPCODE_NOT_IN_ROOM:
		state->setTestResult(_currentRoom != instr->operand[0]);
		break;

	case OPCODE_CURRENT_OBJECT_NOT_VALID:
		state->setTestResult(nounToObject(noun) == NULL);
		break;

	case OPCODE_CURRENT_OBJECT_PRESENT:
		obj = nounToObject(noun);
		if (!obj)
			state->setTestResult(false);
		else
			state->setTestResult(obj->room == _currentRoom);
		break;

	case OPCODE_CURRENT_OBJECT_NOT_PRESENT:
		obj = nounToObject(noun);
		if (!obj)
			state->setTestResult(true);
		else
			state->setTestResult(obj->room != _currentRoom);
		break;

	case OPCODE_CURRENT_OBJECT_IS_NOWHERE:
		obj = nounToObject(noun);
		if (!obj)
			state->setTestResult(false);
		else
			state->setTestResult(obj->room == kRoomNowhere);
		break;

	case OPCODE_CURRENT_OBJECT_IS_NOT_NOWHERE:
		obj = nounToObject(noun);
		if (!obj)
			state->setTestResult(false);
		else
			state->setTestResult(obj->room != kRoomNowhere);
		break;

	case OPCODE_OBJECT_IS_NOWHERE:
		obj = &_gameData->_objects[instr->operand[0] - 1];
		state->setTestResult(obj->room == kRoomNowhere);
		break;

	case OPCODE_OBJECT_IS_NOT_NOWHERE:
		obj = &_gameData->_objects[instr->operand[0] - 1];
		state->setTestResult(obj->room != kRoomNowhere);
		break;

	case OPCODE_OBJECT_IN_ROOM:
		obj = &_gameData->_objects[instr->operand[0] - 1];
		state->setTestResult(obj->room == instr->operand[1]);
		break;

	case OPCODE_CURRENT_OBJECT_NOT_TAKEABLE:
		obj = nounToObject(noun);
		if (!obj)
			state->setTestResult(false);
		else
			state->setTestResult(!(obj->flags & kObjectTakeable));
		break;

	case OPCODE_HAVE_CURRENT_OBJECT:
		obj = nounToObject(noun);
		if (!obj)
			state->setTestResult(false);
		else
			state->setTestResult(obj->room == kRoomInventory);
		break;

	case OPCODE_NOT_HAVE_CURRENT_OBJECT:
		obj = nounToObject(noun);
		if (!obj)
			state->setTestResult(true);
		else
			state->setTestResult(obj->room != kRoomInventory);
		break;

	case OPCODE_HAVE_OBJECT:
		obj = &_gameData->_objects[instr->operand[0] - 1];
		state->setTestResult(obj->room == kRoomInventory);
		break;

	case OPCODE_NOT_HAVE_OBJECT:
		obj = &_gameData->_objects[instr->operand[0] - 1];
		state->setTestResult(obj->room != kRoomInventory);
		break;

	case OPCODE_TAKE_CURRENT_OBJECT:
		obj = nounToObject(noun);
		if (obj)
			moveObject(obj, kRoomInventory);
		break;

	case OPCODE_TAKE_OBJECT:
		obj = &_gameData->_objects[instr->operand[0] - 1];
		moveObject(obj, kRoomInventory);
		break;

	case OPCODE_DROP_CURRENT_OBJECT:
		obj = nounToObject(noun);
		if (obj)
			moveObject(obj, _currentRoom);
		break;

	case OPCODE_OBJECT_PRESENT:
		obj = &_gameData->_objects[instr->operand[0] - 1];
		state->setTestResult(obj->room == _currentRoom);
		break;

	case OPCODE_OBJECT_NOT_PRESENT:
		obj = &_gameData->_objects[instr->operand[0] - 1];
		state->setTestResult(obj->room != _currentRoom);
		break;

	case OPCODE_MOVE_OBJECT_TO_ROOM:
		obj = &_gameData->_objects[instr->operand[0] - 1];
		moveObject(obj, instr->operand[1]);
		break;

	case OPCODE_MOVE_OBJECT_TO_CURRENT_ROOM:
		obj = &_gameData->_objects[instr->operand[0] - 1];
		moveObject(obj, _currentRoom);
		break;

	case OPCODE_REMOVE_OBJECT:
		obj = &_gameData->_objects[instr->operand[0] - 1];
		moveObject(obj, kRoomNowhere);
		break;

	case OPCODE_REMOVE_CURRENT_OBJECT:
		obj = nounToObject(noun);
		if (obj)
			moveObject(obj, kRoomNowhere);
		break;

	case OPCODE_INVENTORY_FULL:
		// FIXME - no inventory limit yet
		state->setTestResult(false);
		break;

	case OPCODE_SET_ROOM_DESCRIPTION:
		room = &_gameData->_rooms[instr->operand[0]];
		// FIXME - common string assignment?
		switch (instr->operand[2]) {
		case 0x80:
			room->description = instr->operand[1];
			break;
		case 0x81:
			room->description = instr->operand[1] + 0x100;
			break;
		case 0x82:
			room->description = instr->operand[1] + 0x200;
			break;
		}
		break;

	case OPCODE_SPECIAL:
		/* Game specific */
		handleSpecialOpcode(state, instr, verb, noun);
		break;

	case OPCODE_CALL_FUNC:
		index = instr->operand[0];
		if (instr->operand[1] == 0x81)
			index += 0x100;
		evalFunction(&_gameData->_functions[index], verb, noun);
		break;

	case OPCODE_SAVE_ACTION:
		// FIXME - implement this
		break;

	case OPCODE_INVENTORY:
		showInventory();
		break;

	case OPCODE_SET_FLAG:
		_gameData->_flags[instr->operand[0]] = 1;
		break;

	case OPCODE_CLEAR_FLAG:
		_gameData->_flags[instr->operand[0]] = 0;
		break;

	case OPCODE_TEST_FLAG:
		state->setTestResult(_gameData->_flags[instr->operand[0]]);
		break;

	case OPCODE_TEST_NOT_FLAG:
		state->setTestResult(!_gameData->_flags[instr->operand[0]]);
		break;

	case OPCODE_OR:
		if (state->orCount) {
			state->orCount += 2;
		} else {
			state->testResult = false;
			state->orCount += 3;
		}
		break;

	case OPCODE_ELSE:
		state->testResult = state->elseResult;
		break;

	case OPCODE_SET_STRING_REPLACEMENT:
		_gameData->_currentVariableWord = instr->operand[0] - 1;
		break;

	case OPCODE_SET_CURRENT_NOUN_STRING_REPLACEMENT:
		// TODO - Not sure what the operand is for. Possibly capitalisation?
		if (noun) {
			switch (noun->type) {
			case kWordNounPlural:
				_gameData->_currentVariableWord = 3;
				break;
			case kWordNounFemale:
				_gameData->_currentVariableWord = 0;
				break;
			case kWordNounMale:
				_gameData->_currentVariableWord = 1;
				break;
			default:
				_gameData->_currentVariableWord = 2;
				break;
			}
		}
		break;

	default:
		debugN("UNHANDLED: [%.2x] ", instr->opcode);
		for (size_t i = 0; i < instr->numOperands(); i++)
			debugN("%.2x ", instr->operand[i]);
		debugN("\n");
		break;
	}
}

void ComprehendEngine::evalFunction(struct function *func, struct wordIndex *verb, struct wordIndex *noun) {
	struct functionState state;
	size_t i;

	memset(&state, 0, sizeof(state));
	state.testResult = true;
	state.elseResult = true;
	state.executed = false;

	for (i = 0; i < func->instructions.size(); i++) {
		if (state.executed && !func->instructions[i].isCommand()) {
			// At least one command has been executed and the
			// current instruction is a test. Exit the function.
			break;
		}

		evalInstruction(&state, &func->instructions[i], verb, noun);
	}
}

void ComprehendEngine::handleSentence(struct sentence *sentence) {
	struct action *action;
	struct wordIndex *verb = NULL, *noun = NULL;
	size_t i, j;

	// Find a matching action
	for (i = 0; i < _gameData->_actions.size(); i++) {
		action = &_gameData->_actions[i];

		//
		// If the action type is verb only then it may optionally
		// have a noun. All other action types require an exact
		// number of words.
		//
		if (action->type == kActionV && sentence->numWords > action->numWords + 1)
			continue;
		if (action->type != kActionV && sentence->numWords != action->numWords)
			continue;

		for (j = 0; j < action->numWords; j++) {
			if (!(sentence->word[j] &&
			      sentence->word[j]->index == action->word[j].index &&
			      (sentence->word[j]->type & action->word[j].type)))
				break;
		}
		if (j == action->numWords) {
			verb = sentence->word[0];
			if (sentence->numWords > 1)
				noun = sentence->word[1];
			evalFunction(&_gameData->_functions[action->function], verb, noun);
			return;
		}
	}

	_console->writeWrappedText(_gameData->_strings[kStringDontUnderstand]);
}

void ComprehendEngine::describeObjectsInCurrentRoom(void) {
	bool printedHeader = false;
	struct object *obj;
	size_t i;

	for (i = 0; i < _gameData->_numObjects; i++) {
		obj = &_gameData->_objects[i];

		if (obj->room == _currentRoom && obj->description != 0) {
			if (!printedHeader) {
				_console->writeWrappedText(_gameData->_strings[kStringYouSee]);
				printedHeader = true;
			}

			_console->writeWrappedText(_gameData->getString(obj->description));
		}
	}
}

void ComprehendEngine::update(void) {
	struct room *room = &_gameData->_rooms[_currentRoom];
	struct object *obj;
	bool drawObjects = true;
	size_t i;

	// Draw the current room image
	switch (roomType(_currentRoom)) {
	case kRoomDark:
		if (_updateFlags & kUpdateGraphics)
			_renderer->drawDarkRoom();
		drawObjects = false;
		break;

	case kRoomBright:
		if (_updateFlags & kUpdateGraphics)
			_renderer->drawBrightRoom();
		drawObjects = false;
		break;

	default:
		if (_updateFlags & kUpdateGraphics)
			_renderer->drawRoomImage(room->graphic - 1);
		break;
	}

	if (drawObjects && ((_updateFlags & kUpdateGraphics) || (_updateFlags & kUpdateGraphicsObjects))) {
		for (i = 0; i < _gameData->_numObjects; i++) {
			obj = &_gameData->_objects[i];

			if (obj->room == _currentRoom)
				_renderer->drawObjectImage(obj->graphic - 1);
		}
	}

	if (_updateFlags & kUpdateRoomDesc)
		_console->writeWrappedText(_gameData->getString(room->description));

	if ((_updateFlags & kUpdateObjectList) && roomType(_currentRoom) == kRoomNormal)
		describeObjectsInCurrentRoom();

	_updateFlags = kUpdateNone;
}

Common::Error ComprehendEngine::run() {
	// Initialize graphics using following:
	initGraphics(320, 200, false);

	_gameData = new GameData();
	_gameData->loadGameData(getMainDataFile(), getStringFiles());

	switch (_gameData->_comprehendVersion) {
	case 1:
	    _opcodeMap = new OpcodeMapV1();
	    break;

	case 2:
	default:
	    _opcodeMap = new OpcodeMapV2();
	    break;
	}

	_imageManager.init(getRoomImageFiles(), getObjectImageFiles());

	_renderer = new Renderer(&_imageManager);
	_console = new Console(_renderer, _gameData);
	_parser = new Parser(_gameData);

	// FIXME - read from data file
	_currentRoom = _gameData->_startRoom;

	titleSequence();

	_updateFlags = kUpdateAll;
	while (1) {
		Common::Array<struct sentence *> sentences;
		char *line;
		size_t i;

		if (shouldQuit())
			break;

		/* Run the each turn functions */
		beforeTurn();
		evalFunction(&_gameData->_functions[0], NULL, NULL);

		/* Update graphics */
		update();

		line = _console->getLine();

		_parser->readString(sentences, line);
		for (i = 0; i < sentences.size(); i++)
			handleSentence(sentences[i]);
	}

	return Common::kNoError;
}

} // End of namespace Comprehend
