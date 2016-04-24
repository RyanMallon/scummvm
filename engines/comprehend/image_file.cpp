#include "comprehend/comprehend.h"
#include "comprehend/image_manager.h"
#include "comprehend/image_file.h"

#include "common/debug.h"

namespace Comprehend {

ImageFile::ImageFile(const char *filename) : _filename(filename) {

}

ImageFile::~ImageFile() {

}

void ImageFile::loadHeader(void) {
	uint16 firstWord;
	size_t i;

	if (!_file.open(_filename))
		error("Failed to open file: %s", _filename);

	// In earlier versions of Comprehend the first word is 0x1000 and
	// the image offsets start four bytes in. In newer versions the
	// image offsets start at the beginning of the image file.
	firstWord = _file.readUint16LE();
	if (firstWord == 0x1000)
		_file.seek(4, SEEK_SET);
	else
		_file.seek(0, SEEK_SET);

	// Table of image offsets
	for (i = 0; i < kImagesPerFile; i++) {
		_imageOffsets[i] = _file.readUint16LE();
		if (firstWord == 0x1000)
			_imageOffsets[i] += 4;		
	}
}

} // End of namespace Comprehend
