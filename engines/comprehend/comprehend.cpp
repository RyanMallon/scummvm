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

void ComprehendEngine::handleSentence(struct sentence *sentence) {
	struct action *action;
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
			if (sentence->word[j] &&
			    sentence->word[j]->index == action->word[j].index &&
			    (sentence->word[j]->type & action->word[j].type)) {
				// Found a matching action
				debug("Sentence matches function %.4x", action->function);
				return;
			}
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
