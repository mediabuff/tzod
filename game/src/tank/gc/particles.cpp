// particles.cpp

#include "particles.h"
#include "SaveFile.h"
#include "World.h"

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_SELF_REGISTRATION(GC_BrickFragment)
{
	return true;
}

IMPLEMENT_1LIST_MEMBER(GC_BrickFragment, LIST_timestep);

GC_BrickFragment::GC_BrickFragment(World &world, const vec2d &v0)
  : GC_2dSprite()
  , _startFrame(rand())
  , _time(0)
  , _timeLife(frand(0.5f) + 1.0f)
  , _velocity(v0)
{
	static TextureCache tex("particle_brick");

	SetTexture(tex);
    SetShadow(true);
}

GC_BrickFragment::GC_BrickFragment(FromFile)
  : GC_2dSprite(FromFile())
{
}

void GC_BrickFragment::Serialize(World &world, SaveFile &f)
{
	GC_2dSprite::Serialize(world, f);
	f.Serialize(_startFrame);
	f.Serialize(_time);
	f.Serialize(_timeLife);
	f.Serialize(_velocity);
}

void GC_BrickFragment::TimeStepFloat(World &world, float dt)
{
	_time += dt;

	if( _time >= _timeLife * 0.5f )
	{
		Kill(world);
		return;
	}

	SetFrame((_startFrame + int(_time * ANIMATION_FPS)) % (GetFrameCount() - 1));

	MoveTo(world, GetPos() + _velocity * dt);
	_velocity += vec2d(0, 300.0f) * dt;
}

/////////////////////////////////////////////////////////////

IMPLEMENT_SELF_REGISTRATION(GC_Particle)
{
	return true;
}

IMPLEMENT_1LIST_MEMBER(GC_Particle, LIST_timestep);

GC_Particle::GC_Particle(World &world, const vec2d &v, const TextureCache &texture,
                         float lifeTime, const vec2d &orient)
  : _time(0)
  , _timeLife(lifeTime)
  , _rotationSpeed(0)
  , _velocity(v)
{
	assert(_timeLife > 0);

	SetTexture(texture);
	SetDirection(orient);
}

GC_Particle::GC_Particle(World &world, const vec2d &v, ParticleType ptype,
                         float lifeTime, const vec2d &orient)
  : _time(0)
  , _timeLife(lifeTime)
  , _rotationSpeed(0)
  , _velocity(v)
  , _ptype(ptype)
{
	assert(_timeLife > 0);
	SetDirection(orient);
}

GC_Particle::GC_Particle(FromFile)
  : GC_2dSprite(FromFile())
{
}

void GC_Particle::Serialize(World &world, SaveFile &f)
{
	GC_2dSprite::Serialize(world, f);
	f.Serialize(_sizeOverride);
	f.Serialize(_time);
	f.Serialize(_timeLife);
	f.Serialize(_rotationSpeed);
	f.Serialize(_velocity);
	f.Serialize(_ptype);
}

void GC_Particle::TimeStepFloat(World &world, float dt)
{
	assert(_timeLife > 0);
	_time += dt;

	if( _time >= _timeLife )
	{
		Kill(world);
		return;
	}

	SetFrame( int((float)(GetFrameCount() - 1) * _time / _timeLife) );

	if( CheckFlags(GC_FLAG_PARTICLE_FADE) )
		SetOpacity(1.0f - _time / _timeLife);

	if( _rotationSpeed )
		SetDirection(vec2d(_rotationSpeed * _time));

	MoveTo(world, GetPos() + _velocity * dt);
}

void GC_Particle::SetFade(bool fade)
{
	SetFlags(GC_FLAG_PARTICLE_FADE, fade);
	if( fade )
		SetOpacity(1.0f - _time / _timeLife);
	else
		SetOpacity1i(255);
}

void GC_Particle::SetAutoRotate(float speed)
{
	_rotationSpeed = speed;
}

void GC_Particle::Draw(DrawingContext &dc, bool editorMode) const
{
	if( _sizeOverride >= 0 )
	{
		dc.DrawSprite(GetTexture(), GetCurrentFrame(), GetColor(),
					  GetPos().x, GetPos().y, _sizeOverride, _sizeOverride, GetDirection());
	}
	else
	{
		base::Draw(dc, editorMode);
	}
}


///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_SELF_REGISTRATION(GC_ParticleExplosion)
{
	return true;
}

IMPLEMENT_SELF_REGISTRATION(GC_ParticleDecal)
{
	return true;
}

IMPLEMENT_SELF_REGISTRATION(GC_ParticleGauss)
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// end of file
