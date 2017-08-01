#include "common/tokenizer.h"
#include "common/array.h"
#include "common/debug.h"

#include "comprehend/comprehend.h"
#include "comprehend/game_data.h"
#include "comprehend/parser.h"

namespace Comprehend {

Parser::Parser(GameData *gameData) : _gameData(gameData) {
}

Parser::~Parser() {
}

void Parser::readString(Common::Array<struct sentence *> &sentences, const char *string) {
	Common::StringTokenizer tokenizer(Common::String(string), " ,");
	Common::String token;
	Common::Array<struct wordIndex *> words;
	struct sentence *sentence;
	size_t i;

	// Convert the string to an array of dictionary words
	while (!tokenizer.empty()) {
		token = tokenizer.nextToken();
		words.push_back(_gameData->lookupDictionaryWord(token.c_str()));
	}

	// FIXME - currently just returning first four words
	sentence = new struct sentence();
	sentence->numWords = MIN(4, (int)words.size());
	for (i = 0; i < sentence->numWords; i++)
		sentence->word[i] = words[i];
	sentences.push_back(sentence);
}

} // namespace Comprehend
