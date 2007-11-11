// Trigger.cpp

#include "stdafx.h"
#include "Trigger.h"
#include "Vehicle.h"

#include "level.h"
#include "Macros.h"


#include "fs/SaveFile.h"
#include "fs/MapFile.h"

IMPLEMENT_SELF_REGISTRATION(GC_Trigger)
{
	ED_ITEM( "trigger", "�������", 6 );
	return true;
}


GC_Trigger::GC_Trigger(float x, float y) : GC_2dSprite()
{
	SetTexture("editor_trigger");
	MoveTo(vec2d(x, y));
	SetZ(Z_WOOD);
	SetEvents(GC_FLAG_OBJECT_EVENTS_TS_FIXED);
	SetFlags(GC_FLAG_TRIGGER_ACTIVE);
	SetFlags(GC_FLAG_TRIGGER_ONLYVISIBLE);

	_radius = 1;
	_radiusDelta = 0;
}

GC_Trigger::GC_Trigger(FromFile) : GC_2dSprite(FromFile())
{
}

GC_Trigger::~GC_Trigger()
{
}

void GC_Trigger::Serialize(SaveFile &f)
{
	f.Serialize(_radius);
	f.Serialize(_radiusDelta);
	f.Serialize(_onEnter);
	f.Serialize(_onLeave);
	f.Serialize(_veh);
}

void GC_Trigger::mapExchange(MapFile &f)
{
	GC_2dSprite::mapExchange(f);

	int onlyVisible = CheckFlags(GC_FLAG_TRIGGER_ONLYVISIBLE);
	int active = CheckFlags(GC_FLAG_TRIGGER_ACTIVE);

	MAP_EXCHANGE_INT(active, active, 1);
	MAP_EXCHANGE_INT(only_visible, onlyVisible, 0);
	MAP_EXCHANGE_FLOAT(radius, _radius, 1);
	MAP_EXCHANGE_FLOAT(radius_delta, _radiusDelta, 0);
	MAP_EXCHANGE_STRING(on_enter, _onEnter, "");
	MAP_EXCHANGE_STRING(on_leave, _onLeave, "");

	if( f.loading() )
	{
		onlyVisible ? SetFlags(GC_FLAG_TRIGGER_ONLYVISIBLE) : ClearFlags(GC_FLAG_TRIGGER_ONLYVISIBLE);
		active ? SetFlags(GC_FLAG_TRIGGER_ACTIVE) : ClearFlags(GC_FLAG_TRIGGER_ACTIVE);
	}
}

void GC_Trigger::TimeStepFixed(float dt)
{
	if( !_veh && CheckFlags(GC_FLAG_TRIGGER_ACTIVE) )
	{
		// find nearest vehicle
		float rr_min = _radius * _radius * CELL_SIZE * CELL_SIZE;
		FOREACH( g_level->GetList(LIST_vehicles), GC_Vehicle, veh )
		{
			if( !veh->IsKilled() )
			{
				float rr = (GetPos() - veh->GetPos()).sqr();
				if( rr < rr_min )
				{
					if( CheckFlags(GC_FLAG_TRIGGER_ONLYVISIBLE) && rr > veh->_radius * veh->_radius )
					{
						GC_RigidBodyStatic *object = g_level->agTrace(
							g_level->grid_rigid_s, NULL, GetPos(), veh->GetPos() - GetPos());
						if( object != veh )
						{
							continue; // vehicle is invisible. skipping
						}
					}
					rr_min = rr;
					_veh = veh;
				}
			}
		}
		if( _veh )
		{
			script_exec(g_env.L, _onEnter.c_str());
		}
	}
	else if( _veh )
	{
		if( _veh->IsKilled() )
		{
			_veh = NULL;
			return;
		}

		float r = (_radius + _radiusDelta) * CELL_SIZE;
		if( (GetPos() - _veh->GetPos()).sqr() > r*r )
		{
			script_exec(g_env.L, _onLeave.c_str());
			_veh = NULL;
			return;
		}
	}
}

void GC_Trigger::Draw()
{
	if( g_level->_modeEditor )
		GC_2dSprite::Draw();
}

PropertySet* GC_Trigger::NewPropertySet()
{
	return new MyPropertySet(this);
}


GC_Trigger::MyPropertySet::MyPropertySet(GC_Object *object)
  : BASE(object)
  , _propActive(ObjectProperty::TYPE_INTEGER, "active")
  , _propRadius(ObjectProperty::TYPE_FLOAT, "radius")
  , _propRadiusDelta(ObjectProperty::TYPE_FLOAT, "radius_delta")
  , _propOnlyVisible(ObjectProperty::TYPE_INTEGER, "only_visible")
  , _propOnEnter(ObjectProperty::TYPE_STRING, "on_enter")
  , _propOnLeave(ObjectProperty::TYPE_STRING, "on_leave")
{
	_propActive.SetIntRange(0, 1);
	_propOnlyVisible.SetIntRange(0, 1);
	_propRadius.SetFloatRange(0, 100);
	_propRadiusDelta.SetFloatRange(0, 100);
}

int GC_Trigger::MyPropertySet::GetCount() const
{
	return BASE::GetCount() + 6;
}

ObjectProperty* GC_Trigger::MyPropertySet::GetProperty(int index)
{
	if( index < BASE::GetCount() )
		return BASE::GetProperty(index);

	switch( index - BASE::GetCount() )
	{
	case 0: return &_propActive;
	case 1: return &_propOnlyVisible;
	case 2: return &_propRadius;
	case 3: return &_propRadiusDelta;
	case 4: return &_propOnEnter;
	case 5: return &_propOnLeave;
	}

	_ASSERT(FALSE);
	return NULL;
}

void GC_Trigger::MyPropertySet::Exchange(bool applyToObject)
{
	BASE::Exchange(applyToObject);

	GC_Trigger *tmp = static_cast<GC_Trigger *>(GetObject());

	if( applyToObject )
	{
		_propOnlyVisible.GetIntValue() ? tmp->SetFlags(GC_FLAG_TRIGGER_ONLYVISIBLE) : tmp->ClearFlags(GC_FLAG_TRIGGER_ONLYVISIBLE);
		_propActive.GetIntValue() ? tmp->SetFlags(GC_FLAG_TRIGGER_ACTIVE) : tmp->ClearFlags(GC_FLAG_TRIGGER_ACTIVE);
		tmp->_radius = _propRadius.GetFloatValue();
		tmp->_radiusDelta = _propRadiusDelta.GetFloatValue();
		tmp->_onEnter = _propOnEnter.GetStringValue();
		tmp->_onLeave = _propOnLeave.GetStringValue();
	}
	else
	{
		_propOnlyVisible.SetIntValue(tmp->CheckFlags(GC_FLAG_TRIGGER_ONLYVISIBLE) ? 1 : 0);
		_propActive.SetIntValue(tmp->CheckFlags(GC_FLAG_TRIGGER_ACTIVE) ? 1 : 0);
		_propRadius.SetFloatValue(tmp->_radius);
		_propRadiusDelta.SetFloatValue(tmp->_radiusDelta);
		_propOnEnter.SetStringValue(tmp->_onEnter);
		_propOnLeave.SetStringValue(tmp->_onLeave);
	}
}


// end of file