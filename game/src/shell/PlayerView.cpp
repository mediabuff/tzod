#include "PlayerView.h"
#include "inc/shell/Config.h"
#include <ui/LayoutContext.h>
#include <video/TextureManager.h>
#include <video/DrawingContext.h>

PlayerView::PlayerView(UI::LayoutManager &manager, TextureManager &texman)
	: UI::Window(manager)
{
}

void PlayerView::SetPlayerConfig(ConfVarTable &playerConf, TextureManager &texman)
{
	_playerConfCache.reset(new ConfPlayerLocal(&playerConf));
	_texSkin = texman.FindSprite(std::string("skin/") + _playerConfCache->skin.Get());
}

void PlayerView::Draw(const UI::StateContext &sc, const UI::LayoutContext &lc, const UI::InputContext &ic, DrawingContext &dc, TextureManager &texman) const
{
	if (_playerConfCache)
	{
		vec2d pxSize = UI::ToPx(vec2d{ 64, 64 }, lc);
		dc.DrawSprite(MakeRectWH(pxSize), _texSkin, 0xffffffff, 0);
	}
}

