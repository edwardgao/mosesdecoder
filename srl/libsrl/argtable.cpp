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

Implementation of argument table
*/
#include "argtable.h"

using namespace std;

std::pair<size_t, bool> ArgumentTable::QueryPred(size_t pred, ArgumentType arg, bool is_on_right, bool insert_not_exist){
	int entry = (is_on_right ? 1 : -1 ) * ( pred * MAX_ARGUMENT_TYPES  + (int) arg );

	pair<int, bool> result;
	map<uint, list<uint> >::iterator it;
	it = m_index.find(pred);

	if(it == m_index.end()){
		if(insert_not_exist){
			pair<map<uint, list<uint> >::iterator, bool> ret = m_index.insert(make_pair(pred,list<uint>()));
			it = ret.first;
		}else{
			result.first = 0; 
			result.second = false;
			return result;
		}
	}

	for(list<uint>::const_iterator iit = it->second.begin() ; iit!=it->second.end(); iit++){
		if(m_preddir[*iit] == entry){
			result.first = *iit;
			result.second = true;
			return result;
		}
	}
	// Not found
	if(insert_not_exist){
		uint nindex = m_preddir.size();
		m_preddir.push_back(entry);
		it->second.push_back(nindex);
		result.first = nindex;
		result.second = false;
		return result;
	}else{
		result.first = 0; 
		result.second = false;
		return result;
	}

}

bool ArgumentTable::InsertArgument(size_t id, size_t pred, ArgumentType arg, bool is_on_right){
	uint nindex = m_preddir.size();
	if(nindex!=id)
		return false;
	int entry = (is_on_right ? 1 : -1 ) * ( pred * MAX_ARGUMENT_TYPES  + (int) arg );
	m_preddir.push_back(entry);
	map<uint, list<uint> >::iterator it;
	it = m_index.find(pred);
	if(it == m_index.end()){
		pair<map<uint, list<uint> >::iterator, bool> ret = m_index.insert(make_pair(pred,list<uint>()));
		it = ret.first;
	}
	it->second.push_back(nindex);
	return true;
}
