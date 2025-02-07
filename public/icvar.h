//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
//===========================================================================//

#ifndef ICVAR_H
#define ICVAR_H
#ifdef _WIN32
#pragma once
#endif

#include "appframework/IAppSystem.h"
#include "tier1/utlvector.h"
#include "tier1/characterset.h"
#include "tier0/memalloc.h"
#include "convar.h"
#include <cstdint>

// TO-DO: Remove Metamod ConVar PR is finished
class ConCommandBase;

// Shorthand helper to iterate registered convars
// Example usage:
// FOR_EACH_CONVAR( iter )
// {
//     ConVarRefAbstract aref( iter );
//     Msg( "%s = %d\n", aref.GetName(), aref.GetInt() );
// 
//     /* or use typed version, but make sure to check its validity after,
//        since it would be invalid on type mismatch */
// 
//     CConVarRef<int> cref( iter );
//     if(cref.IsValidRef())
//         Msg( "%s = %d\n", cref.GetName(), cref.Get() );
// }
#define FOR_EACH_CONVAR( iter ) for(ConVarRef iter = g_pCVar->FindFirstConVar(); iter.IsValidRef(); iter = g_pCVar->FindNextConVar( iter ))

// Shorthand helper to iterate registered concommands
#define FOR_EACH_CONCOMMAND( iter ) for(ConCommandRef iter = icvar->FindFirstConCommand(); iter.IsValidRef(); iter = icvar->FindNextConCommand( iter ))


struct ConVarSnapshot_t;
class KeyValues;

//-----------------------------------------------------------------------------
// Called when a ConVar changes value
//-----------------------------------------------------------------------------
typedef void(*FnChangeCallbackGlobal_t)(ConVarRefAbstract* ref, CSplitScreenSlot nSlot, const char *pNewValue, const char *pOldValue);

//-----------------------------------------------------------------------------
// ConVar & ConCommand creation listener callbacks
//-----------------------------------------------------------------------------
class ICVarListenerCallbacks
{
public:
	virtual void OnConVarCreated( ConVarRefAbstract *pNewCvar ) {};
	virtual void OnConCommandCreated( ConCommand *pNewCommand ) {};
};

//-----------------------------------------------------------------------------
// Purpose: DLL interface to ConVars/ConCommands
//-----------------------------------------------------------------------------
abstract_class ICvar : public IAppSystem
{
public:
	// allow_defensive - Allows finding convars with FCVAR_DEFENSIVE flag
	virtual ConVarRef		FindConVar( const char *name, bool allow_defensive = false ) = 0;
	virtual ConVarRef		FindFirstConVar() = 0;
	virtual ConVarRef		FindNextConVar( ConVarRef prev ) = 0;
	virtual void			CallChangeCallback( ConVarRef cvar, const CSplitScreenSlot nSlot, const CVValue_t* pNewValue, const CVValue_t* pOldValue ) = 0;

	// allow_defensive - Allows finding commands with FCVAR_DEFENSIVE flag
	virtual ConCommandRef	FindConCommand( const char *name, bool allow_defensive = false ) = 0;
	virtual ConCommandRef	FindFirstConCommand() = 0;
	virtual ConCommandRef	FindNextConCommand( ConCommandRef prev ) = 0;
	virtual void			DispatchConCommand( ConCommandRef cmd, const CCommandContext &ctx, const CCommand &args ) = 0;

	// Install a global change callback (to be called when any convar changes) 
	virtual void			InstallGlobalChangeCallback( FnChangeCallbackGlobal_t callback ) = 0;
	virtual void			RemoveGlobalChangeCallback( FnChangeCallbackGlobal_t callback ) = 0;
	virtual void			CallGlobalChangeCallbacks( ConVarRefAbstract* ref, CSplitScreenSlot nSlot, const char* newValue, const char* oldValue ) = 0;

	// Reverts cvars to default values which contain a specific flag,
	// cvars with a flag FCVAR_COMMANDLINE_ENFORCED would be skipped
	virtual void			ResetConVarsToDefaultValuesByFlag( uint64 nFlag ) = 0;

	virtual void			SetMaxSplitScreenSlots( int nSlots ) = 0;
	virtual int				GetMaxSplitScreenSlots() const = 0;

	virtual void			RegisterCreationListeners( ICVarListenerCallbacks *callbacks ) = 0;
	virtual void			RemoveCreationListeners( ICVarListenerCallbacks *callbacks ) = 0;

	virtual void			unk001() = 0;

	// Reverts cvars to default values which match pszPrefix string,
	// ignores FCVAR_COMMANDLINE_ENFORCED
	virtual void				ResetConVarsToDefaultValuesByName( const char *pszPrefix ) = 0;

	virtual ConVarSnapshot_t	*TakeConVarSnapshot( void ) = 0;
	virtual void				ResetConVarsToSnapshot( ConVarSnapshot_t *pSnapshot ) = 0;
	virtual void				DestroyConVarSnapshot( ConVarSnapshot_t *pSnapshot ) = 0;

	virtual characterset_t		*GetCharacterSet( void ) = 0;
	virtual void				SetConVarsFromGameInfo( KeyValues *pKV ) = 0;

	// Removes FCVAR_DEVELOPMENTONLY | FCVAR_DEFENSIVE from all cvars and concommands
	// that have FCVAR_DEFENSIVE set
	virtual void				StripDevelopmentFlags() = 0;

	// Register, unregister vars
	virtual void				RegisterConVar( const ConVarCreation_t& setup, uint64 nAdditionalFlags, ConVarRef* pCvarRef, ConVarData** pCvarData ) = 0;
	// Unregisters convar change callback, but leaves the convar in the lists,
	// so all ConVarRefs would still be valid as well as searching for it.
	// Expects ref to have registered index to be set (is set on convar creation)
	virtual void				UnregisterConVarCallbacks( ConVarRef cvar ) = 0;
	// Returns convar data or nullptr if not found
	virtual ConVarData*			GetConVarData( ConVarRef cvar ) = 0;

	// Register, unregister commands
	virtual ConCommand			RegisterConCommand( const ConCommandCreation_t& setup, uint64 nAdditionalFlags = 0 ) = 0;
	// Unregisters concommand callbacks, but leaves the command in the lists,
	// so all ConCommandRefs would still be valid as well as searching for it.
	// Expects ref to have registered index to be set (is set on command creation)
	virtual void				UnregisterConCommandCallbacks( ConCommandRef cmd ) = 0;
	// Returns command info or empty <unknown> command struct if not found, never nullptr
	virtual ConCommandData*		GetConCommandData( ConCommandRef cmd ) = 0;

	// Queues up value (creates a copy of it) to be set when convar is ready to be edited
	virtual void				QueueThreadSetValue( ConVarRefAbstract* ref, CSplitScreenSlot nSlot, CVValue_t* value ) = 0;
};

//-----------------------------------------------------------------------------
// These global names are defined by tier1.h, duplicated here so you
// don't have to include tier1.h
//-----------------------------------------------------------------------------

// These are marked DLL_EXPORT for Linux.
DECLARE_TIER1_INTERFACE( ICvar, cvar );
DECLARE_TIER1_INTERFACE( ICvar, g_pCVar );


#endif // ICVAR_H
