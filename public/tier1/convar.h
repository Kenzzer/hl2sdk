//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $NoKeywords: $
//===========================================================================//

#ifndef CONVAR_H
#define CONVAR_H

#if _WIN32
#pragma once
#endif

#include "tier0/dbg.h"
#include "tier1/utlvector.h"
#include "tier1/utlstring.h"
#include "tier1/characterset.h"
#include "Color.h"
#include "icvar.h"
#include "playerslot.h"

#include <cstdint>
#include <cinttypes>

#ifdef _WIN32
#define FORCEINLINE_CVAR FORCEINLINE
#elif POSIX
#define FORCEINLINE_CVAR inline
#else
#error "implement me"
#endif


//-----------------------------------------------------------------------------
// Uncomment me to test for threading issues for material system convars
// NOTE: You want to disable all threading when you do this
// +host_thread_mode 0 +r_threaded_particles 0 +sv_parallel_packentities 0 +sv_disable_querycache 0
//-----------------------------------------------------------------------------
//#define CONVAR_TEST_MATERIAL_THREAD_CONVARS 1


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CCommand;
class ConCommand;

struct CVarCreationBase_t
{
	CVarCreationBase_t( ) :
	m_pszName( nullptr ),
	m_pszHelpString( nullptr ),
	m_nFlags( 0 )
	{}

	bool				IsFlagSet( int64_t flag ) const;
	void				AddFlags( int64_t flags );
	void				RemoveFlags( int64_t flags );
	int64_t				GetFlags() const;

	const char*				m_pszName;
	const char*				m_pszHelpString;
	int64_t					m_nFlags;
};

enum CommandTarget_t
{
	CT_NO_TARGET = -1,
	CT_FIRST_SPLITSCREEN_CLIENT = 0,
	CT_LAST_SPLITSCREEN_CLIENT = 3,
};

class CCommandContext
{
public:
	CCommandContext(CommandTarget_t nTarget, CPlayerSlot nSlot) :
		m_nTarget(nTarget), m_nPlayerSlot(nSlot)
	{
	}

	CommandTarget_t GetTarget() const
	{
		return m_nTarget;
	}

	CPlayerSlot GetPlayerSlot() const
	{
		return m_nPlayerSlot;
	}

private:
	CommandTarget_t m_nTarget;
	CPlayerSlot m_nPlayerSlot;
};

//-----------------------------------------------------------------------------
// ConVar flags
//-----------------------------------------------------------------------------
// The default, no flags at all
#define FCVAR_NONE				0 

// Command to ConVars and ConCommands
// ConVar Systems
#define FCVAR_LINKED_CONCOMMAND (1<<0)	// Allows concommand callback chaining. When command is dispatched all chained callbacks would fire.
#define FCVAR_DEVELOPMENTONLY	(1<<1)	// Hidden in released products. Flag is removed automatically if ALLOW_DEVELOPMENT_CVARS is defined.
#define FCVAR_GAMEDLL			(1<<2)	// defined by the game DLL
#define FCVAR_CLIENTDLL			(1<<3)  // defined by the client DLL
#define FCVAR_HIDDEN			(1<<4)	// Hidden. Doesn't appear in find or auto complete. Like DEVELOPMENTONLY, but can't be compiled out.

// ConVar only
#define FCVAR_PROTECTED			(1<<5)  // It's a server cvar, but we don't send the data since it's a password, etc.  Sends 1 if it's not bland/zero, 0 otherwise as value
#define FCVAR_SPONLY			(1<<6)  // This cvar cannot be changed by clients connected to a multiplayer server.
#define	FCVAR_ARCHIVE			(1<<7)	// set to cause it to be saved to vars.rc
#define	FCVAR_NOTIFY			(1<<8)	// notifies players when changed
#define	FCVAR_USERINFO			(1<<9)	// changes the client's info string

#define FCVAR_MISSING0	 		(1<<10) // Something that hides the cvar from the cvar lookups
#define FCVAR_UNLOGGED			(1<<11)  // If this is a FCVAR_SERVER, don't log changes to the log file / console if we are creating a log
#define FCVAR_MISSING1			(1<<12)  // Something that hides the cvar from the cvar lookups

// It's a ConVar that's shared between the client and the server.
// At signon, the values of all such ConVars are sent from the server to the client (skipped for local
//  client, of course )
// If a change is requested it must come from the console (i.e., no remote client changes)
// If a value is changed while a server is active, it's replicated to all connected clients
#define FCVAR_REPLICATED		(1<<13)	// server setting enforced on clients, TODO rename to FCAR_SERVER at some time
#define FCVAR_CHEAT				(1<<14) // Only useable in singleplayer / debug / multiplayer & sv_cheats
#define FCVAR_PER_USER			(1<<15) // causes varnameN where N == 2 through max splitscreen slots for mod to be autogenerated
#define FCVAR_DEMO				(1<<16) // record this cvar when starting a demo file
#define FCVAR_DONTRECORD		(1<<17) // don't record these command in demofiles
#define FCVAR_PERFORMING_SETVALUE	(1<<18)	// Cvar is in a SetValue state
#define FCVAR_RELEASE			(1<<19) // Cvars tagged with this are the only cvars avaliable to customers
#define FCVAR_MENUBAR_ITEM		(1<<20)
#define FCVAR_MISSING3			(1<<21)

#define FCVAR_NOT_CONNECTED		(1<<22)	// cvar cannot be changed by a client that is connected to a server
#define FCVAR_VCONSOLE_FUZZY_MATCHING (1<<23)

#define FCVAR_SERVER_CAN_EXECUTE	(1<<24) // the server is allowed to execute this command on clients via ClientCommand/NET_StringCmd/CBaseClientState::ProcessStringCmd.
#define FCVAR_CLIENT_CAN_EXECUTE	(1<<25) // Assigned to commands to let clients execute them
#define FCVAR_SERVER_CANNOT_QUERY	(1<<26) // If this is set, then the server is not allowed to query this cvar's value (via IServerPluginHelpers::StartQueryCvarValue).
#define FCVAR_VCONSOLE_SET_FOCUS	(1<<27)
#define FCVAR_CLIENTCMD_CAN_EXECUTE	(1<<28)	// IVEngineClient::ClientCmd is allowed to execute this command. 
											// Note: IVEngineClient::ClientCmd_Unrestricted can run any client command.

#define FCVAR_EXECUTE_PER_TICK		(1<<29)

#define FCVAR_DEFENSIVE				(1<<32)
//-----------------------------------------------------------------------------
// ConVar & ConCommand creation listener callbacks
//-----------------------------------------------------------------------------
class ICVarListenerCallbacks
{
public:
	virtual void OnConVarCreated( BaseConVar* pNewCvar ) = 0;
	virtual void OnConCommandCreated( ConCommand* pNewCommand ) = 0;
};
#define FCVAR_MISSING4				(1<<33)


//-----------------------------------------------------------------------------
// Called when a ConCommand needs to execute
//-----------------------------------------------------------------------------
typedef void ( *FnCommandCallback_t )( const CCommandContext &context, const CCommand &command );
typedef void ( *FnCommandCallbackNoContext_t )( const CCommand &command );
typedef void ( *FnCommandCallbackVoid_t )();

//-----------------------------------------------------------------------------
// Returns 0 to COMMAND_COMPLETION_MAXITEMS worth of completion strings
//-----------------------------------------------------------------------------
typedef void (*FnCommandCompletionCallback)( const CCommand &command, CUtlVector< CUtlString > &completions );


//-----------------------------------------------------------------------------
// Interface version
//-----------------------------------------------------------------------------
class ICommandCallback
{
public:
	virtual void CommandCallback(const CCommandContext &context, const CCommand &command) = 0;
};

class ICommandCompletionCallback
{
public:
	virtual int  CommandCompletionCallback( const CCommand &command, CUtlVector< CUtlString > &completions ) = 0;
};

//-----------------------------------------------------------------------------
// Command tokenizer
//-----------------------------------------------------------------------------
class CCommand
{
public:
	CCommand();
	CCommand( int nArgC, const char **ppArgV );
	bool Tokenize( CUtlString pCommand, characterset_t *pBreakSet = nullptr );
	void Reset();

	int ArgC() const;
	const char **ArgV() const;
	const char *ArgS() const;					// All args that occur after the 0th arg, in string form
	const char *GetCommandString() const;		// The entire command in string form, including the 0th arg
	const char *operator[]( int nIndex ) const;	// Gets at arguments
	const char *Arg( int nIndex ) const;		// Gets at arguments
	
	// Helper functions to parse arguments to commands.
	// 
	// Returns index of argument, or -1 if no such argument.
	int FindArg( const char *pName ) const;

	int FindArgInt( const char *pName, int nDefaultVal ) const;

	static int MaxCommandLength();
	static characterset_t* DefaultBreakSet();

private:
	void EnsureBuffers();

	enum
	{
		COMMAND_MAX_ARGC = 64,
		COMMAND_MAX_LENGTH = 512,
	};

    int m_nArgv0Size;
    CUtlVectorFixedGrowable<char, COMMAND_MAX_LENGTH> m_ArgSBuffer;
    CUtlVectorFixedGrowable<char, COMMAND_MAX_LENGTH> m_ArgvBuffer;
    CUtlVectorFixedGrowable<char*, COMMAND_MAX_ARGC> m_Args;
};

inline int CCommand::MaxCommandLength()
{
	return COMMAND_MAX_LENGTH - 1;
}

inline int CCommand::ArgC() const
{
	return m_Args.Count();
}

inline const char **CCommand::ArgV() const
{
	return ArgC() ? (const char**)m_Args.Base() : NULL;
}

inline const char *CCommand::ArgS() const
{
	return m_nArgv0Size ? (m_ArgSBuffer.Base() + m_nArgv0Size) : "";
}

inline const char *CCommand::GetCommandString() const
{
	return ArgC() ? m_ArgSBuffer.Base() : "";
}

inline const char *CCommand::Arg( int nIndex ) const
{
	// FIXME: Many command handlers appear to not be particularly careful
	// about checking for valid argc range. For now, we're going to
	// do the extra check and return an empty string if it's out of range
	if ( nIndex < 0 || nIndex >= ArgC() )
		return "";
	return m_Args[nIndex];
}

inline const char *CCommand::operator[]( int nIndex ) const
{
	return Arg( nIndex );
}

struct ConCommandCallbackInfo_t
{
	ConCommandCallbackInfo_t() :
		m_fnCommandCallback( nullptr ),
		m_bIsInterface( false ),
		m_bIsVoidCallback( false ),
		m_bIsContextLess( false )
	{}

	ConCommandCallbackInfo_t( FnCommandCallback_t cb ) :
		m_fnCommandCallback( cb ),
		m_bIsInterface( false ),
		m_bIsVoidCallback( false ),
		m_bIsContextLess( false )
	{}

	ConCommandCallbackInfo_t( FnCommandCallbackVoid_t cb ) :
		m_fnVoidCommandCallback( cb ),
		m_bIsInterface( false ),
		m_bIsVoidCallback( cb ? true : false ),
		m_bIsContextLess( false )
	{}

	ConCommandCallbackInfo_t( FnCommandCallbackNoContext_t cb ) :
		m_fnContextlessCommandCallback( cb ),
		m_bIsInterface( false ),
		m_bIsVoidCallback( false ),
		m_bIsContextLess( cb ? true : false )
	{}

	ConCommandCallbackInfo_t( ICommandCallback *cb ) :
		m_pCommandCallback( cb ),
		m_bIsInterface( cb ? true : false ),
		m_bIsVoidCallback( false ),
		m_bIsContextLess( false )
	{}

	bool IsValid() const { return m_fnCommandCallback != nullptr; }

	void Dispatch( const CCommandContext &context, const CCommand &command ) const
	{
		if(!IsValid())
			return;

		if(m_bIsInterface)
			m_pCommandCallback->CommandCallback( context, command );
		else if(m_bIsVoidCallback)
			m_fnVoidCommandCallback();
		else if(m_bIsContextLess)
			m_fnContextlessCommandCallback( command );
		else
			m_fnCommandCallback( context, command );
	}

	union
	{
		FnCommandCallback_t m_fnCommandCallback;
		FnCommandCallbackVoid_t m_fnVoidCommandCallback;
		FnCommandCallbackNoContext_t m_fnContextlessCommandCallback;
		ICommandCallback *m_pCommandCallback;
	};

	bool m_bIsInterface : 1;
	bool m_bIsVoidCallback : 1;
	bool m_bIsContextLess : 1;
};

struct ConCommandCompletionCallbackInfo_t
{
	ConCommandCompletionCallbackInfo_t() :
		m_fnCompletionCallback( nullptr ),
		m_bIsFunction( false ),
		m_bIsInterface( false )
	{}

	ConCommandCompletionCallbackInfo_t( FnCommandCompletionCallback cb ) :
		m_fnCompletionCallback( cb ),
		m_bIsFunction( cb ? true : false ),
		m_bIsInterface( false )
	{}

	ConCommandCompletionCallbackInfo_t( ICommandCompletionCallback *cb ) :
		m_pCommandCompletionCallback( cb ),
		m_bIsFunction( false ),
		m_bIsInterface( cb ? true : false )
	{}

	bool IsValid() const { return m_fnCompletionCallback != nullptr; }

	int Dispatch( const CCommand &command, CUtlVector< CUtlString > &completions ) const
	{
		if(!IsValid())
			return 0;

		if(m_bIsInterface)
			return m_pCommandCompletionCallback->CommandCompletionCallback( command, completions );
		
		m_fnCompletionCallback( command, completions );
		return completions.Count();
	}

	union
	{
		FnCommandCompletionCallback	m_fnCompletionCallback;
		ICommandCompletionCallback *m_pCommandCompletionCallback;
	};

	bool m_bIsFunction;
	bool m_bIsInterface;
};

struct ConCommandCreation_t : CVarCreationBase_t
{
	ConCommandCallbackInfo_t m_CBInfo;
	ConCommandCompletionCallbackInfo_t m_CompletionCBInfo;
};

//-----------------------------------------------------------------------------
// ConCommands internal data storage class
//-----------------------------------------------------------------------------
class ConCommandData : public CVarCreationBase_t
{
public:
	int GetAutoCompleteSuggestions( const CCommand &command, CUtlVector< CUtlString > &completions ) const
	{
		return m_CompletionCB.Dispatch( command, completions );
	}

	bool HasCompletionCallback() const { return m_CompletionCB.IsValid(); }
	bool HasCallback() const { return m_CallbackInfoIndex > 0; }

	ConCommandCompletionCallbackInfo_t m_CompletionCB;

	// Register index of concommand which completion cb comes from
	int m_ComplitionCBCmdIndex;
	// Index in a linkedlist of callbackinfos
	uint16 m_CallbackInfoIndex;
};

//-----------------------------------------------------------------------------
// Used to access registered concommands
//-----------------------------------------------------------------------------
class ConCommandRef
{
public:
	ConCommandRef() : m_CommandAccessIndex( -1 ) {}
	ConCommandRef( uint16_t command_idx ) : m_CommandAccessIndex( command_idx ) {}

	ConCommandRef( const char *command, bool allow_developer = false );

	ConCommandData *GetRawData();
	const ConCommandData *GetRawData() const { return const_cast<ConCommandRef *>(this)->GetRawData(); }

	const char *GetName() const { return GetRawData()->m_pszName; }
	const char *GetHelpString() const { return GetRawData()->m_pszHelpString; }

	bool IsFlagSet( int64_t flag ) const { return GetRawData()->GetFlags(); }
	void AddFlags( int64_t flags ) { GetRawData()->AddFlags( flags ); }
	void RemoveFlags( int64_t flags ) { GetRawData()->RemoveFlags( flags ); }
	int64_t GetFlags() const { return GetRawData()->GetFlags(); }

	bool HasCompletionCallback() const { return GetRawData()->HasCompletionCallback(); }
	bool HasCallback() const { return GetRawData()->HasCallback(); }

	uint16_t GetCallbackIndex() const { return GetRawData()->m_CallbackInfoIndex; }

	int GetAutoCompleteSuggestions( const CCommand &command, CUtlVector< CUtlString > &completions ) const
	{
		return GetRawData()->GetAutoCompleteSuggestions( command, completions );
	}

	void Invalidate() { m_CommandAccessIndex = -1; }
	bool IsValid() const { return m_CommandAccessIndex != -1; }
	uint16_t GetCommandAccessIndex() const { return m_CommandAccessIndex; }

private:
	// Index into internal linked list of concommands
	uint16_t m_CommandAccessIndex;
};

//-----------------------------------------------------------------------------
// Used to register concommands in the system as well as access its own data
//-----------------------------------------------------------------------------
class ConCommand : public ConCommandRef
{
public:
	ConCommand( uint16_t access_idx, int reg_idx ) : ConCommandRef( access_idx ), m_CommandRegisteredIndex( reg_idx ) {}

	ConCommand( const char *pName, ConCommandCallbackInfo_t callback,
		const char *pHelpString = 0, int64 flags = 0, ConCommandCompletionCallbackInfo_t completionFunc = ConCommandCompletionCallbackInfo_t() )
		: ConCommandRef(), m_CommandRegisteredIndex( 0 )
	{
		Create( pName, callback, pHelpString, flags, completionFunc );
	}

	~ConCommand()
	{
		Destroy();
	}

	int GetRegisteredIndex() const { return m_CommandRegisteredIndex; }

private:
	void Create( const char *pName, const ConCommandCallbackInfo_t &cb, const char *pHelpString, int64_t flags, const ConCommandCompletionCallbackInfo_t &completion_cb );
	void Destroy( );

	// Commands registered positional index
	int m_CommandRegisteredIndex;
};

using FnGenericChangeCallback_t = void(*)(BaseConVar* ref, CSplitScreenSlot nSlot, CVValue_t* pNewValue, CVValue_t* pOldValue);

struct ConVarCreation_t : CVarCreationBase_t {
	ConVarCreation_t() :
	m_pHandle(nullptr),
	m_pConVarData(nullptr)
	{}
	
	#pragma pack(push,1)
	struct ConVarValueInfo_t
	{
		ConVarValueInfo_t() :
			m_Version(0),
			m_bHasDefault(false),
			m_bHasMin(false),
			m_bHasMax(false)
		{}

		template<typename T>
		T& DefaultValue()	{ return *reinterpret_cast<T*>(m_defaultValue); }
		template<typename T>
		T& MinValue()		{ return *reinterpret_cast<T*>(m_minValue); }
		template<typename T>
		T& MaxValue()		{ return *reinterpret_cast<T*>(m_maxValue); }

		int32_t m_Version; // 0x18

		bool m_bHasDefault; // 0x22
		bool m_bHasMin; // 0x23
		bool m_bHasMax; // 0x24
	private:
		// Don't use CVValue_t directly, to avoid initialising memory
		uint8_t m_defaultValue[sizeof(CVValue_t)]; // 0x25
		uint8_t m_minValue[sizeof(CVValue_t)]; // 0x35
		uint8_t m_maxValue[sizeof(CVValue_t)]; // 0x45
	} m_valueInfo; // 0x22
	#pragma pack(pop)

	FnGenericChangeCallback_t m_fnCallBack; // 0x56
	EConVarType m_eVarType; // 0x58

	ConVarHandle* m_pHandle; // 0x60
	CConVarBaseData** m_pConVarData; // 0x68
};
static_assert(sizeof(ConVarCreation_t) == 0x70, "ConVarCreation_t wrong size!");
static_assert(sizeof(ConVarCreation_t) % 8 == 0x0, "ConVarCreation_t isn't 8 bytes aligned!");
static_assert(sizeof(CVValue_t) == 0x10, "CVValue_t wrong size!");

static CConVarBaseData* GetInvalidConVar( EConVarType type )
{
	static CConVarBaseData* invalid_convar[EConVarType_MAX + 1] = {
		new CConVarData<bool>(),
		new CConVarData<int16_t>(),
		new CConVarData<uint16_t>(),
		new CConVarData<int32_t>(),
		new CConVarData<uint32_t>(),
		new CConVarData<int64_t>(),
		new CConVarData<uint64_t>(),
		new CConVarData<float>(),
		new CConVarData<double>(),
		new CConVarData<const char*>(),
		new CConVarData<Color>(),
		new CConVarData<Vector2D>(),
		new CConVarData<Vector>(),
		new CConVarData<Vector4D>(),
		new CConVarData<QAngle>(),
		new CConVarData<void*>() // EConVarType_MAX
	};

	if (type == EConVarType_Invalid)
	{
		return invalid_convar[ EConVarType_MAX ];
	}
	return invalid_convar[ type ];
}

void SetupConVar( ConVarCreation_t& cvar );
void UnRegisterConVar( ConVarHandle& cvar );
void RegisterConVar( ConVarCreation_t& cvar );

//-----------------------------------------------------------------
// Used to read/write/create convars (replaces the FindVar method)
//-----------------------------------------------------------------
class BaseConVar
{
public:
	inline const char*	GetName( ) const			{ return m_ConVarData->GetName( ); }
	inline const char*	GetDescription( ) const		{ return m_ConVarData->GetDescription( ); }
	inline EConVarType	GetType( ) const			{ return m_ConVarData->GetType( ); }

	inline bool HasDefaultValue( ) const	{ return m_ConVarData->HasDefaultValue( ); }
	inline bool HasMinValue( ) const		{ return m_ConVarData->HasMinValue( ); }
	inline bool HasMaxValue( ) const		{ return m_ConVarData->HasMaxValue( ); }

	inline bool		IsFlagSet( int64_t flag ) const		{ return m_ConVarData->IsFlagSet( flag ); }
	inline void		AddFlags( int64_t flags )			{ m_ConVarData->AddFlags( flags ); }
	inline void		RemoveFlags( int64_t flags )		{ return m_ConVarData->RemoveFlags( flags ); }
	inline int64_t	GetFlags( void ) const				{ return m_ConVarData->GetFlags( ); }

	inline ConVarHandle GetHandle() const { return m_Handle; };
	inline CConVarBaseData* GetConVarData() const { return m_ConVarData; };
protected:
	// High-speed method to read convar data
	ConVarHandle m_Handle;
	CConVarBaseData* m_ConVarData;
};

template<typename T>
class ConVar : public BaseConVar
{
public:
	using FnChangeCallback_t = void(*)(ConVar<T>* ref, const CSplitScreenSlot nSlot, const T* pNewValue, const T* pOldValue);

	ConVar(const char* name, int32_t flags, const char* description, const T& value, FnChangeCallback_t cb = nullptr)
	{
		this->Init(INVALID_CONVAR_HANDLE, TranslateConVarType<T>());

		ConVarCreation_t setup;
		setup.m_valueInfo.m_bHasDefault = true;
		setup.m_valueInfo.DefaultValue<T>() = value;
		setup.m_eVarType = TranslateConVarType<T>();
		setup.m_fnCallBack = reinterpret_cast<FnGenericChangeCallback_t>(cb);

		this->Register(name, flags &~ FCVAR_DEVELOPMENTONLY, description, setup);
	}

	ConVar(const char* name, int32_t flags, const char* description, const T& value, bool min, const T& minValue, bool max, const T& maxValue, FnChangeCallback_t cb = nullptr)
	{
		this->Init(INVALID_CONVAR_HANDLE, TranslateConVarType<T>());

		ConVarCreation_t setup;
		setup.m_valueInfo.m_bHasDefault = true;
		setup.m_valueInfo.DefaultValue<T>() = value;

		setup.m_valueInfo.m_bHasMin = min;
		setup.m_valueInfo.m_bHasMax = max;
		setup.m_valueInfo.MinValue<T>() = minValue;
		setup.m_valueInfo.MaxValue<T>() = maxValue;

		setup.m_eVarType = TranslateConVarType<T>();
		setup.m_fnCallBack = reinterpret_cast<FnGenericChangeCallback_t>(cb);

		this->Register(name, flags &~ FCVAR_DEVELOPMENTONLY, description, setup);
	}

	~ConVar()
	{
		UnRegisterConVar(this->m_Handle);
	}

	inline const CConVarData<T>* GetConVarData() const { return reinterpret_cast<const CConVarData<T>*>(m_ConVarData); }
	inline CConVarData<T>* GetConVarData() { return reinterpret_cast<CConVarData<T>*>(m_ConVarData); }

	inline const T&	GetDefaultValue( ) const	{ return GetConVarData()->GetDefaultValue( ); }
	inline const T&	GetMinValue( ) const		{ return GetConVarData()->GetMinValue( ); }
	inline const T&	GetMaxValue( ) const		{ return GetConVarData()->GetMaxValue( ); }

	inline void SetDefaultValue( const T& value )	{ GetConVarData()->SetDefaultValue( value ); }
	inline void SetMinValue( const T& value )		{ GetConVarData()->SetMinValue( value ); }
	inline void SetMaxValue( const T& value )		{ GetConVarData()->SetMaxValue( value ); }

	inline void RemoveDefaultValue( )	{ GetConVarData()->RemoveDefaultValue( ); }
	inline void RemoveMinValue( )		{ GetConVarData()->RemoveMinValue( ); }
	inline void RemoveMaxValue( )		{ GetConVarData()->RemoveMaxValue( ); }

	inline const T& Clamp(const T& value) const { return GetConVarData()->Clamp( value ); }
	
	inline const T&	GetValue( const CSplitScreenSlot& index = CSplitScreenSlot() ) const { return GetConVarData()->GetValue( index ); }
	inline void	SetValue( const T& val, const CSplitScreenSlot& index = CSplitScreenSlot() )
	{
		auto newValue = this->Clamp( val );

		char szNewValue[256], szOldValue[256];
		CConVarData<T>::ValueToString( newValue, szNewValue, sizeof(szNewValue) );
		GetConVarData()->GetStringValue( szOldValue, sizeof(szOldValue), index );

		// Deep copy
		T oldValue = this->GetValue( );
		this->UpdateValue( newValue, index, (const CVValue_t*)&newValue, (const CVValue_t*)&oldValue, szNewValue, szOldValue );
	}

	inline void GetStringValue( char* dst, size_t len, const CSplitScreenSlot& index = 0 ) const { GetConVarData()->GetStringValue( dst, len, index ); }

	inline void GetStringDefaultValue( char* dst, size_t len ) const	{ GetConVarData()->GetStringDefaultValue( dst, len ); }
	inline void GetStringMinValue( char* dst, size_t len ) const		{ GetConVarData()->GetStringMinValue( dst, len ); }
	inline void GetStringMaxValue( char* dst, size_t len ) const		{ GetConVarData()->GetStringMaxValue( dst, len ); }
private:
	void Init(ConVarHandle defaultHandle, EConVarType type)
	{
		this->m_Handle.Invalidate( );
		this->m_ConVarData = nullptr;

		if ( g_pCVar )
		{
			auto cvar = g_pCVar->GetConVar( defaultHandle );
			this->m_ConVarData = ( cvar && cvar->Cast<T>( ) ) ? cvar : nullptr;
			if ( !this->m_ConVarData )
			{
				this->m_ConVarData = GetInvalidConVar( TranslateConVarType<T>( ) );
			}
			// technically this
			//result = *(char ***)(sub_10B7760((unsigned int)a3) + 80);
		}
		this->m_Handle = defaultHandle;
	}

	void Register(const char* name, int32_t flags, const char* description, ConVarCreation_t& cvar)
	{
		this->m_ConVarData = GetInvalidConVar( cvar.m_eVarType );
		this->m_Handle.Invalidate();

		if (!CommandLine()->HasParm("-tools")
		&& (flags & (FCVAR_DEVELOPMENTONLY
		|FCVAR_ARCHIVE
		|FCVAR_USERINFO
		|FCVAR_CHEAT
		|FCVAR_RELEASE
		|FCVAR_SERVER_CAN_EXECUTE
		|FCVAR_CLIENT_CAN_EXECUTE
		|FCVAR_CLIENTCMD_CAN_EXECUTE)) == 0)
		{
			flags |= FCVAR_DEVELOPMENTONLY;
		}

		cvar.m_pszName = name;
		cvar.m_pszHelpString = description;
		cvar.m_nFlags = flags;

		cvar.m_pHandle = &this->m_Handle;
		cvar.m_pConVarData = &this->m_ConVarData;

		SetupConVar(cvar);
	}

	inline void UpdateValue( const T& value, const CSplitScreenSlot& index, const CVValue_t* newValue, const CVValue_t* oldValue, const char* szNewValue, const char* szOldValue )
	{
		GetConVarData()->SetValue( value, index );

		GetConVarData()->SetTimesChanged( GetConVarData()->GetTimesChanged( ) + 1 );
		g_pCVar->CallChangeCallback( this->m_Handle, index, newValue, oldValue );
		g_pCVar->CallGlobalChangeCallbacks( this, index, szNewValue, szOldValue );
	}
};
static_assert(sizeof(ConVar<int>) == 0x10, "ConVar is of the wrong size!");
static_assert(sizeof(ConVar<int>) == sizeof(ConVar<Vector>), "Templated ConVar size varies!");

// Special case for string
template<> inline void ConVar<const char*>::SetValue( const char*const& val, const CSplitScreenSlot& index )
{
	auto newValue = this->Clamp( val );

	char szNewValue[256], szOldValue[256];
	CConVarData<const char*>::ValueToString( newValue, szNewValue, sizeof(szNewValue) );
	GetConVarData()->GetStringValue( szOldValue, sizeof(szOldValue), index );

	this->UpdateValue( newValue, index, (const CVValue_t*)&newValue, (const CVValue_t*)&szOldValue, szNewValue, szOldValue );
}

//-----------------------------------------------------------------------------

#ifdef CONVAR_WORK_FINISHED
class CSplitScreenAddedConVar : public ConVar
{
	typedef ConVar BaseClass;
public:
	CSplitScreenAddedConVar( int nSplitScreenSlot, const char *pName, const ConVar *pBaseVar ) :
		BaseClass
		( 
			pName, 
			pBaseVar->GetDefault(), 
			// Keep basevar flags, except remove _SS and add _SS_ADDED instead
			( pBaseVar->GetFlags() & ~FCVAR_SS ) | FCVAR_SS_ADDED, 
			pBaseVar->GetHelpText(), 
			pBaseVar->HasMin(),
			pBaseVar->GetMinValue(),
			pBaseVar->HasMax(),
			pBaseVar->GetMaxValue()
		),
		m_pBaseVar( pBaseVar ),
		m_nSplitScreenSlot( nSplitScreenSlot )
	{
		for ( int i = 0; i < pBaseVar->GetChangeCallbackCount(); ++i )
		{
			InstallChangeCallback( pBaseVar->GetChangeCallback( i ), false );
		}
		Assert( nSplitScreenSlot >= 1 );
		Assert( nSplitScreenSlot < MAX_SPLITSCREEN_CLIENTS );
		Assert( m_pBaseVar );
		Assert( IsFlagSet( FCVAR_SS_ADDED ) );
		Assert( !IsFlagSet( FCVAR_SS ) );
	}

	const ConVar *GetBaseVar() const;
	virtual const char *GetBaseName() const;
	void SetSplitScreenPlayerSlot( int nSlot );
	virtual int GetSplitScreenPlayerSlot() const;

protected:

	const ConVar	*m_pBaseVar;
	int		m_nSplitScreenSlot;
};

FORCEINLINE_CVAR const ConVar *CSplitScreenAddedConVar::GetBaseVar() const 
{ 
	Assert( m_pBaseVar );
	return m_pBaseVar; 
}

FORCEINLINE_CVAR const char *CSplitScreenAddedConVar::GetBaseName() const 
{ 
	Assert( m_pBaseVar );
	return m_pBaseVar->GetName(); 
}

FORCEINLINE_CVAR void CSplitScreenAddedConVar::SetSplitScreenPlayerSlot( int nSlot ) 
{ 
	m_nSplitScreenSlot = nSlot; 
}

FORCEINLINE_CVAR int CSplitScreenAddedConVar::GetSplitScreenPlayerSlot() const 
{ 
	return m_nSplitScreenSlot; 
}

#if 0
//-----------------------------------------------------------------------------
// Helper for referencing splitscreen convars (i.e., "name" and "name2")
//-----------------------------------------------------------------------------
class SplitScreenConVarRef
{
public:
	SplitScreenConVarRef( const char *pName );
	SplitScreenConVarRef( const char *pName, bool bIgnoreMissing );
	SplitScreenConVarRef( IConVar *pConVar );

	void Init( const char *pName, bool bIgnoreMissing );
	bool IsValid() const;
	bool IsFlagSet( int64 nFlags ) const;

	// Get/Set value
	float GetFloat( int nSlot ) const;
	int GetInt( int nSlot ) const;
	Color GetColor( int nSlot ) const;
	bool GetBool( int nSlot ) const { return !!GetInt( nSlot ); }
	const char *GetString( int nSlot  ) const;

	void SetValue( int nSlot, const char *pValue );
	void SetValue( int nSlot, float flValue );
	void SetValue( int nSlot, int nValue );
	void SetValue( int nSlot, Color value );
	void SetValue( int nSlot, bool bValue );

	const char *GetName( int nSlot ) const;

	const char *GetDefault() const;

	const char *GetBaseName() const;

private:
	struct cv_t
	{
		IConVar *m_pConVar;
		ConVar *m_pConVarState;
	};

	cv_t	m_Info[ MAX_SPLITSCREEN_CLIENTS ];
};

//-----------------------------------------------------------------------------
// Did we find an existing convar of that name?
//-----------------------------------------------------------------------------
FORCEINLINE_CVAR bool SplitScreenConVarRef::IsFlagSet( int64 nFlags ) const
{
	return ( m_Info[ 0 ].m_pConVar->IsFlagSet( nFlags ) != 0 );
}

FORCEINLINE_CVAR const char *SplitScreenConVarRef::GetName( int nSlot ) const
{
	return m_Info[ nSlot ].m_pConVar->GetName();
}

FORCEINLINE_CVAR const char *SplitScreenConVarRef::GetBaseName() const
{
	return m_Info[ 0 ].m_pConVar->GetBaseName();
}

//-----------------------------------------------------------------------------
// Purpose: Return ConVar value as a float
//-----------------------------------------------------------------------------
FORCEINLINE_CVAR float SplitScreenConVarRef::GetFloat( int nSlot ) const
{
	return m_Info[ nSlot ].m_pConVarState->m_Value.m_fValue;
}

//-----------------------------------------------------------------------------
// Purpose: Return ConVar value as an int
//-----------------------------------------------------------------------------
FORCEINLINE_CVAR int SplitScreenConVarRef::GetInt( int nSlot ) const 
{
	return m_Info[ nSlot ].m_pConVarState->m_Value.m_nValue;
}

//-----------------------------------------------------------------------------
// Purpose: Return ConVar value as an int
//-----------------------------------------------------------------------------
FORCEINLINE_CVAR Color SplitScreenConVarRef::GetColor( int nSlot ) const 
{
	return m_Info[ nSlot ].m_pConVarState->GetColor();
}

//-----------------------------------------------------------------------------
// Purpose: Return ConVar value as a string, return "" for bogus string pointer, etc.
//-----------------------------------------------------------------------------
FORCEINLINE_CVAR const char *SplitScreenConVarRef::GetString( int nSlot ) const 
{
	Assert( !IsFlagSet( FCVAR_NEVER_AS_STRING ) );
	return m_Info[ nSlot ].m_pConVarState->m_Value.m_pszString;
}


FORCEINLINE_CVAR void SplitScreenConVarRef::SetValue( int nSlot, const char *pValue )
{
	m_Info[ nSlot ].m_pConVar->SetValue( pValue );
}

FORCEINLINE_CVAR void SplitScreenConVarRef::SetValue( int nSlot, float flValue )
{
	m_Info[ nSlot ].m_pConVar->SetValue( flValue );
}

FORCEINLINE_CVAR void SplitScreenConVarRef::SetValue( int nSlot, int nValue )
{
	m_Info[ nSlot ].m_pConVar->SetValue( nValue );
}

FORCEINLINE_CVAR void SplitScreenConVarRef::SetValue( int nSlot, Color value )
{
	m_Info[ nSlot ].m_pConVar->SetValue( value );
}

FORCEINLINE_CVAR void SplitScreenConVarRef::SetValue( int nSlot, bool bValue )
{
	m_Info[ nSlot ].m_pConVar->SetValue( bValue ? 1 : 0 );
}

FORCEINLINE_CVAR const char *SplitScreenConVarRef::GetDefault() const
{
	return m_Info[ 0 ].m_pConVarState->m_pszDefaultValue;
}
#endif
#endif // CONVAR_WORK_FINISHED

//-----------------------------------------------------------------------------
// Called by the framework to register ConVars and ConCommands with the ICVar
//-----------------------------------------------------------------------------
void ConVar_Register( int64 nCVarFlag = 0 );
void ConVar_Unregister( );


//-----------------------------------------------------------------------------
// Utility methods 
//-----------------------------------------------------------------------------
void ConVar_PrintDescription( const CVarCreationBase_t* pVar );


//-----------------------------------------------------------------------------
// Purpose: Utility class to quickly allow ConCommands to call member methods
//-----------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning (disable : 4355 )
#endif

template< class T >
class CConCommandMemberAccessor : public ICommandCallback, public ICommandCompletionCallback, public ConCommand
{
	typedef ConCommand BaseClass;
	typedef void ( T::*FnMemberCommandCallback_t )( const CCommandContext &context, const CCommand &command );
	typedef int  ( T::*FnMemberCommandCompletionCallback_t )( const CCommand &command, CUtlVector< CUtlString > &completions );

public:
	CConCommandMemberAccessor( T* pOwner, const char *pName, FnMemberCommandCallback_t callback, const char *pHelpString = 0,
		int64 flags = 0, FnMemberCommandCompletionCallback_t completionFunc = 0 ) :
		BaseClass( pName, this, pHelpString, flags, ( completionFunc != 0 ) ? this : NULL )
	{
		m_pOwner = pOwner;
		m_Func = callback;
		m_CompletionFunc = completionFunc;
	}

	~CConCommandMemberAccessor()
	{
		this->Destroy();
	}

	void SetOwner( T* pOwner )
	{
		m_pOwner = pOwner;
	}

	virtual void CommandCallback( const CCommandContext &context, const CCommand &command ) override
	{
		Assert( m_pOwner && m_Func );
		(m_pOwner->*m_Func)( context, command );
	}

	virtual int  CommandCompletionCallback( const CCommand &command, CUtlVector< CUtlString > &completions ) override
	{
		Assert( m_pOwner && m_CompletionFunc );
		return (m_pOwner->*m_CompletionFunc)( command, completions );
	}

private:
	T* m_pOwner;
	FnMemberCommandCallback_t m_Func;
	FnMemberCommandCompletionCallback_t m_CompletionFunc;
};

#ifdef _MSC_VER
#pragma warning ( default : 4355 )
#endif

//-----------------------------------------------------------------------------
// Purpose: Utility macros to quicky generate a simple console command
//-----------------------------------------------------------------------------
#define CON_COMMAND( name, description ) \
   static void name##_callback( const CCommand &args ); \
   static ConCommand name##_command( #name, name##_callback, description ); \
   static void name##_callback( const CCommand &args )
#ifdef CLIENT_DLL
	#define CON_COMMAND_SHARED( name, description ) \
		static void name##_callback( const CCommandContext &context, const CCommand &args ); \
		static ConCommand name##_command_client( #name "_client", name##_callback, description ); \
		static void name##_callback( const CCommandContext &context, const CCommand &args )
#else
	#define CON_COMMAND_SHARED( name, description ) \
		static void name##_callback( const CCommandContext &context, const CCommand &args ); \
		static ConCommand name##_command( #name, name##_callback, description ); \
		static void name##_callback( const CCommandContext &context, const CCommand &args )
#endif


#define CON_COMMAND_F( name, description, flags ) \
	static void name##_callback( const CCommandContext &context, const CCommand &args ); \
	static ConCommand name##_command( #name, name##_callback, description, flags ); \
	static void name##_callback( const CCommandContext &context, const CCommand &args )

#ifdef CLIENT_DLL
	#define CON_COMMAND_F_SHARED( name, description, flags ) \
		static void name##_callback( const CCommandContext &context, const CCommand &args ); \
		static ConCommand name##_command_client( #name "_client", name##_callback, description, flags ); \
		static void name##_callback( const CCommandContext &context, const CCommand &args )
#else
	#define CON_COMMAND_F_SHARED( name, description, flags ) \
		static void name##_callback( const CCommandContext &context, const CCommand &args ); \
		static ConCommand name##_command( #name, name##_callback, description, flags ); \
		static void name##_callback( const CCommandContext &context, const CCommand &args )
#endif


#define CON_COMMAND_F_COMPLETION( name, description, flags, completion ) \
	static void name##_callback( const CCommandContext &context, const CCommand &args ); \
	static ConCommand name##_command( #name, name##_callback, description, flags, completion ); \
	static void name##_callback( const CCommandContext &context, const CCommand &args )

#ifdef CLIENT_DLL
	#define CON_COMMAND_F_COMPLETION_SHARED( name, description, flags, completion ) \
		static void name##_callback( const CCommandContext &context, const CCommand &args ); \
		static ConCommand name##_command_client( name##_command, #name "_client", name##_callback, description, flags, completion ); \
		static void name##_callback( const CCommandContext &context, const CCommand &args )
#else
	#define CON_COMMAND_F_COMPLETION_SHARED( name, description, flags, completion ) \
		static void name##_callback( const CCommandContext &context, const CCommand &args ); \
		static ConCommand name##_command( name##_command, #name, name##_callback, description, flags, completion ); \
		static void name##_callback( const CCommandContext &context, const CCommand &args )
#endif


#define CON_COMMAND_EXTERN( name, _funcname, description ) \
	void _funcname( const CCommandContext &context, const CCommand &args ); \
	static ConCommand name##_command( #name, _funcname, description ); \
	void _funcname( const CCommandContext &context, const CCommand &args )

#define CON_COMMAND_EXTERN_F( name, _funcname, description, flags ) \
	void _funcname( const CCommandContext &context, const CCommand &args ); \
	static ConCommand name##_command( #name, _funcname, description, flags ); \
	void _funcname( const CCommandContext &context, const CCommand &args )

#define CON_COMMAND_MEMBER_F( _thisclass, name, _funcname, description, flags ) \
	void _funcname( const CCommandContext &context, const CCommand &args );						\
	friend class CCommandMemberInitializer_##_funcname;			\
	class CCommandMemberInitializer_##_funcname					\
	{															\
	public:														\
		CCommandMemberInitializer_##_funcname() : m_ConCommandAccessor( NULL, name, &_thisclass::_funcname, description, flags )	\
		{														\
			m_ConCommandAccessor.SetOwner( GET_OUTER( _thisclass, m_##_funcname##_register ) );	\
		}														\
	private:													\
		CConCommandMemberAccessor< _thisclass > m_ConCommandAccessor;	\
	};															\
																\
	CCommandMemberInitializer_##_funcname m_##_funcname##_register;		\

#endif // CONVAR_H
