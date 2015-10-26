
#include "comprehend/comprehend.h"
#include "comprehend/game_data.h"

#include "common/debug.h"

namespace Comprehend {

GameData::GameData() {
}

GameData::~GameData() {
}

uint16 GameData::readHeaderAddress(void) {
	uint16 addr;

	addr = _mainFile.readUint16LE();
	
	return addr - 0x5a00 + 4;
}

void GameData::loadGameData(void) {
 	if (!_mainFile.open("TR.GDA"))
		error("File not found: %s", "TR.GDA");

	// Read header
	_header.magic = _mainFile.readUint16LE();

	// Second word is unknown
	_mainFile.readUint16LE();

	_header.actionsVVNN = readHeaderAddress();
	_header.actionsVVN = readHeaderAddress();
	_header.actionsVNJN = readHeaderAddress();
	_header.actionsVJN = readHeaderAddress();
	_header.actionsVDN = readHeaderAddress();
	_header.actionsVNN = readHeaderAddress();
	_header.actionsVN = readHeaderAddress();
	_header.actionsV = readHeaderAddress();

	debug("magic: %x\n", _header.magic);
	debug("VVNN:  %x\n", _header.actionsVVNN);

}

} // End of namespace Comprehend
