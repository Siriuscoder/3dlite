#include <assert.h>
#include <lite3dpp/as_helpers/scriptstdstring.h>
#include <lite3dpp/as_helpers/scriptarray.h>
#include <stdio.h>
#include <string.h>

using namespace std;

#include <lite3dpp/lite3dpp_manageable.h>

BEGIN_AS_NAMESPACE

// This function takes an input string and splits it into parts by looking
// for a specified delimiter. Example:
//
// string str = "A|B||D";
// array<string>@ array = str.split("|");
//
// The resulting array has the following elements:
//
// {"A", "B", "", "D"}
//
// AngelScript signature:
// array<string>@ string::split(const string &in delim) const
static CScriptArray *StringSplit(const lite3dpp::String &delim, const lite3dpp::String &str)
{
	// Obtain a pointer to the engine
	asIScriptContext *ctx = asGetActiveContext();
	asIScriptEngine *engine = ctx->GetEngine();

	// TODO: This should only be done once
	// TODO: This assumes that CScriptArray was already registered
	asIObjectType *arrayType = engine->GetObjectTypeByDecl("array<string>");

	// Create the array object
	CScriptArray *array = CScriptArray::Create(arrayType);

	// Find the existence of the delimiter in the input string
	int pos = 0, prev = 0, count = 0;
	while( (pos = (int)str.find(delim, prev)) != (int)lite3dpp::String::npos )
	{
		// Add the part to the array
		array->Resize(array->GetSize()+1);
		((lite3dpp::String*)array->At(count))->assign(&str[prev], pos-prev);

		// Find the next part
		count++;
		prev = pos + (int)delim.length();
	}

	// Add the remaining part
	array->Resize(array->GetSize()+1);
	((lite3dpp::String*)array->At(count))->assign(&str[prev]);

	return array;
}

static void StringSplit_Generic(asIScriptGeneric *gen)
{
	// Get the arguments
	lite3dpp::String *str   = (lite3dpp::String*)gen->GetObject();
	lite3dpp::String *delim = *(lite3dpp::String**)gen->GetAddressOfArg(0);

	// Return the array by handle
	*(CScriptArray**)gen->GetAddressOfReturnLocation() = StringSplit(*delim, *str);
}



// This function takes as input an array of string handles as well as a
// delimiter and concatenates the array elements into one delimited string.
// Example:
//
// array<string> array = {"A", "B", "", "D"};
// string str = join(array, "|");
//
// The resulting string is:
//
// "A|B||D"
//
// AngelScript signature:
// string join(const array<string> &in array, const string &in delim)
static lite3dpp::String StringJoin(const CScriptArray &array, const lite3dpp::String &delim)
{
	// Create the new string
	lite3dpp::String str = "";
	if( array.GetSize() )
	{
		int n;
		for( n = 0; n < (int)array.GetSize() - 1; n++ )
		{
			str += *(lite3dpp::String*)array.At(n);
			str += delim;
		}

		// Add the last part
		str += *(lite3dpp::String*)array.At(n);
	}

	return str;
}

static void StringJoin_Generic(asIScriptGeneric *gen)
{
	// Get the arguments
	CScriptArray  *array = *(CScriptArray**)gen->GetAddressOfArg(0);
	lite3dpp::String *delim = *(lite3dpp::String**)gen->GetAddressOfArg(1);

	// Return the string
	new(gen->GetAddressOfReturnLocation()) lite3dpp::String(StringJoin(*array, *delim));
}

// This is where the utility functions are registered.
// The string type must have been registered first.
void RegisterStdStringUtils(asIScriptEngine *engine)
{
	int r;

	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
	{
		r = engine->RegisterObjectMethod("string", "array<string>@ split(const string &in) const", asFUNCTION(StringSplit_Generic), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterGlobalFunction("string join(const array<string> &in, const string &in)", asFUNCTION(StringJoin_Generic), asCALL_GENERIC); assert(r >= 0);
	}
	else
	{
		r = engine->RegisterObjectMethod("string", "array<string>@ split(const string &in) const", asFUNCTION(StringSplit), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterGlobalFunction("string join(const array<string> &in, const string &in)", asFUNCTION(StringJoin), asCALL_CDECL); assert(r >= 0);
	}
}

END_AS_NAMESPACE
