#ifndef COMPREHEND_CONSOLE_H
#define COMPREHEND_CONSOLE_H

#include "renderer.h"

namespace Comprehend {

//class Renderer;

class Console {
public:	
	Console(Renderer *renderer);
	~Console();

	void scrollUp(bool pause);
	void drawChar(uint8 c);
	void drawSpace(void);
	void drawString(const char *str, size_t len);
	void drawPrompt();
	void writeWrappedText(const char *text);	
	void updateScreen();

	Renderer *_renderer;
	Graphics::Surface _surfs[2];
	int _currentSurf;
	int _xOffset;

};

} // End of namespace Comprehend

#endif // COMPREHEND_CONSOLE_H

