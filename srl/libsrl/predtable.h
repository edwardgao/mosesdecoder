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

#ifndef __PREDTABLE_H__
#define __PREDTABLE_H__

#include <string>
#include <set>
#include <vector>
#include <list>
#include <map>

#include "defs.h"


/**
An entry of predicate structure in the predicate table
*/
class PredEntry {
public:
	/**
	The argument structure (list of arguments) for a predicate entry
	*/
	class ArgStruct{
	private:
		ArgumentType m_argtype;
		int m_order;
		mutable double m_prob; // prob should not be constant
	public:
		/// Get argument type of the structure
		inline ArgumentType GetArgType() const {return m_argtype;}; 
		/// Get the order and direction of the argument
		inline int GetOrder() const {return m_order;};
		/// Get the reference to the probability. It can be set even if the class is constant
		inline double& GetProb() const {return m_prob;}; 

	public: /*Operators*/
		/// Equal
		inline bool operator==(const ArgStruct& other)const {return m_argtype == other.m_argtype && m_order == other.m_order;};
		/// Order
		inline bool operator<(const ArgStruct& other)const {return m_order < other.m_order ? true : (m_order > other.m_order ? false : m_argtype < other.m_argtype);};

	public: /*Constructors*/
		ArgStruct(ArgumentType /*argtype*/, int /*order*/ , double prob = 0.0);

		ArgStruct(const ArgStruct& arg);
	};

private:
	uint m_id; 
	std::string m_pred;
	int m_group;
	std::set<ArgStruct> m_args;
public:
	/// Get ID
	inline int GetID() const {return m_id;};
	/// Set ID
	inline void SetID(int id) {m_id=id;};
	/// Get the predicate
	inline const std::string& GetPred() const {return m_pred;};
	/// Set the predicate
	inline void SetPred(const char *pred) {m_pred = pred;};
	/// Get the predicate group
	inline int GetGroup() const {return m_group;};
	/// Set the predicate group
	inline void SetGroup(int group) {m_group = group;};
	/// Get the argument list, this is constant implementation
	inline const std::set<ArgStruct>& GetArgs() const {return m_args;};
	/// Get the argument list, this is non-constant implementation
	inline std::set<ArgStruct>& GetArgs(){return m_args;};

public:
	/// Construct a predicate entry, at least need id and predicate representation
	PredEntry(int /*id*/, const char* /*pred*/, int group = 0);

	PredEntry(const PredEntry& entry);
public:
	/// Operator to determine if two predicate entry is the same
	bool operator==(const PredEntry& other) const;
};

/**
Table of predicate structures, will be referenced by srlfeat
*/
class PredTable : public IIndexDirectAccessable{
private:
	std::vector<PredEntry> m_entries;
	std::map<std::string, std::list<uint> >m_index;
public:
	/// Get the entry (non-constant)
	inline PredEntry& operator[](size_t idx){return m_entries[idx];};
	
public:
	/// Query the predicate entries, if it does not exist, you can specify insert_not_exist=true to insert
	/// @return the pair, first entry is the index and the second is a boolean whether it is found.
	std::pair<size_t, bool> QueryPred(const PredEntry& pred, bool insert_not_exist = false);
	/// Insert a new entry (the id will be verified and must match
	bool InsertPred(const PredEntry& pred);


	virtual size_t size() const {return m_entries.size();};
	virtual const void * operator[](size_t index) const {return &(m_entries[index]);};
};


#endif

