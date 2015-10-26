#ifndef COMPREHEND_RENDERER_H
#define COMPREHEND_RENDERER_H

#include "graphics/surface.h"
#include "common/file.h"

#include "comprehend/image_manager.h"

namespace Comprehend {

class Renderer {
public:
	Renderer(ImageManager *imageManager);
	~Renderer();

	int getPixel(int x, int y);
	void floodFill(int x, int y, int oldColor);

	void updateScreen();
	void drawRoomImage(uint16 index);
	void drawImage(Common::File *file, off_t offset);
	
	void drawChar(uint8 c, int x, int y, int color);
	void drawString(const char *string, int x, int y, int color);

	void doImageOpcode(Common::File *file, uint8 opcode);

	void copyRect(Graphics::Surface &srcSurf, Common::Rect srcRect, int x, int y);

	ImageManager *_imageManager;
	Graphics::Surface _surf;
	uint16 _penX;
	uint16 _penY;

	uint8 _penColor;
	uint8 _fillColor;
};

} // End of namespace Comprehend

#endif // COMPREHEND_RENDERER_H
