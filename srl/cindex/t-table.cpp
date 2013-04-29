#pragma warning(disable : 4996)

#include <stdio.h>
#include <string>
#include <list>
#include <iostream>
#include <vector>
#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/lambda.hpp>

#include "SimpleLogger.h"
#include "IOUtils.h"
#include "t-table.h"


namespace cindex{
	using namespace std;

	using namespace boost;
	using namespace boost::filesystem;

	TTable::TTable(double floorvalue){
		floor = floorvalue;
	}


	double TTable::GetProb(const string& src, const string& tgt){
		TTableType::const_iterator it = ttable.find(src);
		if(it == ttable.end()){
			return uniform;
		}
		TDictType::const_iterator it1 = it->second.find(tgt);
		if(it1 == it->second.end()){
			return floor;
		}
		return it1->second;
	}

	double TTable::GetProb(const vector<string>& src, const vector<string>& tgt){
		double viterbi = 1.0;
		// One source word aligns to exactly 1 target word, it could be NULL

		double prob;
		for(size_t i = 0; i< src.size() ; i++){
			double maxprob = -1;
			for(size_t j = 0; j < tgt.size() ; j++){
				prob = GetProb(src[i],tgt[j]);
				if(prob > maxprob)
					maxprob = prob;
			}
			prob = GetProb(src[i],"NULL");
			if(prob > maxprob){
				maxprob = prob;
			}
			viterbi *= maxprob;
		}

		return viterbi;
	}

	bool TTable::LoadTable(const char* filename, bool verify){
		OPEN_STREAM_OR_DIE(ifstream, ifs, filename);
		return LoadTable(ifs,verify);
	}

	bool TTable::LoadTable(istream& input, bool verify){
		string line;
		DictSetType tgtSet;
		vector<string> lp;
		lp.reserve(4);
		int l = 0;
		
		while(getline(input,line)){
			l++;
			split(lp,line,is_any_of(" \t"),token_compress_on);
			if(lp.size()!=3){
				P_WARN("Found and ignored a bad line (%d), content %s. ", l, line);
				continue;
			}
			double value = lexical_cast<float>(lp[2]);
			
			if(value > floor){
				string& src = lp[0], tgt = lp[1];
				pair<TTableType::iterator, bool> ind = ttable.insert(TTableType::value_type(src,TDictType()));
				pair<TDictType::iterator,bool> ind1 = ind.first->second.insert(TDictType::value_type(tgt,value));
				if(!ind1.second){
					P_WARN("Duplicate entry %s " , line.c_str());
					ind1.first->second += value;
				}
				tgtSet.insert(tgt);
			}
			if(l%10000 == 0){
				cerr << "\r[" << l << "]      ";
			}
		}

		if(verify){
			P_INFO("Verifying T-Table");
			for(TTableType::const_iterator it = ttable.begin();
				it != ttable.end(); it++){
				double sum = 0;
				for(TDictType::const_iterator it1 = it->second.begin();
					it1 != it->second.end(); it1++){
					sum += it1->second;
				}
				if(1-sum > floor * 10000 || sum - 1 > floor * 10000){
					P_WARN("Bad probability sum (%lf) for word %s", sum, it->first.c_str());
				}
			}
		}
		uniform = 1.0 /tgtSet.size();

		return true;
	}
}

