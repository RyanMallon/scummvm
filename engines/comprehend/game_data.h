#ifndef COMPREHEND_GAME_DATA_H
#define COMPREHEND_GAME_DATA_H

#include "common/file.h"

#include "comprehend/comprehend.h"

namespace Comprehend {

#define NR_DIRECTIONS	8

class GameData {
public:
	struct Header {
		uint16	magic;

		uint16	actionsVVNN;
		uint16	actionsVVN;
		uint16	actionsVNJN;
		uint16	actionsVJN;
		uint16	actionsVDN;
		uint16	actionsVNN;
		uint16	actionsVN;
		uint16	actionsV;
		
		uint16	roomDescTable;
		uint16	roomDirectionTable[NR_DIRECTIONS];
		uint16	roomFlagsTable;
		uint16	roomGraphicsTable;
		
		uint16	itemLocations;
		uint16	itemFlags;
		uint16	itemWords;
		uint16	itemStrings;
		uint16	itemGraphics;
		
		uint16	dictionary;
		uint16	wordMap;
		
		uint16	strings;
		uint16	stringsEnd;
		
		uint16	functions;	
	} _header;

	Common::File _mainFile;

	GameData();
	~GameData();
	void loadGameData();

private:
	uint16 readHeaderAddress();
};

} // End of namespace Comprehend

#endif // COMPREHEND_GAME_DATA_H
