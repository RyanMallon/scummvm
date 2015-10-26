#ifndef COMPREHEND_IMAGE_FILE_H
#define COMPREHEND_IMAGE_FILE_H

#include "common/file.h"

#include "comprehend/comprehend.h"
#include "comprehend/image_manager.h"

namespace Comprehend {

class ImageFile {
public:
	static const int kImagesPerFile = 16;

	ImageFile(const char *filename);
	~ImageFile();

	const char *_filename;
	Common::File _file;
	uint16 _imageOffsets[kImagesPerFile];

	void drawImage(Common::File *file, off_t offset);
	void drawRoomImage(unsigned int index);
	void loadHeader(void);
};

} // End of namespace Comprehend

#endif // COMPREHEND_IMAGE_FILE_H
