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

Common::Error ComprehendEngine::run() {
	// Initialize graphics using following:
	initGraphics(320, 200, false);

	const char *roomImageFiles[] = {
		"RA.MS1",
		"RB.MS1",
		"RC.MS1",
	};

	_gameData.loadGameData();

	_imageManager.init(roomImageFiles, 3, NULL, 0);

	_renderer = new Renderer(&_imageManager);
	_console = new Console(_renderer);

	// TESTING
	debug("100, 100 = %x\n", _renderer->getPixel(100, 100));
	debug("100, 190 = %x\n", _renderer->getPixel(100, 190));

	// Additional setup.
	debug("Comprehend::init");

	int index = 0;
	_renderer->drawRoomImage(index++);

	while (1) {
		char *line;

		if (shouldQuit())
			break;

		line = _console->getLine();
		debug("Line: '%s'", line);
	}

	return Common::kNoError;
}

} // End of namespace Comprehend
