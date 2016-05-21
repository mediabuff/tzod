#pragma once
#include <ui/Window.h>
#include <functional>

class LangCache;
class ConfCache;
class DefaultCamera;
class GC_Object;
class PropertyList;
class ServiceEditor;
class TextureManager;
class World;
class WorldView;
struct lua_State;

namespace UI
{
	template <class, class> class ListAdapter;
	class ListDataSourceDefault;
	class ComboBox;
	class ConsoleBuffer;
	class Text;
}

class EditorLayout : public UI::Window
{
	typedef UI::ListAdapter<UI::ListDataSourceDefault, UI::ComboBox> DefaultComboBox;

	ConfCache &_conf;
	LangCache &_lang;
	UI::ConsoleBuffer &_logger;
	const DefaultCamera &_defaultCamera;
	std::shared_ptr<PropertyList> _propList;
	std::shared_ptr<ServiceEditor> _serviceList;
	std::shared_ptr<UI::Text> _layerDisp;
	std::shared_ptr<DefaultComboBox> _typeList;
	std::shared_ptr<UI::Text> _help;
	size_t _fontSmall;

	size_t _texSelection;

	GC_Object *_selectedObject;
	bool _isObjectNew;
	bool _click;
	int  _mbutton;
	World &_world;
	WorldView &_worldView;
	lua_State *_globL;


	void OnKillSelected(World &world, GC_Object *sender, void *param);
	void OnMoveSelected(World &world, GC_Object *sender, void *param);

public:
	EditorLayout(UI::LayoutManager &manager,
		TextureManager &texman,
		World &world,
		WorldView &worldView,
		const DefaultCamera &defaultCamera,
		lua_State *globL,
		ConfCache &conf,
		LangCache &lang,
		UI::ConsoleBuffer &logger);
	virtual ~EditorLayout();

	void Select(GC_Object *object, bool bSelect);
	void SelectNone();

protected:
	void Draw(bool focused, bool enabled, vec2d size, DrawingContext &dc, TextureManager &texman) const override;

	bool OnMouseWheel(float x, float y, float z) override;
	bool OnPointerDown(UI::InputContext &ic, float x, float y, int button, UI::PointerType pointerType, unsigned int pointerID) override;
	bool OnPointerUp(UI::InputContext &ic, float x, float y, int button, UI::PointerType pointerType, unsigned int pointerID) override;
	bool OnPointerMove(UI::InputContext &ic, float x, float y, UI::PointerType pointerType, unsigned int pointerID) override;
	bool GetNeedsFocus() override;
	bool OnKeyPressed(UI::InputContext &ic, UI::Key key) override;
	void OnSize(float width, float height) override;

	void OnChangeObjectType(int index);
	void OnChangeUseLayers();
};
