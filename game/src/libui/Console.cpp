// Console.cpp

#include "inc/ui/Console.h"
#include "inc/ui/Edit.h"
#include "inc/ui/Scroll.h"
#include "inc/ui/ConsoleBuffer.h"
#include "inc/ui/GuiManager.h"
#include "inc/ui/Keys.h"
#include <video/TextureManager.h>
#include <video/DrawingContext.h>
#include <algorithm>

using namespace UI;

ConsoleHistoryDefault::ConsoleHistoryDefault(size_t maxSize)
  : _maxSize(maxSize)
{
}

void ConsoleHistoryDefault::Enter(std::string str)
{
    _buf.push_back(std::move(str));
	if( _buf.size() > _maxSize )
	{
		_buf.pop_front();
	}
}

size_t ConsoleHistoryDefault::GetItemCount() const
{
	return _buf.size();
}

const std::string& ConsoleHistoryDefault::GetItem(size_t index) const
{
	return _buf[index];
}

///////////////////////////////////////////////////////////////////////////////

std::shared_ptr<Console> Console::Create(Window *parent, TextureManager &texman, float x, float y, float w, float h, ConsoleBuffer *buf)
{
	auto res = std::make_shared<Console>(parent->GetManager(), texman);
	res->Move(x, y);
	res->Resize(w, h);
	res->SetBuffer(buf);
	parent->AddFront(res);
	return res;
}

Console::Console(LayoutManager &manager, TextureManager &texman)
  : Window(manager)
  , _cmdIndex(0)
  , _font(texman.FindSprite("font_small"))
  , _buf(nullptr)
  , _history(nullptr)
  , _echo(true)
  , _autoScroll(true)
{
	_input = std::make_shared<Edit>(manager, texman);
	AddFront(_input);
	_scroll = std::make_shared<ScrollBarVertical>(manager, texman);
	_scroll->eventScroll = std::bind(&Console::OnScroll, this, std::placeholders::_1);
	AddFront(_scroll);
	SetTexture(texman, "ui/console", false);
	SetDrawBorder(true);
	SetTimeStep(true); // FIXME: workaround
}

float Console::GetInputHeight() const
{
	return _input->GetHeight();
}

void Console::SetColors(const SpriteColor *colors, size_t count)
{
	_colors.assign(colors, colors + count);
}

void Console::SetHistory(IConsoleHistory *history)
{
	_history = history;
	_cmdIndex = _history ? _history->GetItemCount() : 0;
}

void Console::SetBuffer(ConsoleBuffer *buf)
{
	_buf = buf;
	_scroll->SetDocumentSize(_buf ? (float) _buf->GetLineCount() + _scroll->GetPageSize() : 0);
}

void Console::SetEcho(bool echo)
{
	_echo = echo;
}

bool Console::OnChar(int c)
{
	GetManager().SetFocusWnd(_input);
	return true;
}

bool Console::OnKeyPressed(Key key)
{
	switch(key)
	{
	case Key::Up:
//		if( GetAsyncKeyState(VK_CONTROL) & 0x8000 ) // FIXME: workaround
//		{
//			_scroll->SetPos(_scroll->GetPos() - 1);
//			_autoScroll = _scroll->GetPos() + _scroll->GetPageSize() >= _scroll->GetDocumentSize();
//		}
//		else
        if( _history )
		{
			_cmdIndex = std::min(_cmdIndex, _history->GetItemCount());
			if( _cmdIndex > 0 )
			{
				--_cmdIndex;
				_input->SetText(_history->GetItem(_cmdIndex));
			}
		}
		break;
	case Key::Down:
//		if( GetAsyncKeyState(VK_CONTROL) & 0x8000 ) // FIXME: workaround
//		{
//			_scroll->SetPos(_scroll->GetPos() + 1);
//			_autoScroll = _scroll->GetPos() + _scroll->GetPageSize() >= _scroll->GetDocumentSize();
//		}
//		else
        if( _history )
		{
			++_cmdIndex;
			if( _cmdIndex < _history->GetItemCount() )
			{
				_input->SetText(_history->GetItem(_cmdIndex));
			}
			else
			{
				_input->SetText(std::string());
				_cmdIndex = _history->GetItemCount();
			}
		}
		break;
	case Key::Enter:
	{
		const std::string &cmd = _input->GetText();
		if( cmd.empty() )
		{
			_buf->WriteLine(0, std::string(">"));
		}
		else
		{
			if( _history )
			{
				if( _history->GetItemCount() == 0 || cmd != _history->GetItem(_history->GetItemCount() - 1) )
				{
					_history->Enter(cmd);
				}
				_cmdIndex = _history->GetItemCount();
			}

			if( _echo )
			{
				_buf->Format(0) << "> " << cmd;       // echo to the console
			}
			if( eventOnSendCommand )
				eventOnSendCommand(cmd.c_str());
			_input->SetText(std::string());
		}
		_scroll->SetPos(_scroll->GetDocumentSize());
		_autoScroll = true;
		break;
	}
	case Key::PageUp:
		_scroll->SetPos(_scroll->GetPos() - _scroll->GetPageSize());
		_autoScroll = _scroll->GetPos() + _scroll->GetPageSize() >= _scroll->GetDocumentSize();
		break;
	case Key::PageDown:
		_scroll->SetPos(_scroll->GetPos() + _scroll->GetPageSize());
		_autoScroll = _scroll->GetPos() + _scroll->GetPageSize() >= _scroll->GetDocumentSize();
		break;
//	case Key::Home:
//		break;
//	case Key::End:
//		break;
	case Key::Escape:
		if( _input->GetText().empty() )
			return false;
		_input->SetText(std::string());
		break;
	case Key::Tab:
		if( eventOnRequestCompleteCommand )
		{
			std::string result;
			int pos = _input->GetSelEnd();
			if( eventOnRequestCompleteCommand(_input->GetText(), pos, result) )
			{
				_input->SetText(result);
				_input->SetSel(pos, pos);
			}
			_scroll->SetPos(_scroll->GetDocumentSize());
			_autoScroll = true;
		}
		break;
	default:
		return false;
	}
	return true;
}

bool Console::OnMouseWheel(float x, float y, float z)
{
	_scroll->SetPos(_scroll->GetPos() - z * 3);
	_autoScroll = _scroll->GetPos() + _scroll->GetPageSize() >= _scroll->GetDocumentSize();
	return true;
}

bool Console::OnPointerDown(float x, float y, int button, PointerType pointerType, unsigned int pointerID)
{
	return true;
}

bool Console::OnPointerUp(float x, float y, int button, PointerType pointerType, unsigned int pointerID)
{
	return true;
}

bool Console::OnPointerMove(float x, float y, PointerType pointerType, unsigned int pointerID)
{
	return true;
}

void Console::OnTimeStep(float dt)
{
	// FIXME: workaround
	_scroll->SetDocumentSize(_buf ? (float) _buf->GetLineCount() + _scroll->GetPageSize() - 1 : 0);
	if( _autoScroll )
		_scroll->SetPos(_scroll->GetDocumentSize());
}

void Console::Draw(vec2d size, DrawingContext &dc, TextureManager &texman) const
{
	Window::Draw(size, dc, texman);

	if( _buf )
	{
		_buf->Lock();

		float h = texman.GetFrameHeight(_font, 0);
		size_t visibleLineCount = size_t(_input->GetY() / h);
		size_t scroll  = std::min(size_t(_scroll->GetDocumentSize() - _scroll->GetPos() - _scroll->GetPageSize()), _buf->GetLineCount());
		size_t lineMax = _buf->GetLineCount() - scroll;
		size_t count   = std::min(lineMax, visibleLineCount);

		float y = -fmod(_input->GetY(), h) + (float) (visibleLineCount - count) * h;

		for( size_t line = lineMax - count; line < lineMax; ++line )
		{
			unsigned int sev = _buf->GetSeverity(line);
			SpriteColor color = sev < _colors.size() ? _colors[sev] : 0xffffffff;
			dc.DrawBitmapText(4, y, _font, color, _buf->GetLine(line));
			y += h;
		}

		_buf->Unlock();

		if( _autoScroll )
		{
			// FIXME: magic number
			dc.DrawBitmapText(_scroll->GetX() - 2, _input->GetY(), _font, 0x7f7f7f7f, "auto", alignTextRB);
		}
	}
}

void Console::OnSize(float width, float height)
{
	_input->Move(0, height - _input->GetHeight());
	_input->Resize(width, _input->GetHeight());
	_scroll->Move(width - _scroll->GetWidth(), 0);
	_scroll->Resize(_scroll->GetWidth(), height - _input->GetHeight());
	_scroll->SetPageSize(_input->GetY() / GetManager().GetTextureManager().GetFrameHeight(_font, 0));
	_scroll->SetDocumentSize(_buf ? (float) _buf->GetLineCount() + _scroll->GetPageSize() : 0);
}

bool Console::OnFocus(bool focus)
{
	return true;
}

void Console::OnScroll(float pos)
{
	_autoScroll = pos + _scroll->GetPageSize() >= _scroll->GetDocumentSize();
}

