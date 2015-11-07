#ifndef COMPREHEND_H
#define COMPREHEND_H

#include "engines/engine.h"
#include "common/random.h"
#include "gui/debugger.h"

#include "comprehend/image_manager.h"
#include "comprehend/game_data.h"
#include "comprehend/renderer.h"
#include "comprehend/console.h"
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

class ComprehendEngine : public Engine {
public:
	ComprehendEngine(OSystem *syst, const ComprehendGameDescription *gd);
	~ComprehendEngine();

	virtual bool hasFeature(EngineFeature f) const;

	void initGame(const ComprehendGameDescription *gd);

	virtual Common::Error run();

	const ComprehendGameDescription *_gameDescription;
	Common::RandomSource *_rnd;

private:
	GameData *_gameData;
	ImageManager _imageManager;
	Renderer *_renderer;
	Console *_console;
	Parser *_parser;
};

} // End of namespace Comprehend

#endif // COMPREHEND_H
