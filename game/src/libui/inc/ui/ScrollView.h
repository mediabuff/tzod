#pragma once
#include "Window.h"
#include <memory>

namespace UI
{
	class ScrollView
		: public Window
		, private ScrollSink
	{
	public:
		ScrollView(LayoutManager &manager);

		void SetContent(std::shared_ptr<Window> content);
		void SetVerticalScrollEnabled(bool enabled) { _verticalScrollEnabled = enabled; }
		void SetHorizontalScrollEnabled(bool enabled) { _horizontalScrollEnabled = enabled; }

		// Window
		ScrollSink* GetScrollSink() override { return this; }
		FRECT GetChildRect(const LayoutContext &lc, const Window &child) const override;

	private:
		std::shared_ptr<Window> _content;
		vec2d _offset = {};
		bool _verticalScrollEnabled = true;
		bool _horizontalScrollEnabled = false;

		// ScrollSink
		void OnScroll(UI::InputContext &ic, LayoutContext &lc, vec2d pointerPosition, vec2d offset) override;
	};

}// namespace UI
