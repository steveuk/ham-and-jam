//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#if !defined( GAMEMOVEMENT_H )
#define GAMEMOVEMENT_H
#ifdef _WIN32
#pragma once
#endif

#include "igamemovement.h"
#include "cmodel.h"
#include "hl2mp_player_shared.h"
#include "haj_weapon_base.h"
#include "tier0/vprof.h"

#define CTEXTURESMAX		512			// max number of textures loaded
#define CBTEXTURENAMEMAX	13			// only load first n chars of name

#define GAMEMOVEMENT_DUCK_TIME				1000.0f		// ms
#define GAMEMOVEMENT_JUMP_TIME				510.0f		// ms approx - based on the 21 unit height jump
#define GAMEMOVEMENT_JUMP_HEIGHT			21.0f		// units
#define GAMEMOVEMENT_TIME_TO_UNDUCK			( TIME_TO_UNDUCK * 1000.0f )		// ms
#define GAMEMOVEMENT_TIME_TO_UNDUCK_INV		( GAMEMOVEMENT_DUCK_TIME - GAMEMOVEMENT_TIME_TO_UNDUCK )

// *HAJ 020* - Jed
// NOTE: These should relate to the length of the player anims
// IMPORTANT - at present these values are fucked. Something in the math
// logic in gamemovement falls over if its more than 1 sec. It needs
// redoing. Using times more than a sec make it get stuck at the end of the
// prone transition

#define TIME_TO_PRONE						0.9999f
#define TIME_TO_UNPRONE						0.9999f
#define GAMEMOVEMENT_PRONE_TIME				1000.0f
#define GAMEMOVEMENT_TIME_TO_UNPRONE		( TIME_TO_UNPRONE * 1000.0f )		// ms
#define GAMEMOVEMENT_TIME_TO_UNPRONE_INV	( GAMEMOVEMENT_PRONE_TIME - GAMEMOVEMENT_TIME_TO_UNPRONE )

struct surfacedata_t;

class CBasePlayer;
class CHL2MP_Player;
class CHAJWeaponBase; // forward declaration

class CGameMovement : public IGameMovement
{
public:
	DECLARE_CLASS_NOBASE( CGameMovement );
	
	CGameMovement( void );
	virtual			~CGameMovement( void );

	virtual void	ProcessMovement( CBasePlayer *pPlayer, CMoveData *pMove );

	// *HAJ 020* - Jed
	virtual const Vector&	GetPlayerMins( bool ducked, bool proned ) const;
	virtual const Vector&	GetPlayerMaxs( bool ducked, bool proned ) const;
	virtual const Vector&	GetPlayerViewOffset( bool ducked, bool proned ) const;

protected:
	// Input/Output for this movement
	CMoveData		*mv;
	CBasePlayer		*player;
	
	int				m_nOldWaterLevel;
	int				m_nOnLadder;

	Vector			m_vecForward;
	Vector			m_vecRight;
	Vector			m_vecUp;


	// Does most of the player movement logic.
	// Returns with origin, angles, and velocity modified in place.
	// were contacted during the move.
	virtual void	PlayerMove(	void );

	// Set ground data, etc.
	void			FinishMove( void );

	// Debug helpers to track down prediction errors.
	void StartTrackPredictionErrors();
	void FinishTrackPredictionErrors();

	virtual float	CalcRoll( const QAngle &angles, const Vector &velocity, float rollangle, float rollspeed );

	virtual	void	DecayPunchAngle( void );

	void			CheckWaterJump(void );

	virtual void	WaterMove( void );

	void			WaterJump( void );

	// Handles both ground friction and water friction
	void			Friction( void );

	void			AirAccelerate( Vector& wishdir, float wishspeed, float accel );

	virtual void	AirMove( void );
	
	virtual bool	CanAccelerate();
	virtual void	Accelerate( Vector& wishdir, float wishspeed, float accel);

	// Only used by players.  Moves along the ground when player is a MOVETYPE_WALK.
	virtual void	WalkMove( void );

	// Try to keep a walking player on the ground when running down slopes etc
	void			StayOnGround( void );

	// Handle MOVETYPE_WALK.
	virtual void	FullWalkMove();

	// Implement this if you want to know when the player collides during OnPlayerMove
	virtual void	OnTryPlayerMoveCollision( trace_t &tr ) {}

	virtual const Vector&	GetPlayerMins( void ) const; // uses local player
	virtual const Vector&	GetPlayerMaxs( void ) const; // uses local player

	typedef enum
	{
		GROUND = 0,
		STUCK,
		LADDER
	} IntervalType_t;

	virtual int		GetCheckInterval( IntervalType_t type );

	// Useful for things that happen periodically. This lets things happen on the specified interval, but
	// spaces the events onto different frames for different players so they don't all hit their spikes
	// simultaneously.
	bool			CheckInterval( IntervalType_t type );


	// Decompoosed gravity
	void			StartGravity( void );
	void			FinishGravity( void );

	// Apply normal ( undecomposed ) gravity
	void			AddGravity( void );

	// Handle movement in noclip mode.
	void			FullNoClipMove( float factor, float maxacceleration );

	// Returns true if he started a jump (ie: should he play the jump animation)?
	virtual bool	CheckJumpButton( void );	// Overridden by each game.

	// Dead player flying through air., e.g.
	void			FullTossMove( void );
	
	// Player is a Observer chasing another player
	void			FullObserverMove( void );

	// Handle movement when in MOVETYPE_LADDER mode.
	virtual void	FullLadderMove();

	// The basic solid body movement clip that slides along multiple planes
	virtual int		TryPlayerMove( Vector *pFirstDest=NULL, trace_t *pFirstTrace=NULL );
	
	virtual bool	LadderMove( void );
	virtual bool	OnLadder( trace_t &trace );
	virtual float	LadderDistance( void ) const { return 2.0f; }	///< Returns the distance a player can be from a ladder and still attach to it
	virtual unsigned int LadderMask( void ) const { return MASK_PLAYERSOLID; }
	virtual float	ClimbSpeed( void ) const { return MAX_CLIMB_SPEED; }
	virtual float	LadderLateralMultiplier( void ) const { return 1.0f; }

	// See if the player has a bogus velocity value.
	void			CheckVelocity( void );

	// Does not change the entities velocity at all
	void			PushEntity( Vector& push, trace_t *pTrace );

	// Slide off of the impacting object
	// returns the blocked flags:
	// 0x01 == floor
	// 0x02 == step / wall
	int				ClipVelocity( Vector& in, Vector& normal, Vector& out, float overbounce );

	// If pmove.origin is in a solid position,
	// try nudging slightly on all axis to
	// allow for the cut precision of the net coordinates
	int				CheckStuck( void );
	
	// Check if the point is in water.
	// Sets refWaterLevel and refWaterType appropriately.
	// If in water, applies current to baseVelocity, and returns true.
	virtual bool			CheckWater( void );
	
	// Determine if player is in water, on ground, etc.
	virtual void CategorizePosition( void );

	virtual void	CheckParameters( void );

	virtual	void	ReduceTimers( void );

	virtual void	CheckFalling( void );

	void			PlayerWaterSounds( void );

	void ResetGetPointContentsCache();
	int GetPointContentsCached( const Vector &point );

	// Ducking
	virtual void	Duck( void );
	virtual void	HandleDuckingSpeedCrop();
	virtual void	FinishUnDuck( void );
	virtual void	FinishDuck( void );
	virtual bool	CanUnduck();
	void			UpdateDuckJumpEyeOffset( void );
	bool			CanUnDuckJump( trace_t &trace );
	void			StartUnDuckJump( void );
	void			FinishUnDuckJump( trace_t &trace );
	void			SetDuckedEyeOffset( float duckFraction );
	void			FixPlayerCrouchStuck( bool moveup );

	// *HAJ 020* - Jed	
	// Proning

	virtual void	Prone( void );
	
	virtual void	CheckProne( void );

	virtual void	DoProne( void );
	virtual void	DoUnProne( void );

	virtual void	HandleProningSpeedCrop();
	virtual void	FinishUnProne( void );
	virtual void	FinishProne( void );
	virtual bool	CanUnprone();
	void			SetPronedEyeOffset( float duckFraction );

	bool			CanVault();


	bool			m_bDoProne;
	bool			m_bDoUnProne;
	bool			m_bHandleProne;

	float			SplineFraction( float value, float scale );

	void			CategorizeGroundSurface( void );

	bool			InWater( void );

	// Commander view movement
	void			IsometricMove( void );

	// Traces the player bbox as it is swept from start to end
	void			TracePlayerBBox( const Vector& start, const Vector& end, unsigned int fMask, int collisionGroup, trace_t& pm );

	// Tests the player position
	CBaseHandle		TestPlayerPosition( const Vector& pos, int collisionGroup, trace_t& pm );

	// Checks to see if we should actually jump 
	void			PlaySwimSound();

	bool			IsDead( void ) const;

	// Figures out how the constraint should slow us down
	float			ComputeConstraintSpeedFactor( void );

	virtual void	SetGroundEntity( CBaseEntity *newGround );

	void			StepMove( Vector &vecDestination, trace_t &trace );

	#define BRUSH_ONLY true
	virtual unsigned int PlayerSolidMask( bool brushOnly = false );	///< returns the solid mask for the given player, so bots can have a more-restrictive set

private:
	// Performs the collision resolution for fliers.
	void			PerformFlyCollisionResolution( trace_t &pm, Vector &move );


protected:
	
	// Cache used to remove redundant calls to GetPointContents().
	int m_CachedGetPointContents;
	Vector m_CachedGetPointContentsPoint;	

	Vector			m_vecProximityMins;		// Used to be globals in sv_user.cpp.
	Vector			m_vecProximityMaxs;

	float			m_fFrameTime;
	float			m_flLastProneTime;

//private:
	bool			m_bSpeedCropped;

	float			m_flStuckCheckTime[MAX_PLAYERS+1][2]; // Last time we did a full test
};


//-----------------------------------------------------------------------------
// Traces player movement + position
//-----------------------------------------------------------------------------
inline void CGameMovement::TracePlayerBBox( const Vector& start, const Vector& end, unsigned int fMask, int collisionGroup, trace_t& pm )
{
	VPROF( "CGameMovement::TracePlayerBBox" );

	Ray_t ray;
	ray.Init( start, end, GetPlayerMins(), GetPlayerMaxs() );
	UTIL_TraceRay( ray, fMask, mv->m_nPlayerHandle.Get(), collisionGroup, &pm );
}

#endif // GAMEMOVEMENT_H
