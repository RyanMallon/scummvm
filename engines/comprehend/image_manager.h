#ifndef COMPREHEND_IMAGE_MANAGER_H
#define COMPREHEND_IMAGE_MANAGER_H

#include "common/array.h"

#include "comprehend/comprehend.h"
#include "comprehend/image_file.h"

namespace Comprehend {

class ImageManager {
public:
	Common::Array<ImageFile *> _roomImageFiles;

	//ImageFile *_roomImageFiles;
	size_t _numRoomFiles;

	ImageManager();
	~ImageManager();
	
	void init(const char *roomFiles[], size_t numRoomFiles, const char *objectFiles[], size_t numObjectFiles);
	void drawRoomImage(uint16 index);
};

} // End of namespace Comprehend

#endif // COMPREHEND_IMAGE_MANAGER_H
