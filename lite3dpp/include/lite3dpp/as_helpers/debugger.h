#ifndef DEBUGGER_H
#define DEBUGGER_H

#ifndef ANGELSCRIPT_H 
// Avoid having to inform include path if header is already include before
#include <lite3dpp/as/angelscript.h>
#endif

#include <vector>
#include <map>

#include <lite3dpp/lite3dpp_manageable.h>

BEGIN_AS_NAMESPACE

class CDebugger
{
public:
	CDebugger();
	virtual ~CDebugger();

	// Register callbacks to handle to-lite3dpp::lited3dpp_string conversions of application types
	typedef lite3dpp::String (*ToStringCallback)(void *obj, bool expandMembers, CDebugger *dbg);
	virtual void RegisterToStringCallback(const asIObjectType *ot, ToStringCallback callback);

	// User interaction
	virtual void TakeCommands(asIScriptContext *ctx);
	virtual void Output(const lite3dpp::String &str);

	// Line callback invoked by context
	virtual void LineCallback(asIScriptContext *ctx);

	// Commands
	virtual void PrintHelp();
	virtual void AddFileBreakPoint(const lite3dpp::String &file, int lineNbr);
	virtual void AddFuncBreakPoint(const lite3dpp::String &func);
	virtual void ListBreakPoints();
	virtual void ListLocalVariables(asIScriptContext *ctx);
	virtual void ListGlobalVariables(asIScriptContext *ctx);
	virtual void ListMemberProperties(asIScriptContext *ctx);
	virtual void ListStatistics(asIScriptContext *ctx);
	virtual void PrintCallstack(asIScriptContext *ctx);
	virtual void PrintValue(const lite3dpp::String &expr, asIScriptContext *ctx);

	// Helpers
	virtual bool InterpretCommand(const lite3dpp::String &cmd, asIScriptContext *ctx);
	virtual bool CheckBreakPoint(asIScriptContext *ctx);
	virtual lite3dpp::String ToString(void *value, asUINT typeId, bool expandMembers, asIScriptEngine *engine);

protected:
	enum DebugAction
	{
		CONTINUE,  // continue until next break point
		STEP_INTO, // stop at next instruction
		STEP_OVER, // stop at next instruction, skipping called functions
		STEP_OUT   // run until returning from current function
	};
	DebugAction        m_action;
	asUINT             m_lastCommandAtStackLevel;
	asIScriptFunction *m_lastFunction;

	struct BreakPoint
	{
		BreakPoint(lite3dpp::String f, int n, bool _func) : name(f), lineNbr(n), func(_func), needsAdjusting(true) {}
		lite3dpp::String name;
		int         lineNbr;
		bool        func;
		bool        needsAdjusting;
	};
	std::vector<BreakPoint> m_breakPoints;

	// Registered callbacks for converting objects to strings
	std::map<const asIObjectType*, ToStringCallback> m_toStringCallbacks;
};

END_AS_NAMESPACE

#endif