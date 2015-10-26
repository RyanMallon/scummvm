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
#if 0
	_numRoomFiles = numRoomFiles;
	_roomImageFiles = new ImageFile[_numRoomFiles];
	for (i = 0; i < _numRoomFiles; i++) {
		_roomImageFiles[i] = new ImageFile(roomFiles[i]);
		_roomImageFiles[i].loadheader();
	}
#endif

	for (i = 0; i < numRoomFiles; i++) {
		ImageFile *imageFile;

		debug("Adding image file %s", roomFiles[i]);
		imageFile = new ImageFile(roomFiles[i]);
		imageFile->loadHeader();
		_roomImageFiles.push_back(imageFile);
	}

	// FIXME - object image files
}


} // End of namespace Comprehend
