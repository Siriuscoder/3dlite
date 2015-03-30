#ifndef SCRIPTBUILDER_H
#define SCRIPTBUILDER_H

//---------------------------
// Compilation settings
//

// Set this flag to turn on/off metadata processing
//  0 = off
//  1 = on
#ifndef AS_PROCESS_METADATA
#define AS_PROCESS_METADATA 1
#endif

// TODO: Implement flags for turning on/off include directives and conditional programming



//---------------------------
// Declaration
//

#ifndef ANGELSCRIPT_H 
// Avoid having to inform include path if header is already include before
#include <3dlitepp/as/angelscript.h>
#endif


#if defined(_MSC_VER) && _MSC_VER <= 1200 
// disable the annoying warnings on MSVC 6
#pragma warning (disable:4786)
#endif

#include <string>
#include <map>
#include <set>
#include <vector>

#include <3dlitepp/3dlitepp_manageable.h>

BEGIN_AS_NAMESPACE

class CScriptBuilder;

// This callback will be called for each #include directive encountered by the
// builder. The callback should call the AddSectionFromFile or AddSectionFromMemory
// to add the included section to the script. If the include cannot be resolved
// then the function should return a negative value to abort the compilation.
typedef int (*INCLUDECALLBACK_t)(const char *include, const char *from, CScriptBuilder *builder, void *userParam);

// Helper class for loading and pre-processing script files to 
// support include directives and metadata declarations
class CScriptBuilder
{
public:
	CScriptBuilder();

	// Start a new module
	int StartNewModule(asIScriptEngine *engine, const char *moduleName);

	// Load a script section from a file on disk
	// Returns  1 if the file was included
	//          0 if the file had already been included before
	//         <0 on error
	int AddSectionFromFile(const char *filename);

	// Load a script section from memory
	// Returns  1 if the section was included
	//          0 if a section with the same name had already been included before
	//         <0 on error
	int AddSectionFromMemory(const char *sectionName,
							 const char *scriptCode, 
							 unsigned int scriptLength = 0,
							 int lineOffset = 0);

	// Build the added script sections
	int BuildModule();

	// Returns the current module
	asIScriptModule *GetModule();

	// Register the callback for resolving include directive
	void SetIncludeCallback(INCLUDECALLBACK_t callback, void *userParam);

	// Add a pre-processor define for conditional compilation
	void DefineWord(const char *word);

	// Enumerate included script sections
	unsigned int GetSectionCount() const;
	lite3dpp::lite3dpp_string  GetSectionName(unsigned int idx) const;

#if AS_PROCESS_METADATA == 1
	// Get metadata declared for class types and interfaces
	const char *GetMetadataStringForType(int typeId);

	// Get metadata declared for functions
	const char *GetMetadataStringForFunc(asIScriptFunction *func);

	// Get metadata declared for global variables
	const char *GetMetadataStringForVar(int varIdx);

	// Get metadata declared for class variables
	const char *GetMetadataStringForTypeProperty(int typeId, int varIdx);

	// Get metadata declared for class functions
	const char *GetMetadataStringForTypeMethod(int typeId, asIScriptFunction *method);
#endif

protected:
	void ClearAll();
	int  Build();
	int  ProcessScriptSection(const char *script, unsigned int length, const char *sectionname, int lineOffset);
	int  LoadScriptSection(const char *filename);
	bool IncludeIfNotAlreadyIncluded(const char *filename);

	int  SkipStatement(int pos);

	int  ExcludeCode(int start);
	void OverwriteCode(int start, int len);

	asIScriptEngine           *engine;
	asIScriptModule           *module;
	lite3dpp::lite3dpp_string modifiedScript;

	INCLUDECALLBACK_t  includeCallback;
	void              *callbackParam;

#if AS_PROCESS_METADATA == 1
	int  ExtractMetadataString(int pos, lite3dpp::lite3dpp_string &outMetadata);
	int  ExtractDeclaration(int pos, lite3dpp::lite3dpp_string &outDeclaration, int &outType);

	// Temporary structure for storing metadata and declaration
	struct SMetadataDecl
	{
		SMetadataDecl(lite3dpp::lite3dpp_string m, lite3dpp::lite3dpp_string d, int t, lite3dpp::lite3dpp_string c, lite3dpp::lite3dpp_string ns) : metadata(m), declaration(d), type(t), parentClass(c), nameSpace(ns) {}
		lite3dpp::lite3dpp_string  metadata;
		lite3dpp::lite3dpp_string  declaration;
		int                         type;
		lite3dpp::lite3dpp_string  parentClass;
		lite3dpp::lite3dpp_string  nameSpace;
	};
	lite3dpp::stl<SMetadataDecl>::vector foundDeclarations;
	lite3dpp::lite3dpp_string currentClass;
	lite3dpp::lite3dpp_string currentNamespace;

	// Storage of metadata for global declarations
	lite3dpp::stl<int, lite3dpp::lite3dpp_string>::map typeMetadataMap;
	lite3dpp::stl<int, lite3dpp::lite3dpp_string>::map funcMetadataMap;
	lite3dpp::stl<int, lite3dpp::lite3dpp_string>::map varMetadataMap;

	// Storage of metadata for class member declarations
	struct SClassMetadata
	{
		SClassMetadata(const lite3dpp::lite3dpp_string& aName) : className(aName) {}
		lite3dpp::lite3dpp_string className;
		lite3dpp::stl<int, lite3dpp::lite3dpp_string>::map funcMetadataMap;
		lite3dpp::stl<int, lite3dpp::lite3dpp_string>::map varMetadataMap;
	};
    lite3dpp::stl<int, SClassMetadata>::map classMetadataMap;

#endif

	lite3dpp::stl<lite3dpp::lite3dpp_string>::set      includedScripts;
	lite3dpp::stl<lite3dpp::lite3dpp_string>::set      definedWords;
};

END_AS_NAMESPACE

#endif
