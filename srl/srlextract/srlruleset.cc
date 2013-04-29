
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/lexical_cast.hpp>

#include "srlruleset.h"
#include "IOUtils.h"


using namespace std;
using namespace boost;

namespace srl{

SRLGenRuleSet::SRLGenRuleSet(const char* inputf){

	OPEN_STREAM_OR_DIE(ifstream, input,inputf);
	
	string line;
	while(true){
		line = "";
		getline(input,line);
		if(line.length()==0){
			break;
		}
		vector<string> entries;
		entries.reserve(4);


//		typedef boost::split_iterator<string::iterator> char_split_iterator;
//
//		for(char_split_iterator It=char_split_iterator(line.begin(), line.end(),boost::first_finder(" ||| "));
//				It!=char_split_iterator(); ++It) {
//			string str = *it;
//			entries.push_back(str);
//		}
		boost::algorithm::split_regex(entries,line,regex(" \\|\\|\\| "));
		if(entries.size()!=5)
			continue;

		SRLGenRuleSet::TFrameMap::iterator it;
		pair<SRLGenRuleSet::TFrameMap::iterator,bool> ret =
				rules_.insert(pair<string, SRLGenRuleSet::TArgMap>(entries[0], SRLGenRuleSet::TArgMap()));
		it = ret.first;
		pair<SRLGenRuleSet::TArgMap::iterator,bool> ret2 =
				it->second.insert(pair<string, SRLGenRuleSet::TPairList>(entries[1],SRLGenRuleSet::TPairList()));
		SRLGenRuleSet::TArgMap::iterator it2 = ret2.first;

		SRLGenRuleWithFeature sgen;

		sgen.source = entries[2];
		sgen.target = entries[3];

		trim(entries[4]);


		vector<string> features;
		features.reserve(7);
		boost::algorithm::split_regex(features,entries[4],regex("\\s+"));
		sgen.features.resize(features.size());
		for(size_t j =0 ; j< features.size();j++){
			sgen.features[j] = lexical_cast<float>(features[j]);
		}

		it2->second.push_back(sgen);


	}
}

const std::vector<SRLGenRuleWithFeature>& SRLGenRuleSet::GetRules(const string& frame, const string& argument) const{
	SRLGenRuleSet::TFrameMap::const_iterator it;
	it = rules_.find(frame);
	if(it == rules_.end())
		return null_list_;
	SRLGenRuleSet::TArgMap::const_iterator it2;
	it2 = it->second.find(argument);
	if(it2 == it->second.end())
		return null_list_;
	return it2->second;
}


SRLGenMonoRuleSet::SRLGenMonoRuleSet(const char* inputf){

	OPEN_STREAM_OR_DIE(ifstream, input,inputf);
	string line;
	while(true){
		line = "";
		getline(input,line);
		if(line.length()==0){
			break;
		}
		vector<string> entries;
		entries.reserve(4);


		boost::algorithm::split_regex(entries,line,regex(" \\|\\|\\| "));
		if(entries.size()<3)
			continue;

		SRLGenMonoRuleSet::TFrameMap::iterator it;
		pair<SRLGenMonoRuleSet::TFrameMap::iterator,bool> ret =
			rules_.insert(pair<string, SRLGenMonoRuleSet::TArgMap>(entries[0], SRLGenMonoRuleSet::TArgMap()));
		it = ret.first;
		pair<SRLGenMonoRuleSet::TArgMap::iterator,bool> ret2 =
			it->second.insert(pair<string, SRLGenMonoRuleSet::TList>(entries[1],SRLGenMonoRuleSet::TList()));
		SRLGenMonoRuleSet::TArgMap::iterator it2 = ret2.first;

				

		trim(entries[2]);


		it2->second.push_back(entries[2]); // Must have been sorted


	}
}

const std::vector<std::string>& SRLGenMonoRuleSet::GetRules(const string& frame, const string& argument) const{
	SRLGenMonoRuleSet::TFrameMap::const_iterator it;
	it = rules_.find(frame);
	if(it == rules_.end())
		return null_list_;
	SRLGenMonoRuleSet::TArgMap::const_iterator it2;
	it2 = it->second.find(argument);
	if(it2 == it->second.end())
		return null_list_;
	return it2->second;
}

}
