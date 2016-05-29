#pragma once
#include "Pointers.h"
#include <math/MyMath.h>
#include <memory>
#include <stack>
#include <unordered_map>

namespace UI
{

enum class Key;
struct IInput;
struct IClipboard;
struct PointerSink;
class Window;

enum class Msg
{
	KEYUP,
	KEYDOWN,
	PointerDown,
	PointerMove,
	PointerUp,
	PointerCancel,
	MOUSEWHEEL,
	TAP,
};

class InputContext
{
public:
	InputContext(IInput &input, IClipboard &clipboard);

	bool ProcessPointer(
		std::shared_ptr<Window> wnd,
		vec2d size,
		vec2d pointerPosition,
		float z,
		Msg msg,
		int button,
		PointerType pointerType,
		unsigned int pointerID);
	bool ProcessKeys(std::shared_ptr<Window> wnd, Msg msg, UI::Key key);
	bool ProcessText(std::shared_ptr<Window> wnd, int c);

	IClipboard &GetClipboard() const { return _clipboard; }
	IInput& GetInput() const { return _input; }

	void PushTransform(vec2d offset);
	void PopTransform();

	vec2d GetMousePos() const;

	const std::vector<std::shared_ptr<Window>>* GetCapturePath(unsigned int pointerID) const;
	bool HasCapturedPointers(const Window* wnd) const;

	bool GetMainWindowActive() const { return _isAppActive; }

#ifndef NDEBUG
	const std::unordered_map<unsigned int, vec2d>& GetLastPointerLocation() const { return _lastPointerLocation; }
#endif

	void ResetWindow(Window &wnd);

private:
	bool ProcessKeyPressedRecursive(std::shared_ptr<Window> wnd, Key key);
	bool ProcessCharRecursive(std::shared_ptr<Window> wnd, int c);

	IInput &_input;
	IClipboard &_clipboard;

	std::stack<vec2d> _transformStack;

	struct PointerCapture
	{
		std::vector<std::shared_ptr<Window>> capturePath;
	};

	std::unordered_map<unsigned int, PointerCapture> _pointerCaptures;

	unsigned int _captureCountSystem;

	bool _isAppActive;
#ifndef NDEBUG
	std::unordered_map<unsigned int, vec2d> _lastPointerLocation;
#endif
};


struct PointerSinkSearch
{
	bool topMostPass;
	std::vector<std::shared_ptr<Window>> outSinkPath;
};

struct PointerSink;

PointerSink* FindPointerSink(
	PointerSinkSearch &search,
	std::shared_ptr<Window> wnd,
	vec2d size,
	vec2d pointerPosition,
	bool insideTopMost);

} // namespace UI