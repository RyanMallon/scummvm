#include "comprehend/comprehend.h"
#include "comprehend/image_manager.h"
#include "comprehend/image_file.h"

#include "common/debug.h"

namespace Comprehend {

ImageManager::ImageManager() {
}

ImageManager::~ImageManager() {
}

void ImageManager::init(Common::Array<const char *>roomFiles, Common::Array<const char *>objectFiles) {
	size_t i;

	// Load room image files
	for (i = 0; i < roomFiles.size(); i++) {
		ImageFile *imageFile;

		imageFile = new ImageFile(roomFiles[i]);
		imageFile->loadHeader();
		_roomImageFiles.push_back(imageFile);
	}

	// FIXME - object image files
	for (i = 0; i < objectFiles.size(); i++) {
		ImageFile *imageFile;

		imageFile = new ImageFile(objectFiles[i]);
		imageFile->loadHeader();
		_objectImageFiles.push_back(imageFile);
	}
}

} // End of namespace Comprehend
