#ifndef COMPREHEND_PARSER_H
#define COMPREHEND_PARSER_H

#include "common/array.h"

#include "comprehend/comprehend.h"
#include "comprehend/game_data.h"

namespace Comprehend {

class GameData;

class Parser {
public:
	Parser(GameData *gameData);
	~Parser();

	void readString(Common::Array<struct sentence *> &sentences, const char *string);

	GameData *_gameData;
};

} // namespace Comprehend

#endif // COMPREHEND_GAME_DATA_H
