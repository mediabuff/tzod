#pragma once
#include "Window.h"
#include <deque>
#include <functional>
#include <string>
#include <vector>

class TextureManager;

namespace UI
{

class ConsoleBuffer;
class ScrollBarVertical;
class Edit;

struct IConsoleHistory
{
	virtual void Enter(std::string str) = 0;
	virtual size_t GetItemCount() const = 0;
	virtual const std::string& GetItem(size_t index) const = 0;
};

class ConsoleHistoryDefault : public IConsoleHistory
{
public:
	ConsoleHistoryDefault(size_t maxSize);

	void Enter(std::string str) override;
	size_t GetItemCount() const override;
	const std::string& GetItem(size_t index) const override;

private:
	size_t _maxSize;
	std::deque<std::string> _buf;
};

///////////////////////////////////////////////////////////////////////////////

class Console
	: public Window
	, private PointerSink
	, private KeyboardSink
{
public:
	Console(LayoutManager &manager, TextureManager &texman);
	static std::shared_ptr<Console> Create(Window *parent, TextureManager &texman, float x, float y, float w, float h, ConsoleBuffer *buf);

	float GetInputHeight() const;

	void SetColors(const SpriteColor *colors, size_t count);
	void SetHistory(IConsoleHistory *history);
	void SetBuffer(ConsoleBuffer *buf);
	void SetEcho(bool echo);
	std::function<void(const std::string &)> eventOnSendCommand;
	std::function<bool(const std::string &, int &, std::string &)> eventOnRequestCompleteCommand;

	// Window
	PointerSink* GetPointerSink() override { return this; }
	KeyboardSink *GetKeyboardSink() override { return this; }
	void OnTimeStep(LayoutManager &manager, float dt) override;
	void Draw(const LayoutContext &lc, InputContext &ic, DrawingContext &dc, TextureManager &texman) const override;
	void OnSize(float width, float height) override;

private:
	std::shared_ptr<ScrollBarVertical> _scroll;
	std::shared_ptr<Edit> _input;
	size_t _cmdIndex;
	size_t _font;

	ConsoleBuffer *_buf;
	IConsoleHistory *_history;
	std::vector<SpriteColor> _colors;

	bool _echo;
	bool _autoScroll;

	void OnScroll(float pos);

	// KeyboardSink
	bool OnKeyPressed(InputContext &ic, Key key) override;

	// PointerSink
	void OnMouseWheel(InputContext &ic, vec2d size, vec2d pointerPosition, float z) override;
};

} // namespace UI
