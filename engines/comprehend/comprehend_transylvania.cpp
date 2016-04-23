#include "comprehend/comprehend.h"

namespace Comprehend {

int ComprehendEngineTransylvania::roomType(unsigned roomIndex) {
	if (roomIndex == 0x28)
		return kRoomDark;
	return kRoomNormal;
}

} // End of namespace Comprehend
