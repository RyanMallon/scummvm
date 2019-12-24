
#include "engines/advancedDetector.h"

#include "common/system.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "graphics/thumbnail.h"
#include "graphics/surface.h"

#include "comprehend/comprehend.h"
#include "comprehend/game_tr.h"
#include "comprehend/game_oo.h"

namespace Comprehend {

struct ComprehendGameDescription {
	ADGameDescription	desc;
	GameType		gameType;
};

static const PlainGameDescriptor comprehendGames[] = {
	{"tr", "Transylvania"},
	{"oo", "OO-Topos"},
	{0, 0},
};

static const ComprehendGameDescription gameDescriptions[] = {
	{
		// Transylvania
		{
			"tr",
			"DOS",
			AD_ENTRY1("TR.GDA", "22e08633eea02ceee49b909dfd982d22"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		kGameTypeTr
	},
	{
		// OO-Topos
		{
			"oo",
			"DOS",
			AD_ENTRY1("G0", "56460c1ee669c253607534155d7e9db4"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		kGameTypeOo
	},

	{AD_TABLE_END_MARKER, kGameTypeNone}
};

class ComprehendMetaEngine : public AdvancedMetaEngine {
public:
	ComprehendMetaEngine() : AdvancedMetaEngine(gameDescriptions, sizeof(ComprehendGameDescription), comprehendGames) {
	}

	const char *getName() const {
		return "Comprehend";
	}

	const char *getOriginalCopyright() const {
		return "Comprehend Engine (C) 1985 Polarware";
	}

	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const;
	bool hasFeature(MetaEngineFeature f) const;

	int getMaximumSaveSlot() const;
	SaveStateList listSaves(const char *target) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
	void removeSaveState(const char *target, int slot) const;
};

bool ComprehendMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	const ComprehendGameDescription *cgd = (const ComprehendGameDescription *)gd;

	if (cgd) {
		// FIXME
		switch (cgd->gameType) {
		case kGameTypeTr:
			*engine = new ComprehendEngineTransylvania(syst, cgd);
			break;

		case kGameTypeOo:
			*engine = new ComprehendEngineOOTopos(syst, cgd);
			break;

		default:
			return false;
		}

		// FIXME - unused?
		((ComprehendEngine *)*engine)->initGame(cgd);
	}

	return cgd != 0;
}

bool ComprehendMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves);
}

int ComprehendMetaEngine::getMaximumSaveSlot() const {
	return 3;
}

SaveStateList ComprehendMetaEngine::listSaves(const char *target) const {
	SaveStateList saveList;

	return saveList;
}

SaveStateDescriptor ComprehendMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	return SaveStateDescriptor();
}

void ComprehendMetaEngine::removeSaveState(const char *target, int slot) const {

}

void ComprehendEngine::initGame(const ComprehendGameDescription *gd) {
}

} // End of namespace Comprehend

#if PLUGIN_ENABLED_DYNAMIC(COMPREHEND)
	REGISTER_PLUGIN_DYNAMIC(COMPREHEND, PLUGIN_TYPE_ENGINE, Comprehend::ComprehendMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(COMPREHEND, PLUGIN_TYPE_ENGINE, Comprehend::ComprehendMetaEngine);
#endif
