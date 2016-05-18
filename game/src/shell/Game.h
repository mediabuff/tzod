#pragma once
#include "InputManager.h"
#include <ctx/GameEvents.h>
#include <gv/GameViewHarness.h>
#include <ui/Window.h>
#include <ui/Text.h>
#include <unordered_map>

class ConfCache;
class LangCache;
class GameContext;
class World;
class WorldView;
class WorldController;
class DefaultCamera;

class MessageArea;
class ScoreTable;

class TimeElapsed : public UI::Text
{
public:
	TimeElapsed(UI::LayoutManager &manager, TextureManager &texman, float x, float y, enumAlignText align, World &world);

private:
	void OnTimeStep(float dt) override;

	World &_world;
};

namespace UI
{
	class ConsoleBuffer;
}

class GameLayout
	: public UI::Window
	, private GameListener
{
public:
	GameLayout(UI::LayoutManager &manager,
	           TextureManager &texman,
	           GameContext &gameContext,
	           WorldView &worldView,
	           WorldController &worldController,
	           const DefaultCamera &defaultCamera,
	           ConfCache &conf,
	           LangCache &lang,
	           UI::ConsoleBuffer &logger);
	virtual ~GameLayout();

	// Window
	void OnTimeStep(float dt) override;
	void Draw(vec2d size, DrawingContext &dc, TextureManager &texman) const override;
	void OnSize(float width, float height) override;
	bool OnFocus(bool focus) override { return true; }
	bool OnPointerDown(float x, float y, int button, UI::PointerType pointerType, unsigned int pointerID) override;
	bool OnPointerUp(float x, float y, int button, UI::PointerType pointerType, unsigned int pointerID) override;
	bool OnPointerMove(float x, float y, UI::PointerType pointerType, unsigned int pointerID) override;
	bool OnTap(float x, float y) override;

private:
	void OnChangeShowTime();
	vec2d GetDragDirection() const;
	unsigned int GetEffectiveDragCount() const;

	std::shared_ptr<MessageArea> _msg;
	std::shared_ptr<ScoreTable> _score;
	std::shared_ptr<TimeElapsed> _time;

	GameContext &_gameContext;
	GameViewHarness _gameViewHarness;
	WorldView &_worldView;
	WorldController &_worldController;
	const DefaultCamera &_defaultCamera;
	ConfCache &_conf;
	LangCache &_lang;
	InputManager _inputMgr;
	size_t _texDrag;
	size_t _texTarget;

	std::unordered_map<unsigned int, std::pair<vec2d, vec2d>> _activeDrags;

	// GameListener
	void OnMurder(GC_Player &victim, GC_Player *killer, MurderType murderType) override;
};
