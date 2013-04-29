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

Implementations of parsers
*/

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <list>
#include <set>
#include <vector>

#include "srl.h"
#include "srlio.h"


using namespace std;

int SPredEntryParser::parse(const char* text, void *object){

	m_error = "";
	PredTable * ptable = (PredTable *)object;
	
	
	list<string> spl;
	boost::split(spl,string(text),boost::is_any_of("|"),boost::token_compress_on);

	if(spl.size()<3){
		m_error = "The line ";
		m_error += text;
		m_error += " does not have enough fields (3+)";
		return -1;
	}
	
	list<string>::const_iterator it = spl.begin();
	uint id = boost::lexical_cast<int>(*it);
	it++;
	string predstr = *it;
	boost::trim(predstr);
	it++;
	int group = boost::lexical_cast<int>(*it);

	PredEntry pred(id,predstr.c_str(),group);
	std::set<PredEntry::ArgStruct>& args = pred.GetArgs();
	args.clear();
	ArgumentType atype;
	int order ;
	double prob;
	vector<string> flds;
	flds.reserve(3);
	for(;it!=spl.end();it++){
		boost::split(flds,*it,boost::is_any_of(","),boost::token_compress_off);
		if(flds.size()!=3){
			m_error = "The feature entry ";
			m_error += *it;
			m_error += " does not have correct fields (3)";
			return -1;
		}
		atype =(ArgumentType) boost::lexical_cast<int>(flds[0]);
		order = boost::lexical_cast<int>(flds[1]);
		prob = boost::lexical_cast<double>(flds[2]);
		args.insert(PredEntry::ArgStruct(atype,order,prob));
	}

	if(ptable->InsertPred(pred)){
		return SRLIO_SUCCESSFUL;
	}else{
		m_error = "The line ";
		m_error += text;
		m_error += " has wrong id and cannot be inserted";
		return -1;
	}
}

std::string SPredEntryParser::getErrorMessage(int errorcode){
	return m_error;
}


int SArgEntryParser::parse(const char* text, void *object){

	m_error = "";
	ArgumentTable * argt = (ArgumentTable*) argt;

	vector<string> spl;
	spl.reserve(4);
	boost::split(spl,string(text),boost::is_any_of("|"),boost::token_compress_on);

	if(spl.size()!=4){
		m_error = "The line ";
		m_error += text;
		m_error += " does not have correct fields (4)";
		return -1;
	}

	uint id = boost::lexical_cast<int>(spl[0]);
	uint pred = boost::lexical_cast<int>(spl[1]);
	boost::trim(spl[2]);
	bool dir = spl[2]==string("L");
	ArgumentType arg = (ArgumentType)boost::lexical_cast<int>(spl[3]);

	if(argt->InsertArgument(id,pred,arg,dir)){
		return SRLIO_SUCCESSFUL;
	}else{
		m_error = "The line ";
		m_error += text;
		m_error += " has wrong id and cannot be inserted";
		return -1;
	}

}

std::string SArgEntryParser::getErrorMessage(int errorcode){
	return m_error;
}


int SFeatEntryParser::parse(const char* text, void *object){
	SRLFeatEntry* ret = (SRLFeatEntry*)object;
	ret->clear();
	

	m_error = "";
	
	list<string> spl;
	vector<string> info;
	info.reserve(4);
	boost::split(spl,string(text),boost::is_any_of(" "),boost::token_compress_on);
	ProvideFeature* pfeat = NULL;
	for(list<string>::iterator it = spl.begin();it!=spl.end();it++){
		const char* cstr = it->c_str();
		size_t len = it->size();
		if(len<2) continue;
		if(cstr[0] == 'p' || cstr[0] == 'a' ||cstr[0] == 'n' ){
			info.resize(0);
			boost::split(spl,string(cstr+1),boost::is_any_of("_"),boost::token_compress_on);
			if(info.size()<2){
				m_error = "Parse error " + *it;
				return -1;
			}
			uint id = boost::lexical_cast<int>(info[0]);
			uint start = boost::lexical_cast<int>(info[1]);
			uint len = info.size()==2 ? 1 : boost::lexical_cast<int>(info[2]);
			PfeatureType ty = (cstr[0] == 'p') ? PT_PREDICATE : ((cstr[0] == 'n') ? PT_NONTERMINAL : PT_ARGUMENT);
			if(pfeat){
				ret->AppendPf(*pfeat);
				delete pfeat;
				pfeat = NULL;
			}
			pfeat = new ProvideFeature(ty,id,start,len);
		} else if(cstr[0] >= 'A' && cstr[0] <= 'Z'){
			size_t offset = (cstr[1] >='a' && cstr[1]<='z') ? 2 : 1;
			FeatureId fid = offset==2 ? getFeatureIDInt(*cstr,*(cstr+1)) : getFeatureIDInt(*cstr,FEATUREID_NOCHAR);
			double value = boost::lexical_cast<double>(cstr+offset);
			if(!pfeat){
				m_error = "Feature without feature name " + *it;
				return -1;
			}
			pfeat->SetFeature(fid,value);
		}else{
			if(!pfeat){
				m_error = "Unexpected token " + *it;
				return -1;
			}
		}
	}

	if(pfeat){
		ret->AppendPf(*pfeat);
		delete pfeat;
	}
	return SRLIO_SUCCESSFUL;
}

std::string SFeatEntryParser::getErrorMessage(int errorcode){
	return m_error;
}
