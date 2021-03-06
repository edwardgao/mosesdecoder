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

The header contains definition of predicate structure table, providing the in-memory storage structure 
and necessary methods of querying/manipulating the structure.
*/

#ifndef __ARG_TABLE_H__
#define __ARG_TABLE_H__

#include <vector>
#include <map>
#include <list>

#include "defs.h"

class ArgumentTable : public IIndexDirectAccessable{
public:
	static const int MAX_ARGUMENT_TYPES = 128;
private:
	std::vector<int> m_preddir; // Predicate, argument label and direction, the sign is the direction
	std::map<uint, std::list<uint> > m_index; // Index from predicate to the index

public:
	static inline bool ConvertPosition(int rep){return rep > 0;};
	/// Get the predicate structure id
	static inline int ConvertPredicate(int rep){return rep > 0 ? (rep / MAX_ARGUMENT_TYPES) :   ( (0 - rep)/MAX_ARGUMENT_TYPES);}
	/// Get the argument id
	static inline ArgumentType ConvertArgType(int rep){return (ArgumentType) (rep > 0 ? (rep % MAX_ARGUMENT_TYPES) :   ( (0 - rep)%MAX_ARGUMENT_TYPES));}

	/// Get the position of the predicates and arguments
	inline bool GetPosition(size_t i) const {return ArgumentTable::ConvertPosition(m_preddir[i]);}; // True means to the right of the predicate, false means to the left
	/// Get the predicate structure id
	inline int GetPredicate(size_t i) const {return ArgumentTable::ConvertPredicate(m_preddir[i]);}
	/// Get the argument id
	inline ArgumentType GetArgType(size_t i) const {return ArgumentTable::ConvertArgType(m_preddir[i]);}

public:
	/// Insert a new argument, the id will be checked.
	bool InsertArgument(size_t id, size_t pred, ArgumentType arg, bool is_on_right);
	/// Query the argument entries, if it does not exist, you can specify insert_not_exist=true to insert
	/// @return the pair, first entry is the index and the second is a boolean whether it is found.
	std::pair<size_t, bool> QueryPred(size_t pred, ArgumentType arg, bool is_on_right, bool insert_not_exist = false);

public: // Support IIndexDirectAccessable

	virtual size_t size() const {return m_preddir.size();};
	virtual const void * operator[](size_t index) const {return &(m_preddir[index]);};
}; 

#endif