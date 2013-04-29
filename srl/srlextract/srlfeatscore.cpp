/*
 * srlfeatscore.cpp
 *
 *  Created on: Nov 17, 2010
 *      Author: qing
 */

#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/lexical_cast.hpp>

#include "Params.h"
#include "SimpleLogger.h"

using namespace std;
using namespace boost;

struct Rule{
	string lhs, rhs_src, rhs_tgt, align;
	float weight;
};

void split_words(string tk, map<string,float>& buf, float weight){
	trim(tk);
	vector<string> v;
	split(v,tk,is_any_of(" "), token_compress_on);
	for(int i = 0 ; i< (int)v.size(); i++){
		pair<map<string,float>::iterator,bool> it = buf.insert(pair<string,float>(v[i],weight));
		if(!it.second){
			it.first->second += weight;
		}
	}
}

struct s_vent{
	float prob;
	string rep;
	inline bool operator<(const s_vent& v){return prob > v.prob;}
};
list<s_vent> normalize(map<string,float>& flt, int max_n){
	float sum =0;
	list<s_vent> clist;
	for(map<string,float>::iterator it = flt.begin(); it!=flt.end();it++){
		sum += it->second;
	}
	s_vent vent;
	for(map<string,float>::iterator it = flt.begin(); it!=flt.end();it++){
		it->second /= sum;
		vent.prob = it->second;
		vent.rep = it->first;
		clist.push_back(vent);
	}
	clist.sort();
	int i=0;
	list<s_vent> ret;
	sum = 0;
	for(list<s_vent>::iterator it = clist.begin(); it!=clist.end() && i < max_n; it++, i++){
		ret.push_back(*it);
		sum += it->prob;
	}
	if(1-sum<1 && 1-sum > 1e-5){
		s_vent vent;
		vent.prob = 1-sum;
		vent.rep = "GS_O";
		ret.push_back(vent);
	}
	return ret;
}

string output_result_one(list<s_vent>& flt){
	stringstream out;
	for(list<s_vent>::iterator it = flt.begin();
			it != flt.end() ; it++){
		out << it->rep << ":" << it->prob << " ";
	}

	return out.str();
}

void output_all(vector<Rule>& rules, string& feat, ostream& output){
	for(int i = 0 ; i< (int) rules.size(); i++){
		Rule& r = rules[i];
		output << r.rhs_tgt << " ||| "<< feat << endl;
		break;
//		if(i>0 && r.lhs == rules[i-1].lhs && r.rhs_tgt == rules[i-1].rhs_tgt && r.rhs_src == rules[i-1].rhs_src && r.align == rules[i-1].align){
//			continue;
//		}
//		output << r.rhs_src << " ||| " << r.rhs_tgt << " ||| " << r.align << " ||| "
//				<< feat << endl;
	}
}

void filter_alignment(Rule& r){
	vector<string> rhst;
	split(rhst,r.rhs_tgt,is_any_of(" "),token_compress_on);
	int keep1=-1 , keep2 = -1;
	for(int i =0; i< (int)rhst.size()-1; i++){
		if(rhst[i][0] == '[' && rhst[i][rhst[i].size()-1]==']'){
			if(keep1 <0)
				keep1 = i;
			else
				keep2 = i;
		}
	}

	vector<string> aln;
	split(aln,r.align,is_any_of(" "), token_compress_on);
	set<string> outBuffer;
	for(int i =0 ; i< (int)aln.size(); i++){
		vector<string> fld;
		fld.reserve(2);
		split(fld,aln[i],is_any_of("-"), token_compress_on);
		if(fld.size()!=2) continue;
		int tfl = lexical_cast<int>(fld[0]);
		if(tfl == keep1 || tfl == keep2){
			stringstream out;
			out << fld[1] << "-" << fld[0] ;
			outBuffer.insert(out.str());
		}
	}
	string outfinal;
	for(set<string>::const_iterator it = outBuffer.begin(); it != outBuffer.end(); it++){
		outfinal += *it + " ";
	}
	r.align = outfinal;
	trim(r.align);
}

bool compare_alignment(const string& c1, const string& c2){
	vector<string> v1, v2;
	split(v1,c1,is_any_of(" "),token_compress_on);
	split(v2,c2,is_any_of(" "),token_compress_on);

	set<string> s1,s2;
	s1.insert(v1.begin(),v1.end());
	s2.insert(v2.begin(),v2.end());
	if(s1.size()!=s2.size()) return false;
	set<string>::iterator it1, it2;

	it1 = s1.begin();
	it2 = s2.begin();
	while(it1!=s1.end()&& it2!=s2.end()){
		if(*it1 != *it2){
			return false;
		}
		it1++;it2++;
	}
	return true;
}


void EstimateFeature(istream & input, ostream& output, int max_n){
	string line;
	vector<Rule> rules;
	map<string,float> lhs_sum, rhs1_sum, rhs2_sum;
	while(input){
		line = "";
		getline(input,line);
		if(!line.length())
			break;
		// parse the line
		vector<string> entries;
		entries.reserve(5);
		algorithm::split_regex(entries,line,regex(" \\|\\|\\| "));
		if(entries.size()!=5){
			cerr << entries.size() << line << endl;
			continue;
		}
		Rule r;
		r.rhs_tgt = entries[0];
		r.rhs_src = entries[1];
		r.align = entries[2];
		r.weight = lexical_cast<float>(entries[3]);
		filter_alignment(r);
		if(!rules.size()){
			rules.push_back(r);
		}else{
			if(!(rules.back().rhs_tgt == r.rhs_tgt && r.align == rules.back().align)){
				list<s_vent> lhs_p = normalize(lhs_sum, max_n);
				list<s_vent> rhs1_p = normalize(rhs1_sum, max_n);
				list<s_vent> rhs2_p = normalize(rhs2_sum, max_n);
				string lhsf = output_result_one(lhs_p);
				string rhsf1 = output_result_one(rhs1_p);
				string rhsf2 = output_result_one(rhs2_p);
				string ctn = lhsf + " / " + rhsf1 + " / " + rhsf2;
				
				output_all(rules,ctn,output);
				rules.clear();
				lhs_sum.clear();
				rhs1_sum.clear();
				rhs2_sum.clear();
			}
			rules.push_back(r);
		}
		vector<string> es_entries;
		es_entries.reserve(3);
		algorithm::split_regex(es_entries,entries[4],regex("/"));
		split_words(es_entries[0],lhs_sum,r.weight);
		if(es_entries.size()>1)
			split_words(es_entries[1],rhs1_sum,r.weight);
		if(es_entries.size()>2)
			split_words(es_entries[2],rhs2_sum,r.weight);
	}
	list<s_vent> lhs_p = normalize(lhs_sum, max_n);
	list<s_vent> rhs1_p = normalize(rhs1_sum, max_n);
	list<s_vent> rhs2_p = normalize(rhs2_sum, max_n);
	string lhsf = output_result_one(lhs_p);
	string rhsf1 = output_result_one(rhs1_p);
	string rhsf2 = output_result_one(rhs2_p);
	string ctn = lhsf + "/ " + rhsf1 + "/ " + rhsf2;
	//output_all(rules,ctn,output);
	rules.clear();
}

void MergeFiles(istream& if1, istream& if2, ostream& os) {
	map<string,string> featMap;
	vector<string> v;
	v.reserve(2);
	cerr << "Loading feature mapping" << endl;
	int counter = 0;
	while(if1){
		counter++;
		string line;
		line = "";
		getline(if1,line);
		if(!line.length()){
			break;
		}

		algorithm::split_regex(v, line, regex("\\|\\|\\|"));
		if(v.size()!=2){
			cerr << "BAD feature file : " << line << endl;
			exit(1);
		}
		trim(v[0]);
		trim(v[1]);
		featMap.insert(pair<string,string>(v[0],v[1]));
		if(counter%10000==0){
			cerr << "\r[" << counter << "]";
		}
	}
	cerr << "\nDone" << endl;
	v.reserve(5);
	counter = 0;
	while(if2){
		counter++;
		string line;
		line = "";
		getline(if2,line);
		if(!line.length()){
			break;
		}
		v.resize(0);
		algorithm::split_regex(v, line, regex("\\|\\|\\|"));
		if(v.size()<4){
			cerr << "Bad line: " << line << endl;
			exit(1);
		}
		string vv = trim_copy(v[1]);
		string feat ;
		map<string,string>::iterator it = featMap.find(vv);
		if(it == featMap.end()){
			cerr << "Warning: Cannot find feature for " << vv << endl;
			feat = "G_S:1  / G_S:1  / G_S:1";
		}else{
			feat = it->second;
		}
		for(int i=0; i< 4; i++){
			trim(v[i]);
			if(v[i].length()){
				os<< v[i] << " ||| " ;
			}else{
				os<< v[i] << "||| " ;
			}
		}
		os  << feat << endl;
		if(counter%10000==0){
			cerr << "\r[" << counter << "]";
		}
	}
	cerr << endl;
}

//void MergeFiles(istream& if1, istream& if2, ostream& os) {
//	string line,line2,buffer_feat,buffer_align;
//	vector<Rule> rules;
//	map<string, float> lhs_sum, rhs1_sum, rhs2_sum;
//	while (if1&&if2) {
//		line = "";
//		line2 = "";
//		getline(if1, line);
//		getline(if2, line2);
//
//		if (!line.length())
//			break;
//		if (!line2.length())
//			break;
//		// parse the line
//		vector<string> entries,entries1;
//		entries.reserve(5);
//		algorithm::split_regex(entries, line, regex("\\|\\|\\|"));
//		if (entries.size() < 4) {
//			cerr << entries.size() << line << endl;
//			continue;
//		}
//		entries1.reserve(5);
//		algorithm::split_regex(entries1, line2, regex("\\|\\|\\|"));
//		if (entries1.size() != 4) {
//			cerr << entries.size() << line2 << endl;
//			continue;
//		}
//		trim(entries1[0]);
//		trim(entries1[1]);
//		trim(entries1[2]);
//		trim(entries1[3]);
//		trim(entries[0]);
//		trim(entries[1]);
//		trim(entries[2]);
//		trim(entries[3]);
//
//		if(entries1[0] != entries[0] || entries1[1] != entries[1]  ){
//			if(! compare_alignment(entries1[2], entries[3])){
//				// Try to swap
//				string line3,line4;
//				vector<string> ent3, ent4;
//				getline(if1, line3);
//				getline(if2, line4);
//				if (!line.length())
//					break;
//				if (!line2.length())
//					break;
//				ent3.reserve(5);
//				algorithm::split_regex(ent3, line3, regex("\\|\\|\\|"));
//				if (ent3.size() < 4) {
//					cerr << ent3.size() << line3 << endl;
//					continue;
//				}
//				ent4.reserve(5);
//				algorithm::split_regex(ent4, line4, regex("\\|\\|\\|"));
//				if (entries1.size() != 4) {
//					cerr << entries.size() << line4 << endl;
//					continue;
//				}
//				trim(ent3[0]);
//				trim(ent3[1]);
//				trim(ent3[2]);
//				trim(ent3[3]);
//				trim(ent4[0]);
//				trim(ent4[1]);
//				trim(ent4[2]);
//				trim(ent4[3]);
//				if(ent3[0] != entries[0] || ent3[1] != entries[1]){
//					cerr << "Cannot recover" << endl << line << endl << line2 << endl << line3 << endl << line4 << endl;
//					exit(1);
//				}
//				if(compare_alignment(entries1[2],ent3[3])){
//					os << line3 << " ||| " << entries1[3] << endl;
//				}else{
//					cerr << "Cannot recover" << endl << line << endl << line2 << endl << line3 << endl << line4 << endl;
//					exit(1);
//				}
//				if (compare_alignment(ent4[2], entries[3])) {
//					os << line << " ||| " << ent4[3] << endl << flush;
//				} else {
//					cerr << "Cannot recover" << endl << line << endl << line2
//							<< endl << line3 << endl << line4 << endl;
//					exit(1);
//				}
//			}else{
//				cerr << " Line Mismatch! " << endl << line << endl << line2 << endl;
//				cerr << entries1[2] << "|||" << entries[3] << endl << flush;
//				exit(1);
//			}
//		}
//		os << line << " ||| " << entries1[3] << endl << flush;
//	}
//}

int main(int argc, const char** argv){
	init_logging(0);
	BEGIN_PARAMETERS;
	REQ_PARAM(string, inputFile , "input,i", "Sorted input file, or previously output file with features (in merging)", "Input");
	REQ_PARAM(string, outputFile , "output,o", "Output file with features", "Output");
	DEF_PARAM(int, maxFrame,10,"max-frames,m","Maximum number of frames to keep","Options");
	DEF_PARAM(string, ruleFile,"","rule,r","Rule file to merge, if specified, will instead run consolidation","Input");
	END_PARAMETERS;
	PARSE_COMMANDLINE(argc,argv);

	ifstream ifs(inputFile.c_str());
	if(!ifs){
		P_FATAL("Cannot open %s file for input", inputFile.c_str());
	}
	ofstream ofs(outputFile.c_str());
	if(!ofs){
		P_FATAL("Cannot open %s file for output", outputFile.c_str());
	}

	if(ruleFile.length()){
		ifstream rfs(ruleFile.c_str());
		if(!rfs){
			P_FATAL("Cannot open %s file for input", ruleFile.c_str());
		}
		MergeFiles(ifs,rfs,ofs);
	}else{
		EstimateFeature(ifs,ofs,maxFrame);
	}
	

}
