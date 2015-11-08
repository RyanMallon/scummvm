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

struct function_state {
	bool   testResult;
	bool   elseResult;
	size_t orCount;
	bool   isAnd;
	bool   inCommand;
	bool   executed;
};

void ComprehendEngine::evalInstruction(struct function_state *state, struct instruction *instr, struct wordIndex *verb, struct wordIndex *noun) {
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
	case OPCODE_PRINT:
		index = (instr->operand[1] << 8 | instr->operand[0]) & 0x7fff;
		debug("String index %.4x", index);
		if (index < _gameData->_strings.size())
			_console->writeWrappedText(_gameData->_strings[index]);
		else
			_console->writeWrappedText("BAD STRING");
		break;
	}
}

void ComprehendEngine::evalFunction(struct function *func, struct wordIndex *verb, struct wordIndex *noun) {
	struct function_state state;
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
			debug("Sentence matches function %.4x", action->function);
			verb = sentence->word[0];
			if (sentence->numWords > 1)
				noun = sentence->word[1];
			evalFunction(&_gameData->_functions[action->function], verb, noun);
			return;
		}
	}

	_console->writeWrappedText(_gameData->_strings[kStringDontUnderstand]);
}

Common::Error ComprehendEngine::run() {
	// Initialize graphics using following:
	initGraphics(320, 200, false);

	const char *roomImageFiles[] = {
		"RA.MS1",
		"RB.MS1",
		"RC.MS1",
	};

	_gameData = new GameData();
	_gameData->loadGameData();
	_opcodeMap = new OpcodeMapV1();

	_imageManager.init(roomImageFiles, 3, NULL, 0);

	_renderer = new Renderer(&_imageManager);
	_console = new Console(_renderer);
	_parser = new Parser(_gameData);

	// TESTING
	debug("100, 100 = %x\n", _renderer->getPixel(100, 100));
	debug("100, 190 = %x\n", _renderer->getPixel(100, 190));

	// Additional setup.
	debug("Comprehend::init");

	int index = 0;
	_renderer->drawRoomImage(index++);

	while (1) {
		Common::Array<struct sentence> sentences;
		char *line;
		size_t i;

		if (shouldQuit())
			break;

		line = _console->getLine();
		debug("Line: '%s'", line);

		sentences = _parser->readString(line);
		for (i = 0; i < sentences.size(); i++)
			handleSentence(&sentences[i]);
	}

	return Common::kNoError;
}

} // End of namespace Comprehend
