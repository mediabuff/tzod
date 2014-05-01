// MousePointer.h

#pragma once

#include <Window.h>

class Level;

namespace UI
{
class Text;

///////////////////////////////////////////////////////////////////////////////

class MouseCursor : public Window
{
	Text *_text;
	float _timeShow;
	float _timeAnim;

public:
	MouseCursor(LayoutManager* manager, const char *texture);

protected:
	void OnTimeStep(Level &world, float dt);
};

///////////////////////////////////////////////////////////////////////////////
} // end of namespace UI

// end of file
