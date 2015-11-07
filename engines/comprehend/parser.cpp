#include "common/tokenizer.h"
#include "common/array.h"
#include "common/debug.h"

#include "comprehend/game_data.h"
#include "comprehend/parser.h"

namespace Comprehend {

Parser::Parser(GameData *gameData) : _gameData(gameData) {
}

Parser::~Parser() {
}

void Parser::readString(const char *string) {
	Common::StringTokenizer tokenizer(Common::String(string), " ,");
	Common::String token;
	Common::Array<struct wordIndex *> words;

	// Convert string to dictionary words
	while (!tokenizer.empty()) {
		token = tokenizer.nextToken();
		words.push_back(_gameData->lookupDictionaryWord(token.c_str()));
	}

	size_t i;
	for (i = 0; i < words.size(); i++) {
		if (words[i])
			debug("%.2x:%.2x", words[i]->index, words[i]->type);
		else
			debug("unknown");
	}
}

} // namespace Comprehend
