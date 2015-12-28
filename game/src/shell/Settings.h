#pragma once
#include "inc/shell/Config.h"
#include <ui/Dialog.h>
#include <ui/ListBase.h>
#include <string>
#include <vector>

class LangCache;

namespace UI
{
	class Button;
	class CheckBox;
	class ComboBox;
	class Edit;
	class List;
	class ScrollBarHorizontal;
	template<class, class> class ListAdapter;
	enum class Key;
}

class SettingsDlg : public UI::Dialog
{
	UI::ListDataSourceDefault _profilesDataSource;

	UI::ComboBox  *_player1;
	UI::ComboBox  *_player2;
	UI::List      *_profiles;
	UI::Button    *_editProfile;
	UI::Button    *_deleteProfile;

	UI::CheckBox  *_showFps;
	UI::CheckBox  *_showTime;
	UI::CheckBox  *_showNames;
	UI::CheckBox  *_askDisplaySettings;

	UI::ScrollBarHorizontal *_volumeSfx;
	int _initialVolumeSfx;

	UI::ScrollBarHorizontal *_volumeMusic;
	int _initialVolumeMusic;

	ConfCache &_conf;
	LangCache &_lang;

public:
	SettingsDlg(UI::Window *parent, ConfCache &conf, LangCache &lang);
	virtual ~SettingsDlg();

protected:
	void OnVolumeSfx(float pos);
	void OnVolumeMusic(float pos);

	void OnAddProfile();
	void OnEditProfile();
	void OnDeleteProfile();
	void OnSelectProfile(int index);

	void OnOK();
	void OnCancel();

	void UpdateProfilesList();
	void OnProfileEditorClosed(int code);
};

class ControlProfileDlg : public UI::Dialog
{
public:
	ControlProfileDlg(UI::Window *parent, const char *profileName, ConfCache &conf, LangCache &lang);
	~ControlProfileDlg();

	// UI::Window
	bool OnFocus(bool focus) override;
	bool OnKeyPressed(UI::Key key) override;
	void OnTimeStep(float dt) override;

private:
	void AddAction(ConfVarString &var, std::string actionDisplayName);

	void OnOK();
	void OnCancel();

	void OnSelectAction(int index);
	void SetActiveIndex(int index);

	typedef UI::ListAdapter<UI::ListDataSourceDefault, UI::List> DefaultListBox;
	DefaultListBox  *_actions;
	UI::Edit         *_nameEdit;
	UI::CheckBox     *_aimToMouseChkBox;
	UI::CheckBox     *_moveToMouseChkBox;
	UI::CheckBox     *_arcadeStyleChkBox;
	std::string      _nameOrig;
	std::vector<UI::Key> _keyBindings;
	ConfControllerProfile _profile;
	ConfCache &_conf;
	LangCache &_lang;

	float _time;
	int   _activeIndex;
	bool  _createNewProfile;
};