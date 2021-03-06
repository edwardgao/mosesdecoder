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

Implementation for predicate table
*/

#include "predtable.h"

using namespace std;
PredEntry::ArgStruct::ArgStruct(ArgumentType argtype, int order , double prob):
m_argtype(argtype) , m_order(order), m_prob(prob){}

PredEntry::ArgStruct::ArgStruct(const PredEntry::ArgStruct& arg):
m_argtype(arg.m_argtype) , m_order(arg.m_order), m_prob(arg.m_prob){}


PredEntry::PredEntry(int id, const char* pred, int group):
m_id(id), m_group(group), m_pred(pred){}

PredEntry::PredEntry(const PredEntry& entry):
m_id(entry.m_id), m_group(entry.m_group),m_pred(entry.m_pred),m_args(entry.m_args)
{}

bool PredEntry::operator==(const PredEntry& other) const{
	// The ID should be ignored, the predicate and argument list should be the same
	if(m_pred != other.m_pred){
		return false;
	}
	if(m_args.size()!=other.m_args.size()){
		return false;
	}
	set<PredEntry::ArgStruct>::const_iterator it = m_args.begin();
	set<PredEntry::ArgStruct>::const_iterator it1 = other.m_args.begin();
	while(it!=m_args.end()){
		if(!((*it)==(*it1))){
			return false;
		}
	}
	return true;
}
pair<size_t, bool> PredTable::QueryPred(const PredEntry& pred, bool insert_not_exist){
	pair<size_t, bool> result;
	const string& rep = pred.GetPred();
	map<string, list<size_t> >::iterator it;
	it = m_index.find(rep);
	if(it == m_index.end()){
		if(insert_not_exist){
			pair<map<string, list<size_t> >::iterator, bool> ret = m_index.insert(make_pair(rep,list<size_t>()));
			it = ret.first;
		}else{
			result.first = 0; 
			result.second = false;
			return result;
		}
	}

	for(list<uint>::const_iterator iit = it->second.begin() ; iit!=it->second.end(); iit++){
		if(m_entries[*iit] == pred){
			result.first = *iit;
			result.second = true;
			return result;
		}
	}
	// Not found
	if(insert_not_exist){
		size_t nindex = m_entries.size();
		PredEntry newEntry(pred);
		newEntry.SetID(nindex);
		m_entries.push_back(newEntry);
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

bool PredTable::InsertPred(const PredEntry& pred){
	size_t nindex = m_entries.size();
	if(pred.GetID()!=nindex){
		return false;
	}
	m_entries.push_back(pred);
	const string& rep = pred.GetPred();
	map<string, list<size_t> >::iterator it;
	it = m_index.find(rep);
	if(it == m_index.end()){
		pair<map<string, list<size_t> >::iterator, bool> ret = m_index.insert(make_pair(rep,list<size_t>()));
		it = ret.first;
	}
	it->second.push_back(nindex);
	return true;
}



