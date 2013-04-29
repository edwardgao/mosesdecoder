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

Implementation of formatters
*/

#include "srl.h"
#include "srlio.h"

#include <string>
#include <sstream>
#include <set>
#include <boost/algorithm/string.hpp>

using namespace std;

string SPredEntryFmter::getErrorMessage(int errorcode){
	return m_error;
}


string SArgEntryFmter::getErrorMessage(int errorcode){
	return m_error;
}

string SFeatEntryFmter::getErrorMessage(int errorcode){
	return m_error;
}

int SPredEntryFmter::format(std::string& ostr, const void *object, int index){
	m_error = "";
	const PredEntry* pred = (const PredEntry*) object;
	stringstream ss;
	ss << index << " ||| " << pred->GetPred() << " ||| " << pred->GetGroup() ;
	bool firstsep = true;
	set<PredEntry::ArgStruct>::const_iterator it = pred->GetArgs().begin();
	for(;it!=pred->GetArgs().end();it++){
		if(firstsep)
			ss << " ||| ";
		else
			ss << " | ";
		ss << it->GetArgType() << "," << it->GetOrder() << "," << it->GetProb();
	}
	ss << endl;
	ostr = ss.str();
	return SRLIO_SUCCESSFUL;
}

int SArgEntryFmter::format(std::string& ostr, const void *object, int index){
	m_error = "";
	const int& arg = *((const int*)object);
	stringstream ss;
	ss << index << " ||| " << ArgumentTable::ConvertPredicate(arg) << " ||| " << (ArgumentTable::ConvertPosition(arg) ? "L" : "R") << " ||| "
		<< ArgumentTable::ConvertArgType(arg) << endl;
	ostr = ss.str();
	return SRLIO_SUCCESSFUL;
}

int SFeatEntryFmter::format(std::string& ostr, const void *object, int index){
	m_error = "";
	stringstream ss;
	const SRLFeatEntry& ret = *(const SRLFeatEntry*)object;
	for(size_t i = 0; i< ret.GetNumPf() ; i++){
		const ProvideFeature& pf = ret.GetPf(i);
		PfeatureType ty = pf.GetType();

		ss << (ty==PT_PREDICATE ? "p" : (ty==PT_ARGUMENT ? "a" : "n"));
		ss << pf.GetID() << "_" << pf.GetStart() << "_" << pf.GetLength() << " ";

		for(ProvideFeature::iterator it = pf.begin(); it!=pf.end(); it++){
			ss << getFeatureIDStr(it->first) << it->second << " ";
		}
	}
	ostr = ss.str();
	boost::trim(ostr);
	return SRLIO_SUCCESSFUL;
}