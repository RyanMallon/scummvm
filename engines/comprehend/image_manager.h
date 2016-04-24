#ifndef COMPREHEND_IMAGE_MANAGER_H
#define COMPREHEND_IMAGE_MANAGER_H

#include "common/array.h"

#include "comprehend/comprehend.h"
#include "comprehend/image_file.h"

namespace Comprehend {

class ImageManager {
public:
	Common::Array<ImageFile *> _roomImageFiles;
	Common::Array<ImageFile *> _objectImageFiles;

	ImageManager();
	~ImageManager();
	
	void init(Common::Array<const char *> roomFiles, Common::Array<const char *> objectFiles);
};

} // End of namespace Comprehend

#endif // COMPREHEND_IMAGE_MANAGER_H
