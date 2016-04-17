#include "comprehend/comprehend.h"
#include "comprehend/image_manager.h"
#include "comprehend/image_file.h"

#include "common/debug.h"

namespace Comprehend {

ImageManager::ImageManager() {
}

ImageManager::~ImageManager() {
}

void ImageManager::init(const char *roomFiles[], size_t numRoomFiles, const char *objectFiles[], size_t numObjectFiles) {
	size_t i;

	// Load room image files
	for (i = 0; i < numRoomFiles; i++) {
		ImageFile *imageFile;

		debug("Adding room image file %s", roomFiles[i]);
		imageFile = new ImageFile(roomFiles[i]);
		imageFile->loadHeader();
		_roomImageFiles.push_back(imageFile);
	}

	// FIXME - object image files
	for (i = 0; i < numObjectFiles; i++) {
		ImageFile *imageFile;

		debug("Adding object image file %s", objectFiles[i]);
		imageFile = new ImageFile(objectFiles[i]);
		imageFile->loadHeader();
		_objectImageFiles.push_back(imageFile);
	}
}

} // End of namespace Comprehend
