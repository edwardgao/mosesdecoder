/**
@file 
@author Qin Gao <pku.gaoqin@gmail.com>
@version 0.0.1

@section LICENSE

The source code can be used freely as long as the author is aware of the usage. Send an email
to the author before using it.

The source code is provide "as is" and the author is not responsible for any effect/damage of the code.

The banner of the source code should not be removed.

@section DESCRIPTION

The definitions of argument types etc.
*/

#ifndef __DEFS_H__
#define __DEFS_H__

typedef unsigned int uint;

/**
An integer representation of Argument types.
Can be converted from string using GetArgumentType() functor.
*/
enum ArgumentType {
	AT_NDEF, AT_ARG0, AT_ARG1, AT_ARG2, AT_ARG3, AT_ARG4, AT_ARG5, AT_TMP, AT_MOD, 
};


/**
An integer representation of feature type (predicate/argument or non-terminal)
*/
enum PfeatureType{
	PT_PREDICATE=0,
	PT_ARGUMENT=1,
	PT_NONTERMINAL=2,
	PT_NDEF = 3,
};

/**
An integer representation of feature id (what kind of feature)
*/
typedef unsigned int FeatureId;

/**
To provide a flexible mapping from argument representation to argument type, this 
functor can be override.
*/
class GetArgumentType{
	/**
	The protocol of converting a string represenatation to an argument type
	*/
	virtual ArgumentType operator()(const char* /*Rep*/) = 0;
};

/**
An interface that can be navigated through, with IDs
*/
class IIndexDirectAccessable {
public:
	/**
	Return the size of the 
	*/
	virtual size_t size() const = 0;
	virtual const void * operator[](size_t index) const = 0;
};
#endif