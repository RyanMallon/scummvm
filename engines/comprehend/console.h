#ifndef COMPREHEND_CONSOLE_H
#define COMPREHEND_CONSOLE_H

#include "renderer.h"

namespace Comprehend {

class ComprehendEngine;

class Console {
public:	
	Console(Renderer *renderer);
	~Console();

	void scrollUp(bool pause);

	void clearChar();
	void drawChar(uint8 c);

	void nextChar(void);
	void drawString(const char *str, size_t len);
	void drawPrompt();
	void writeWrappedText(const char *text);	
	void updateScreen();
	bool handleKey(int key);
	char *getLine();

	void updateRect(Common::Rect rect);

	ComprehendEngine *_engine;
	Renderer *_renderer;
	Graphics::Surface _surfs[2];
	int _currentSurf;

	int _xOffset;
	char _inputBuffer[100];
	unsigned int _inputCount;

	int _promptBlinkState;
	uint32 _promptUpdateTime;
};

} // End of namespace Comprehend

#endif // COMPREHEND_CONSOLE_H

