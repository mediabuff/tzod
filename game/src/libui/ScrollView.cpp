#include "inc/ui/LayoutContext.h"
#include "inc/ui/ScrollView.h"
#include <algorithm>
using namespace UI;

ScrollView::ScrollView(LayoutManager &manager)
	: Window(manager)
{
	SetClipChildren(true);
}

void ScrollView::SetContent(std::shared_ptr<Window> content)
{
	if (_content != content)
	{
		if (_content)
		{
			UnlinkChild(*_content);
			_offset = vec2d{};
		}
		_content = content;
		AddBack(_content);
		SetFocus(_content);
	}
}

FRECT ScrollView::GetChildRect(vec2d size, float scale, const Window &child) const
{
	if (_content.get() == &child)
	{
		vec2d contentOffset = {
			std::max(0.f, std::min(_content->GetWidth() - size.x / scale, _offset.x)),
			std::max(0.f, std::min(_content->GetHeight() - size.y / scale, _offset.y)) };
		vec2d contentSize = vec2d{ 
			_horizontalScrollEnabled ? child.GetSize().x : size.x / scale,
			_verticalScrollEnabled ? child.GetSize().y : size.y / scale };
		return CanvasLayout(-contentOffset, contentSize, scale);
	}

	return Window::GetChildRect(size, scale, child);
}

void ScrollView::OnScroll(UI::InputContext &ic, UI::LayoutContext &lc, vec2d pointerPosition, vec2d scrollOffset)
{
	if (_content)
	{
		if (!_verticalScrollEnabled && _horizontalScrollEnabled && scrollOffset.x == 0)
		{
			scrollOffset.x = scrollOffset.y;
		}

		_offset -= scrollOffset * 30;
		_offset.x = std::max(0.f, std::min(_content->GetWidth() - lc.GetPixelSize().x / lc.GetScale(), _offset.x));
		_offset.y = std::max(0.f, std::min(_content->GetHeight() - lc.GetPixelSize().y / lc.GetScale(), _offset.y));
	}
	else
	{
		_offset = vec2d{};
	}
}
