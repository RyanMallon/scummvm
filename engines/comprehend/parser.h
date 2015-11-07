#ifndef COMPREHEND_PARSER_H
#define COMPREHEND_PARSER_H

#include "comprehend/game_data.h"

namespace Comprehend {

class GameData;

class Parser {
public:
	Parser(GameData *gameData);
	~Parser();

	void readString(const char *string);

	GameData *_gameData;
};

} // namespace Comprehend

#endif // COMPREHEND_GAME_DATA_H

