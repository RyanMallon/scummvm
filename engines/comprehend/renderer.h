#ifndef COMPREHEND_RENDERER_H
#define COMPREHEND_RENDERER_H

#include "graphics/surface.h"
#include "common/file.h"

#include "comprehend/image_manager.h"

namespace Comprehend {

class Renderer {
public:
	enum {
		kColorBlack	= 0x80,
		kColorWhite	= 0x00
	};

	Renderer(ImageManager *imageManager);
	~Renderer();

	int getPixel(int x, int y);
	void putPixel(int x, int y, int color);

	void drawShape(int x, int y, int shape, int color);
	void floodFill(int x, int y, int oldColor);

	void updateScreen();

	void drawImage(Common::File *file, off_t offset);
	void drawRoomImage(uint16 index);
	void drawDarkRoom();

	void drawObjectImage(uint16 index);

	void drawChar(uint8 c, int x, int y, int color);
	void drawString(const char *string, int x, int y, int color);

	// FIXME - static?
	void updateBox(unsigned x1, unsigned y1, unsigned x2, unsigned y2);

	void doImageOpcode(Common::File *file, uint8 opcode);

	void copyRect(const void *pixels, int pitch, Common::Rect rect);

	ImageManager *_imageManager;
	Graphics::Surface _surf;
	uint16 _penX;
	uint16 _penY;

	uint16 _textX;
	uint16 _textY;

	uint8 _penColor;
	uint8 _fillColor;
	int _currentShape;
};

} // End of namespace Comprehend

#endif // COMPREHEND_RENDERER_H
