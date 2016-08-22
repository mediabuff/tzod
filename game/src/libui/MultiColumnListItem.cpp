#include "inc/ui/DataSource.h"
#include "inc/ui/ListBase.h"
#include "inc/ui/MultiColumnListItem.h"
#include "inc/ui/Rectangle.h"
#include "inc/ui/StateContext.h"
#include "inc/ui/Text.h"

using namespace UI;

namespace
{
	class DataContextBinding : public TextSource
	{
	public:
		explicit DataContextBinding(int column)
			: _column(column)
		{}

		// UI::TextSource
		const std::string& GetText(const StateContext &sc) const override
		{
			static std::string empty;
			auto listDataSource = reinterpret_cast<const ListDataSource*>(sc.GetDataContext());
			return _column < listDataSource->GetSubItemCount(sc.GetItemIndex()) ?
				listDataSource->GetItemText(sc.GetItemIndex(), _column) : empty;
		}

	private:
		int _column;
	};
}


MultiColumnListItem::MultiColumnListItem(LayoutManager &manager, TextureManager &texman)
	: Window(manager)
	, _selection(std::make_shared<Rectangle>(manager))
{
	static const auto selectionFillColorMap = std::make_shared<UI::ColorMap>(0x00000000, // default
		UI::ColorMap::ColorMapType{ { "Focused", 0xffffffff } });
	static const auto selectionBorderColorMap = std::make_shared<UI::ColorMap>(0x00000000, // default
		UI::ColorMap::ColorMapType{ { "Focused", 0xffffffff },{ "Unfocused", 0xffffffff } });

	AddBack(_selection);
	_selection->SetTexture(texman, "ui/listsel", false);
	_selection->SetBackColor(selectionFillColorMap);
	_selection->SetBorderColor(selectionBorderColorMap);

	EnsureColumn(manager, texman, 0u, 0.f);
}

void MultiColumnListItem::EnsureColumn(LayoutManager &manager, TextureManager &texman, unsigned int columnIndex, float offset)
{
	static const auto textColorMap = std::make_shared<UI::ColorMap>(0xffffffff, // default
		UI::ColorMap::ColorMapType{ { "Disabled", 0xbbbbbbbb },{ "Hover", 0xffccccff },{ "Focused", 0xff000000 } });

	if (columnIndex >= _columns.size())
		_columns.insert(_columns.end(), 1 + columnIndex - _columns.size(), nullptr);

	if (!_columns[columnIndex])
	{
		// TODO: reuse the text object
		_columns[columnIndex] = std::make_shared<UI::Text>(manager, texman);
		AddFront(_columns[columnIndex]);
	}

	_columns[columnIndex]->Move(offset, 0);
	_columns[columnIndex]->SetText(std::make_shared<DataContextBinding>(columnIndex));
	_columns[columnIndex]->SetFont(texman, "font_small");
	_columns[columnIndex]->SetFontColor(textColorMap);
}

vec2d MultiColumnListItem::GetContentSize(const StateContext &sc, TextureManager &texman) const
{
	return _columns[0]->GetContentSize(sc, texman);
}

FRECT MultiColumnListItem::GetChildRect(vec2d size, float scale, const Window &child) const
{
	if (_selection.get() == &child)
	{
		vec2d pxMargins = Vec2dFloor(vec2d{ 1, 2 } * scale);
		return MakeRectWH(-pxMargins, size + pxMargins * 2);
	}

	return Window::GetChildRect(size, scale, child);
}

