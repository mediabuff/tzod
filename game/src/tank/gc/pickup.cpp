#include "Pickup.h"
#include "Explosion.h"
#include "indicators.h"
#include "Light.h"
#include "Macros.h"
#include "particles.h"
#include "Player.h"
#include "Sound.h"
#include "vehicle.h"
#include "Weapons.h"
#include "World.h"
#include "WorldEvents.h"

#include "MapFile.h"
#include "SaveFile.h"

#include "core/Debug.h"


IMPLEMENT_2LIST_MEMBER(GC_Pickup, LIST_pickups, LIST_timestep);
IMPLEMENT_GRID_MEMBER(GC_Pickup, grid_pickup);

GC_Pickup::GC_Pickup(World &world)
  : _label(new GC_HideLabel(world))
  , _radius(25.0)
  , _timeAttached(0)
  , _timeRespawn(0)
{
    _label->Register(world);
	SetAutoSwitch(true);
	SetRespawn(false);
	SetBlinking(false);
	SetVisible(true);
}

GC_Pickup::GC_Pickup(FromFile)
{
}

GC_Pickup::~GC_Pickup()
{
}

void GC_Pickup::Kill(World &world)
{
	if (GetCarrier())
		Detach(world);
	SAFE_KILL(world, _label);
	GC_Actor::Kill(world);
}

void GC_Pickup::Serialize(World &world, SaveFile &f)
{
	GC_Actor::Serialize(world, f);

	f.Serialize(_pickupCarrier);
	f.Serialize(_timeAttached);
	f.Serialize(_timeRespawn);
	f.Serialize(_radius);
	f.Serialize(_scriptOnPickup);
	f.Serialize(_label);
}

GC_Actor* GC_Pickup::FindNewOwner(World &world) const
{
	float r_sq = GetRadius() * GetRadius();

	FOREACH( world.GetList(LIST_vehicles), GC_Vehicle, veh )
	{
		if( (GetPos() - veh->GetPos()).sqr() < r_sq )
		{
			if( GetAutoSwitch() || veh->_state._bState_AllowDrop )
				return veh;
		}
	}

	return NULL;
}

void GC_Pickup::Attach(World &world, GC_Actor *actor)
{
	assert(!_pickupCarrier);
	_pickupCarrier = actor;
	_timeAttached  = 0;
	MoveTo(world, actor->GetPos());
	actor->Subscribe(NOTIFY_ACTOR_MOVE, this, (NOTIFYPROC) &GC_Pickup::OnOwnerMove);
	actor->Subscribe(NOTIFY_OBJECT_KILL, this, (NOTIFYPROC) &GC_Pickup::OnOwnerKill);
	actor->OnPickup(world, this, true);
}

void GC_Pickup::Detach(World &world)
{
	assert(_pickupCarrier);
	_pickupCarrier->Unsubscribe(NOTIFY_OBJECT_KILL, this, (NOTIFYPROC) &GC_Pickup::OnOwnerKill);
	_pickupCarrier->Unsubscribe(NOTIFY_ACTOR_MOVE, this, (NOTIFYPROC) &GC_Pickup::OnOwnerMove);
	_pickupCarrier->OnPickup(world, this, false);
	_pickupCarrier = NULL;
}

void GC_Pickup::Disappear(World &world)
{
	for( auto ls: world.eGC_Pickup._listeners )
		ls->OnDisappear(*this);
	
	if( GetCarrier() )
		Detach(world);
    SetVisible(false);
    _timeAttached = 0;
    if( _label )
        MoveTo(world, _label->GetPos());
}

void GC_Pickup::SetRespawnTime(float respawnTime)
{
	_timeRespawn = respawnTime;
}

float GC_Pickup::GetRespawnTime() const
{
	return _timeRespawn;
}

void GC_Pickup::SetBlinking(bool blink)
{
	SetFlags(GC_FLAG_PICKUP_BLINK, blink);
}

void GC_Pickup::MoveTo(World &world, const vec2d &pos)
{
    if (!CheckFlags(GC_FLAG_PICKUP_KNOWNPOS))
    {
        _label->MoveTo(world, pos);
        SetFlags(GC_FLAG_PICKUP_KNOWNPOS, true);
    }
    GC_Actor::MoveTo(world, pos);
}

void GC_Pickup::TimeStep(World &world, float dt)
{
	_timeAttached += dt;

	if( !GetCarrier() )
	{
		if( GetVisible() )
		{
			if( GC_Actor *actor = FindNewOwner(world) )
			{
				ObjPtr<GC_Object> watch(this);
				Attach(world, actor);
				// FIXME: pickup event will not fire if object is killed inside Attach function
				if (watch)
				{
					for( auto ls: world.eGC_Pickup._listeners )
						ls->OnPickup(*this, *actor);
				}
			}
		}
		else
		{
			if( _timeAttached > _timeRespawn )  // FIXME
			{
				SetRespawn(false);
				SetVisible(true);
				for( auto ls: world.eGC_Pickup._listeners )
					ls->OnRespawn(*this);
				
				for( int n = 0; n < 50; ++n )
				{
					vec2d a(PI2 * (float) n / 50);
					auto p = new GC_Particle(world, a * 25, PARTICLE_TYPE1, frand(0.5f) + 0.1f);
					p->Register(world);
					p->MoveTo(world, GetPos() + a * 25);
				}
			}
		}
	}

	GC_Actor::TimeStep(world, dt);
}

void GC_Pickup::MapExchange(World &world, MapFile &f)
{
	GC_Actor::MapExchange(world, f);
	MAP_EXCHANGE_FLOAT(respawn_time,  _timeRespawn, GetDefaultRespawnTime());
	MAP_EXCHANGE_STRING(on_pickup, _scriptOnPickup, "");
}

void GC_Pickup::OnOwnerMove(World &world, GC_Object *sender, void *param)
{
	assert(GetCarrier());
	assert(GetCarrier() == sender);
	MoveTo(world, GetCarrier()->GetPos());
}

void GC_Pickup::OnOwnerKill(World &world, GC_Object *sender, void *param)
{
	Disappear(world);
}

PropertySet* GC_Pickup::NewPropertySet()
{
	return new MyPropertySet(this);
}

GC_Pickup::MyPropertySet::MyPropertySet(GC_Object *object)
  : BASE(object)
  , _propTimeRespawn(ObjectProperty::TYPE_INTEGER, "respawn_time")
  , _propOnPickup(ObjectProperty::TYPE_STRING, "on_pickup")
{
	_propTimeRespawn.SetIntRange(0, 1000000);
}

int GC_Pickup::MyPropertySet::GetCount() const
{
	return BASE::GetCount() + 2;
}

ObjectProperty* GC_Pickup::MyPropertySet::GetProperty(int index)
{
	if( index < BASE::GetCount() )
		return BASE::GetProperty(index);

	switch( index - BASE::GetCount() )
	{
	case 0: return &_propTimeRespawn; break;
	case 1: return &_propOnPickup;    break;
	}

	assert(false);
	return NULL;
}

void GC_Pickup::MyPropertySet::MyExchange(World &world, bool applyToObject)
{
	BASE::MyExchange(world, applyToObject);

	GC_Pickup *obj = static_cast<GC_Pickup*>(GetObject());
	if( applyToObject )
	{
		obj->_timeRespawn = (float) _propTimeRespawn.GetIntValue() / 1000.0f;
		obj->_scriptOnPickup = _propOnPickup.GetStringValue();
	}
	else
	{
		_propTimeRespawn.SetIntValue(int(obj->_timeRespawn * 1000.0f + 0.5f));
		_propOnPickup.SetStringValue(obj->_scriptOnPickup);
	}
}

/////////////////////////////////////////////////////////////

IMPLEMENT_SELF_REGISTRATION(GC_pu_Health)
{
	ED_ITEM( "pu_health", "obj_health", 4 );
	return true;
}

GC_pu_Health::GC_pu_Health(World &world)
  : GC_Pickup(world)
{
	SetRespawnTime( GetDefaultRespawnTime() );
}

GC_pu_Health::GC_pu_Health(FromFile)
  : GC_Pickup(FromFile())
{
}

AIPRIORITY GC_pu_Health::GetPriority(World &world, const GC_Vehicle &veh) const
{
	if( veh.GetHealth() < veh.GetHealthMax() )
		return AIP_HEALTH * (veh.GetHealth() / veh.GetHealthMax());

	return AIP_NOTREQUIRED;
}

void GC_pu_Health::Attach(World &world, GC_Actor *actor)
{
	GC_Pickup::Attach(world, actor);

	static_cast<GC_RigidBodyStatic*>(actor)->SetHealth(
		static_cast<GC_RigidBodyStatic*>(actor)->GetHealthMax() );

	Disappear(world);
}

GC_Actor* GC_pu_Health::FindNewOwner(World &world) const
{
	GC_Vehicle *vehicle = static_cast<GC_Vehicle *>(GC_Pickup::FindNewOwner(world));

	if( vehicle && !vehicle->_state._bState_AllowDrop &&
		vehicle->GetHealth() >= vehicle->GetHealthMax() )
			return NULL;

	return vehicle;
}

/////////////////////////////////////////////////////////////

IMPLEMENT_SELF_REGISTRATION(GC_pu_Mine)
{
	ED_ITEM( "pu_mine", "obj_mine", 5 );
	return true;
}

GC_pu_Mine::GC_pu_Mine(World &world)
  : GC_Pickup(world)
{
	SetRespawnTime( GetDefaultRespawnTime() );
}

GC_pu_Mine::GC_pu_Mine(FromFile)
  : GC_Pickup(FromFile())
{
}

AIPRIORITY GC_pu_Mine::GetPriority(World &world, const GC_Vehicle &veh) const
{
	return AIP_NOTREQUIRED;
}

void GC_pu_Mine::Attach(World &world, GC_Actor *actor)
{
	MakeExplosionStandard(world, GetPos(), nullptr);
	Kill(world);
}

/////////////////////////////////////////////////////////////

IMPLEMENT_SELF_REGISTRATION(GC_pu_Shield)
{
	ED_ITEM( "pu_shield", "obj_shield", 4 );
	return true;
}

GC_pu_Shield::GC_pu_Shield(World &world)
  : GC_Pickup(world)
{
	SetRespawnTime( GetDefaultRespawnTime() );
	SetAutoSwitch(true);
	_timeHit = 0;
}

GC_pu_Shield::GC_pu_Shield(FromFile)
  : GC_Pickup(FromFile())
{
}

AIPRIORITY GC_pu_Shield::GetPriority(World &world, const GC_Vehicle &veh) const
{
	return AIP_SHIELD;
}

void GC_pu_Shield::Attach(World &world, GC_Actor *actor)
{
	if (auto vehicle = dynamic_cast<GC_Vehicle*>(actor))
	{
		if (vehicle->GetShield())
			vehicle->GetShield()->Disappear(world);
		vehicle->SetShield(this);
		GC_Pickup::Attach(world, actor);
	}
	else
	{
		Disappear(world);
	}
}

void GC_pu_Shield::Detach(World &world)
{
	assert(dynamic_cast<GC_Vehicle*>(GetCarrier()));
	static_cast<GC_Vehicle*>(GetCarrier())->SetShield(nullptr);
	SetBlinking(false);
	GC_Pickup::Detach(world);
}

void GC_pu_Shield::TimeStep(World &world, float dt)
{
	GC_Pickup::TimeStep(world, dt);

	if( GetCarrier() )
	{
		if( GetTimeAttached() + 2.0f > PROTECT_TIME )
		{
			if( !GetBlinking() )
			{
				PLAY(SND_InvEnd, GetPos());
				SetBlinking(true);
			}
			if( GetTimeAttached() > PROTECT_TIME )
			{
				Disappear(world);
			}
		}
	}
}

void GC_pu_Shield::OnOwnerDamage(World &world, DamageDesc &dd)
{
	assert(dynamic_cast<GC_RigidBodyDynamic*>(GetCarrier()));
	if( dd.damage > 5 || 0 == rand() % 4 || world.GetTime() - _timeHit > 0.2f )
	{
		const vec2d &pos = GetCarrier()->GetPos();

		PLAY(rand() % 2 ? SND_InvHit1 : SND_InvHit2, pos);
		vec2d dir = (dd.hit - pos).Normalize();
		vec2d p   = vec2d(dir.y, -dir.x);
		vec2d v   = static_cast<const GC_RigidBodyDynamic*>(GetCarrier())->_lv;
		for( int i = 0; i < 7; i++ )
		{
			auto p1 = new GC_Particle(world, v, PARTICLE_TYPE3, frand(0.4f)+0.1f);
            p1->Register(world);
            p1->MoveTo(world, pos + dir * 26.0f + p * (float) (i<<1));
			auto p2 = new GC_Particle(world, v, PARTICLE_TYPE3, frand(0.4f)+0.1f);
            p2->Register(world);
            p2->MoveTo(world, pos + dir * 26.0f - p * (float) (i<<1));
		}
	}
	dd.damage *= 0.1;
	_timeHit = world.GetTime();
}

void GC_pu_Shield::Serialize(World &world, SaveFile &f)
{
	GC_Pickup::Serialize(world, f);
	f.Serialize(_timeHit);
}

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_SELF_REGISTRATION(GC_pu_Shock)
{
	ED_ITEM( "pu_shock", "obj_shock", 4 );
	return true;
}

IMPLEMENT_1LIST_MEMBER(GC_pu_Shock, LIST_gsprites);

GC_pu_Shock::GC_pu_Shock(World &world)
  : GC_Pickup(world)
  , _targetPos(0, 0)
{
	SetRespawnTime(GetDefaultRespawnTime());
	SetAutoSwitch(false);
}

GC_pu_Shock::GC_pu_Shock(FromFile)
  : GC_Pickup(FromFile())
{
}

GC_pu_Shock::~GC_pu_Shock()
{
}

void GC_pu_Shock::Kill(World &world)
{
    SAFE_KILL(world, _light);
    GC_Pickup::Kill(world);
}

void GC_pu_Shock::Serialize(World &world, SaveFile &f)
{
	GC_Pickup::Serialize(world, f);
	f.Serialize(_light);
	f.Serialize(_targetPos);
}

AIPRIORITY GC_pu_Shock::GetPriority(World &world, const GC_Vehicle &veh) const
{
	GC_Vehicle *tmp = FindNearVehicle(world, &veh);
	if( !tmp ) return AIP_NOTREQUIRED;

	if( tmp->GetOwner()->GetTeam() == veh.GetOwner()->GetTeam() && 0 != tmp->GetOwner()->GetTeam() )
	{
		return AIP_NOTREQUIRED;
	}

	return AIP_SHOCK;
}

void GC_pu_Shock::Attach(World &world, GC_Actor* actor)
{
	assert(dynamic_cast<GC_RigidBodyStatic*>(actor));
	GC_Pickup::Attach(world, actor);
}

void GC_pu_Shock::Detach(World &world)
{
	GC_Pickup::Detach(world);
	SetGridSet(true);
	SAFE_KILL(world, _light);
}

GC_Vehicle* GC_pu_Shock::FindNearVehicle(World &world, const GC_RigidBodyStatic *ignore) const
{
	//
	// find the nearest enemy
	//

	float min_dist = AI_MAX_SIGHT * CELL_SIZE;
	float dist;

	GC_Vehicle *pNearTarget = NULL;
	FOREACH( world.GetList(LIST_vehicles), GC_Vehicle, pTargetObj )
	{
		if( pTargetObj != ignore )
		{
			// distance to the object
			dist = (GetPos() - pTargetObj->GetPos()).len();

			if( dist < min_dist )
			{
				GC_RigidBodyStatic *pObstacle = world.TraceNearest(world.grid_rigid_s,
					static_cast<GC_RigidBodyStatic*>(GetCarrier()),
					GetPos(), pTargetObj->GetPos() - GetPos());

				if( pObstacle == pTargetObj )
				{
					pNearTarget = pTargetObj;
					min_dist = dist;
				}
			}
		}
	}

	return pNearTarget;
}

void GC_pu_Shock::TimeStep(World &world, float dt)
{
	GC_Pickup::TimeStep(world, dt);

	if( GetCarrier() )
	{
		if( GetGridSet() )
		{
			if( GetTimeAttached() >= SHOCK_TIMEOUT )
			{
				GC_RigidBodyStatic *carrier = static_cast<GC_RigidBodyStatic *>(GetCarrier());
				if( GC_Vehicle *pNearTarget = FindNearVehicle(world, carrier) )
				{
					SetGridSet(false);

					_targetPos = pNearTarget->GetPos();

					_light = new GC_Light(world, GC_Light::LIGHT_DIRECT);
                    _light->Register(world);
					_light->MoveTo(world, GetPos());
					_light->SetRadius(100);

					vec2d tmp = _targetPos - GetPos();
					_light->SetLength(tmp.len());
					_light->SetLightDirection(tmp.Normalize());

					pNearTarget->TakeDamage(world, DamageDesc{1000, pNearTarget->GetPos(), carrier->GetOwner()});
				}
				else
				{
					carrier->TakeDamage(world, DamageDesc{1000, GetCarrier()->GetPos(), carrier->GetOwner()});
					Disappear(world);
				}
			}
		}
		else
		{
			float a = (GetTimeAttached() - SHOCK_TIMEOUT) * 5.0f;
			if( a > 1 )
			{
				Disappear(world);
			}
			else
			{
				_light->SetIntensity(1.0f - powf(a, 6));
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_SELF_REGISTRATION(GC_pu_Booster)
{
	ED_ITEM( "pu_booster", "obj_booster", 4 );
	return true;
}

GC_pu_Booster::GC_pu_Booster(World &world)
  : GC_Pickup(world)
{
	SetRespawnTime(GetDefaultRespawnTime());
}

GC_pu_Booster::GC_pu_Booster(FromFile)
  : GC_Pickup(FromFile())
{
}

GC_pu_Booster::~GC_pu_Booster()
{
}

void GC_pu_Booster::Serialize(World &world, SaveFile &f)
{
	GC_Pickup::Serialize(world, f);
	f.Serialize(_sound);
}

AIPRIORITY GC_pu_Booster::GetPriority(World &world, const GC_Vehicle &veh) const
{
	if( !veh.GetWeapon() )
	{
		return AIP_NOTREQUIRED;
	}

	return veh.GetWeapon()->GetBooster() ? AIP_BOOSTER_HAVE : AIP_BOOSTER;
}

void GC_pu_Booster::Attach(World &world, GC_Actor* actor)
{
	if( GC_Weapon *w = dynamic_cast<GC_Weapon*>(actor) )
	{
		GC_Pickup::Attach(world, w);
		
		if( w->GetBooster() )
			w->GetBooster()->Disappear(world);
		
		w->SetBooster(world, this);
		
		PLAY(SND_B_Start, GetPos());
		assert(NULL == _sound);
		_sound = new GC_Sound_link(world, SND_B_Loop, this);
		_sound->Register(world);
		_sound->SetMode(world, SMODE_LOOP);
	}
	else
	{
		// disappear if actor is not a weapon.
		Disappear(world);
	}
}

void GC_pu_Booster::Detach(World &world)
{
	assert(dynamic_cast<GC_Weapon*>(GetCarrier()));
	static_cast<GC_Weapon*>(GetCarrier())->SetBooster(world, nullptr);
	SAFE_KILL(world, _sound);
	GC_Pickup::Detach(world);
}

GC_Actor* GC_pu_Booster::FindNewOwner(World &world) const
{
	GC_Vehicle *veh = static_cast<GC_Vehicle *>(GC_Pickup::FindNewOwner(world));
	if( veh && !veh->_state._bState_AllowDrop && !veh->GetWeapon() )
		return NULL;
	return (veh && veh->GetWeapon()) ? veh->GetWeapon() : static_cast<GC_Actor *>(veh);
}

void GC_pu_Booster::TimeStep(World &world, float dt)
{
	GC_Pickup::TimeStep(world, dt);
	if( GetCarrier() )
	{
		if( GetTimeAttached() > BOOSTER_TIME )
		{
			Disappear(world);
		}
	}
}
