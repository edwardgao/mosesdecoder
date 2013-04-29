/*
 * ruleextract.cc
 *
 *  Created on: Nov 7, 2010
 *      Author: qing
 */
#ifdef _MSC_VER
#pragma warning(disable : 4706)
#pragma warning(disable : 4996)
#pragma warning(disable : 4244)
#pragma warning(disable : 4305)
#pragma warning(disable : 4018)
#endif

#include <iostream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <string.h>

#include "porter_stemmer.h"


#include "aligninfo.h"
#include "srlinfo.h"
#include "ruleextract.h"
#include "assert.h"
#include "srlruleset.h"

namespace srl {

using namespace std;
using namespace boost;

inline bool overlap(int ss, int se, int ts, int te){return !(se<ts || te < ss);};

SortingSCFGRule::SortingSCFGRule(const SCFGRule& scfg, const SentencePair& sent){
	stringstream src, tgt, aln, alnr;
	for (size_t i = 0; i < scfg.rhs_source.size(); i++) {
		const SCFGToken& token = scfg.rhs_source[i];
		if (token.is_non_terminal) {
			//src << "[X][" << token.non_terminal_tag << "] ";
			// find corresponding target side non-terminal
			int r = *scfg.terminal_alignment[i].begin();
			const SCFGToken& rtoken = scfg.rhs_target[r];
			assert(rtoken.is_non_terminal);
			src << "[X][" << rtoken.non_terminal_tag << "] ";
		} else {
			src << sent.english().GetWordAtIndex(token.index_start) << " ";
		}
	}
	src << "[X]";

	for (size_t i = 0; i < scfg.rhs_target.size(); i++) {
		const SCFGToken& token = scfg.rhs_target[i];
		if (token.is_non_terminal) {
			tgt << "[X][" << token.non_terminal_tag << "] ";
		} else {
			tgt << sent.french().GetWordAtIndex(token.index_start) << " ";
		}
	}
	tgt << "[" << scfg.lhs_ << "]";

	for (size_t i = 0; i < scfg.terminal_alignment.size(); i++) {
		set<int>::const_iterator it = scfg.terminal_alignment[i].begin();
		for (; it != scfg.terminal_alignment[i].end(); it++) {
			aln << i << "-" << *it << " ";
			alnr << *it << "-" << i << " ";
		}
	}
	source_ = src.str();
	target_ = tgt.str();
	align_ = aln.str();
	align_reversed_ = alnr.str();
	weight_ = scfg.weight;

	// events
	for(set<string>::iterator it = scfg.events.begin(); it!=scfg.events.end();it++){
		event_string_ += *it;
		event_string_ += " ";
	}
	event_string_ += "/ ";
	for(set<string>::iterator it = scfg.nt_events_1.begin(); it!=scfg.nt_events_1.end();it++){
		event_string_ += *it;
		event_string_ += " ";
	}
	event_string_ += "/ ";
	for(set<string>::iterator it = scfg.nt_events_2.begin(); it!=scfg.nt_events_2.end();it++){
		event_string_ += *it;
		event_string_ += " ";
	}

}

pair<string,string> SerializeSortedSCFGRule(const SortingSCFGRule& scfg, int sentence, const char* accessory){
	stringstream output, outputinv;

	output << scfg.source() << " ||| " << scfg.target() << " ||| " << scfg.align() << "||| " << scfg.weight();
	outputinv << scfg.target() << " ||| " << scfg.source() << " ||| " << scfg.align_reversed() << "||| " << scfg.weight();

	if (sentence > -1) {
		output << " ||| " << sentence;
		outputinv << " ||| " << sentence;
	}
	if (accessory) {
		output << " ||| " << accessory;
		outputinv << " ||| " << accessory;
	}

	return pair<string, string> (output.str(), outputinv.str());

}

pair<string, string> SerializeSCFGRule(const SCFGRule& scfg,
		const SentencePair& sent, int sentence, const char* accessory) {
	stringstream output, outputinv;
	for (size_t i = 0; i < scfg.rhs_source.size(); i++) {
		const SCFGToken& token = scfg.rhs_source[i];
		if (token.is_non_terminal) {
			output << "[X][" << token.non_terminal_tag << "] ";
		} else {
			output << sent.english().GetWordAtIndex(token.index_start) << " ";
		}
	}
	output << "[X] ||| ";

	for (size_t i = 0; i < scfg.rhs_target.size(); i++) {
		const SCFGToken& token = scfg.rhs_target[i];
		if (token.is_non_terminal) {
			output << "[X][" << token.non_terminal_tag << "] ";
			outputinv << "[X][" << token.non_terminal_tag << "] ";
		} else {
			output << sent.french().GetWordAtIndex(token.index_start) << " ";
			outputinv << sent.french().GetWordAtIndex(token.index_start) << " ";
		}
	}
	output << "[" << scfg.lhs_ << "] ||| ";
	outputinv << "[" << scfg.lhs_ << "] ||| ";

	for (size_t i = 0; i < scfg.rhs_source.size(); i++) {
		const SCFGToken& token = scfg.rhs_source[i];
		if (token.is_non_terminal) {
			outputinv << "[X][" << token.non_terminal_tag << "] ";
		} else {
			outputinv << sent.english().GetWordAtIndex(token.index_start)
					<< " ";
		}
	}
	outputinv << "[X] ||| ";

	for (size_t i = 0; i < scfg.terminal_alignment.size(); i++) {
		set<int>::const_iterator it = scfg.terminal_alignment[i].begin();
		for (; it != scfg.terminal_alignment[i].end(); it++) {
			output << i << "-" << *it << " ";
			outputinv << *it << "-" << i << " ";
		}
	}
	output << "||| " << scfg.weight;
	outputinv << "||| " << scfg.weight;

	if (sentence > -1) {
		output << " ||| " << sentence;
		outputinv << " ||| " << sentence;
	}
	if (accessory) {
		output << " ||| " << accessory;
		outputinv << " ||| " << accessory;
	}

	return pair<string, string> (output.str(), outputinv.str());
}

pair<string, string> SerializeSCFGRule_PhraseOnly(const SCFGRule& scfg,
		const SentencePair& sent, int sentence, const char* accessory) {
	stringstream output, outputinv;
	for (size_t i = 0; i < scfg.rhs_source.size(); i++) {
		const SCFGToken& token = scfg.rhs_source[i];
		output << sent.english().GetWordAtIndex(token.index_start) << " ";
	}
	output << "||| ";

	for (size_t i = 0; i < scfg.rhs_target.size(); i++) {
		const SCFGToken& token = scfg.rhs_target[i];
		if (token.is_non_terminal) {
			output  << token.non_terminal_tag << " ";
			outputinv << token.non_terminal_tag << " ";
		} else {
			output << sent.french().GetWordAtIndex(token.index_start) << " ";
			outputinv << sent.french().GetWordAtIndex(token.index_start) << " ";
		}
	}
	output << "||| ";
	outputinv << "||| ";

	for (size_t i = 0; i < scfg.rhs_source.size(); i++) {
		const SCFGToken& token = scfg.rhs_source[i];
		if (token.is_non_terminal) {
			outputinv  << token.non_terminal_tag << " ";
		} else {
			outputinv << sent.english().GetWordAtIndex(token.index_start)
					<< " ";
		}
	}
	outputinv << "||| ";

	for (size_t i = 0; i < scfg.terminal_alignment.size(); i++) {
		set<int>::const_iterator it = scfg.terminal_alignment[i].begin();
		for (; it != scfg.terminal_alignment[i].end(); it++) {
			output << i << "-" << *it << " ";
			outputinv << *it << "-" << i << " ";
		}
	}
//	output << "||| " << scfg.weight;
//	outputinv << "||| " << scfg.weight;

	if (sentence > -1) {
		output << "||| " << sentence;
		outputinv << "||| " << sentence;
	}
	if (accessory) {
		output << " ||| " << accessory;
		outputinv << " ||| " << accessory;
	}

	return pair<string, string> (output.str(), outputinv.str());
}

pair<string,string> SerializeUncovered_PhraseOnly(const string& source, const string& target){
	string src = source + " ||| " + target + " ||| 0-0 ";
	string tgt = target + " ||| " + source + " ||| 0-0 ";
	return pair<string,string>(src,tgt);
}
pair<string,string> SerializeUncovered(const string& source, const string& target, float weight){
	stringstream output, outputinv;
	output << source << " [X] ||| " << target << " [X] ||| 0-0 ||| " << weight;
	outputinv << target << " [X] ||| " << source << " [X] ||| 0-0 ||| " << weight;
	return pair<string,string>(output.str(),outputinv.str());

}



RuleExtractor::sorted_iterator RuleExtractor::sorted_rule_begin() {
	return this->sorted_rules_.begin();
}
RuleExtractor::sorted_iterator RuleExtractor::sorted_rule_end() {
	return this->sorted_rules_.end();
}


RuleExtractor::sorted_const_iterator RuleExtractor::sorted_rule_begin() const {
	return this->sorted_rules_.begin();
}
RuleExtractor::sorted_const_iterator RuleExtractor::sorted_rule_end() const {
	return this->sorted_rules_.end();
}



RuleExtractor::iterator RuleExtractor::filtered_rule_begin() {
	return this->filtered_rules_.begin();
}
RuleExtractor::iterator RuleExtractor::filtered_rule_end() {
	return this->filtered_rules_.end();
}


RuleExtractor::const_iterator RuleExtractor::filtered_rule_begin() const {
	return this->filtered_rules_.begin();
}
RuleExtractor::const_iterator RuleExtractor::filtered_rule_end() const {
	return this->filtered_rules_.end();
}


RuleExtractor::iterator RuleExtractor::rule_begin() {
	return this->subst_rules_.begin();
}
RuleExtractor::iterator RuleExtractor::rule_end() {
	return this->subst_rules_.end();
}


RuleExtractor::const_iterator RuleExtractor::rule_begin() const {
	return this->subst_rules_.begin();
}
RuleExtractor::const_iterator RuleExtractor::rule_end() const {
	return this->subst_rules_.end();
}

RuleExtractor::const_iterator RuleExtractor::initial_phrase_begin() const {
	return this->initial_phrases_.begin();
}
RuleExtractor::const_iterator RuleExtractor::initial_phrase_end() const {
	return this->initial_phrases_.end();
}

RuleExtractor::iterator RuleExtractor::initial_phrase_begin() {
	return this->initial_phrases_.begin();
}
RuleExtractor::iterator RuleExtractor::initial_phrase_end() {
	return this->initial_phrases_.end();
}

RuleExtractor::const_iterator RuleExtractor::ghkm_rule_begin() const {
	return this->initial_ghkm_rules_.begin();
}
RuleExtractor::const_iterator RuleExtractor::ghkm_rule_end() const {
	return this->initial_ghkm_rules_.end();
}

RuleExtractor::iterator RuleExtractor::ghkm_rule_begin() {
	return this->initial_ghkm_rules_.begin();
}
RuleExtractor::iterator RuleExtractor::ghkm_rule_end() {
	return this->initial_ghkm_rules_.end();
}

RuleExtractor::RuleExtractor() {
	initial_flag_ = EITArgumentCombination;
	length_initial_phrase_ = 10;
	max_tokens_ = 5;
	bypass_ghkm_ = false;
	bypass_hiero_ = false;
	disallow_unaligned_on_boundary_ = true;
	max_tokens_no_terminal_ = 7;
	track_uncovered_words_= false;
	skip_stemming_ = false;
	stemmer = create_stemmer();
	min_gap_source_ = 1;
	min_gap_target_ = 0;
	output_all_text_ = true;
}

RuleExtractor::~RuleExtractor() {
	free_stemmer(stemmer);
}

pair<list<SRLGenRule>, list<SRLGenStruct> >  RuleExtractor::DoExtractSRLGenerationRules(const SentencePair& sent,
		const Alignment& alignment, const vector<SRLInformation>& srl, int sent_number, int max_expanded,
		int max_words) {
	this->sent_ = &sent;
	this->align_ = &alignment;
	this->srl_ = &srl;
	this->sent_number_ = sent_number;
	// Sequence:
	DoInitialization();
	DoMatchSRLInformation();

	vector<vector<string> >& tags = mapped_srl_tags_;
	vector<vector<pair<int, int> > >& regions = mapped_srl_regions_;
	vector<bool>& valid = srl_info_valid_;

	list<SRLGenRule> rules;
	list<SRLGenStruct> structs;
	vector<string> frames;

	for(int i=0; i< (int) srl.size(); i++){
		if(!valid[i]) continue;

		vector<pair<int,int> > cregion = regions[i];
		vector<string>& ctag = tags[i];
		SRLGenStruct nl;
		for (int j = 0; j < (int) cregion.size(); j++) {
			if(ctag[j]=="TARGET"){
				pair<int,int> region = srl[i].GetRegionAtIndex(j);
				string frame = to_lower_copy(srl[i].GetWordAtIndex(region.first));
				if(!this->skip_stemming_)
					nl.frame = this->do_stemming(frame);
				else
					nl.frame = frame;
				frames.push_back(nl.frame);
			}
			nl.arguments.push_back(ctag[j]);
		}
		if(nl.arguments.size()>1){
			structs.push_back(nl);
		}
	}

	if(frames.size()<srl.size()){
		rules.clear();
		structs.clear();
		return pair<list<SRLGenRule>, list<SRLGenStruct> > (rules,structs);
	}


	for (int i = 0; i < (int) tags.size(); i++) {
		vector<pair<int,int> > cregion = regions[i];
		if (!valid[i])
			continue;
		for(int j =0 ; j < (int) cregion.size();j++){

			vector<string>& ctag = tags[i];
			set<int> target_words, source_words;
			for(int k = cregion[j].first; k<= cregion[j].second; k++){
				target_words.insert(k);
			}
			int originalsize =(int) target_words.size();
			int lastsize = originalsize;
			// Find all sourcd words
			do{
				lastsize = target_words.size();
				for (set<int>::iterator it = target_words.begin(); it
						!= target_words.end(); it++) {
					const set<int>& al = alignment.AlignedToFrench(*it);
					source_words.insert(al.begin(),al.end());
				}
				for (set<int>::iterator it = source_words.begin(); it
						!= source_words.end(); it++) {
					const set<int>& al = alignment.AlignedToEnglish(*it);
					target_words.insert(al.begin(),al.end());
				}
			}while((int)target_words.size()<lastsize);

			set<int> exp = source_words;
			// Add internal source word, if it is not aligned
			for (set<int>::iterator it = exp.begin(); it != exp.end(); it++) {
				int left = *it -1;
				int right = *it +1;
				while(left>=0){
					if(source_words.find(left)!=source_words.end()){
						break;
					}
					if(alignment.AlignedToEnglish(left).size()==0){
						source_words.insert(left--);
					}else{
						break;
					}
				}

				while (right < (int)sent.english().length()) {
					if (source_words.find(right) != source_words.end()) {
						break;
					}
					if (alignment.AlignedToEnglish(right).size() == 0) {
						source_words.insert(right++);
					} else {
						break;
					}
				}
			}

			if((int)target_words.size()-originalsize <= max_expanded && (int)target_words.size()< max_words && (int)source_words.size()<max_words){
				string src,tgt;

				rules.push_back(SRLGenRule());
				rules.back().frame = frames[i];
				rules.back().argument = ctag[j];
				for (set<int>::iterator it = target_words.begin(); it != target_words.end(); it++) {
					rules.back().target.push_back(sent.french().GetWordAtIndex(*it));
				}
				for (set<int>::iterator it = source_words.begin(); it != source_words.end(); it++) {
					rules.back().source.push_back(sent.english().GetWordAtIndex(*it));
				}
			}
		}
	}
	return pair<list<SRLGenRule>, list<SRLGenStruct> > (rules,structs);
}

list<SRLGenRuleWithFeature > RuleExtractor::DoSentenceShortening(const SentencePair& sent,
		const Alignment& alignment, const vector<SRLInformation>& srl, int sent_number, int max_expanded,
		int min_words,	SRLGenRuleSet* ruleset,	int max_replacement) {
	this->sent_ = &sent;
	this->align_ = &alignment;
	this->srl_ = &srl;
	this->sent_number_ = sent_number;
	// Sequence:
	DoInitialization();
	DoMatchSRLInformation();

	vector<vector<string> >& tags = mapped_srl_tags_;
	vector<vector<pair<int, int> > >& regions = mapped_srl_regions_;
	vector<bool>& valid = srl_info_valid_;
	list<SRLGenRuleWithFeature > ret;

	for (int i = 0; i < (int) tags.size(); i++) {
		if (!valid[i])
			continue;
		vector<string>& ctag = tags[i];
		vector<pair<int, int> >& cregion = regions[i];
		set<int> target_words, source_words;
		for(int j = 0; j < (int) cregion.size();j++){
			for(int k = cregion[j].first; k<= cregion[j].second; k++){
				target_words.insert(k);
			}
		}
		int originalsize =(int) target_words.size();
		int lastsize = originalsize;
		// Find all sourcd words
		do{
			lastsize = target_words.size();
			for (set<int>::iterator it = target_words.begin(); it
					!= target_words.end(); it++) {
				const set<int>& al = alignment.AlignedToFrench(*it);
				source_words.insert(al.begin(),al.end());
			}
			for (set<int>::iterator it = source_words.begin(); it
					!= source_words.end(); it++) {
				const set<int>& al = alignment.AlignedToEnglish(*it);
				target_words.insert(al.begin(),al.end());
			}
		}while((int)target_words.size()<lastsize);

		set<int> exp = source_words;
		// Add internal source word, if it is not aligned
		for (set<int>::iterator it = exp.begin(); it != exp.end(); it++) {
			int left = *it -1;
			int right = *it +1;
			while(left>=0){
				if(source_words.find(left)!=source_words.end()){
					break;
				}
				if(alignment.AlignedToEnglish(left).size()==0){
					source_words.insert(left--);
				}else{
					break;
				}
			}

			while (right < (int)sent.english().length()) {
				if (source_words.find(right) != source_words.end()) {
					break;
				}
				if (alignment.AlignedToEnglish(right).size() == 0) {
					source_words.insert(right++);
				} else {
					break;
				}
			}
		}

		if(ruleset){
			for(int j=0; j<(int)ctag.size();j++){
				if(ctag[j]=="TARGET") continue;  // should we enable it?
				list<SRLGenRuleWithFeature > new_sent= replace_one_argument(i,j,ruleset, max_expanded,min_words);
				list<SRLGenRuleWithFeature >::iterator it11=ret.end();
				ret.insert(it11,new_sent.begin(),new_sent.end());
			}
		}

	}

	return ret;
}

bool RuleExtractor::DoExtraction(const SentencePair& sent,
		const Alignment& alignment, const vector<SRLInformation>& srl, int sent_number) {
	this->sent_ = &sent;
	this->align_ = &alignment;
	this->srl_ = &srl;
	this->sent_number_ = sent_number;
	// Sequence:
	DoInitialization();
	DoMatchSRLInformation();
	DoInitalPhraseExtraction();
	if(!bypass_ghkm())
		DoGHKMPhraseExtraction();
	if(!bypass_hiero())
		DoHieroSubstitution();
	DoFilterRule();
	if(extract_events())
		ApplyEventStrings();
	if(!bypass_hiero())
		DoSortAndUniq();
	return false;
}

list<string> RuleExtractor::DoMonolingualSRLExtraction (const string& sent, const std::vector<SRLInformation>& srl, int contextLen) {
	this->srl_ = &srl;
	SentencePair * p;
	this->sent_ = p = new SentencePair();
	vector<string> res;
	split(res,sent,is_any_of(" \t"), token_compress_on);
	p->french_.words_ = res;
	

	list<string> ret;
	DoInitialization();
	DoMatchSRLInformation();
	DoOutputSRLConstituents(ret, contextLen);
	return ret;
}


void RuleExtractor::RunDaemon(queue<RuleExtractor::Task>& tasks, queue<RuleExtractor::TaskResult>& results,
boost::mutex& tasks_mutex, boost::mutex& results_mutex, bool& quit_flag){
	while(!quit_flag){
		RuleExtractor::Task task ;
		{
			boost::mutex::scoped_try_lock tasks_lock(tasks_mutex);
			if(!tasks_lock){
				continue;
			}
			if(!tasks.empty()){
				task = tasks.front();
				tasks.pop();
			}else{
				continue;
			}
		}
		DoExtraction(task.sent,task.align, task.srl,task.sent_number);
		{
			boost::mutex::scoped_lock results_lock(results_mutex);
			if(this->bypass_hiero_){
				RuleExtractor::const_iterator it;
				it = initial_phrase_begin();
				for (; it != initial_phrase_end(); it++) {
					pair<string,string> formated = SerializeSCFGRule_PhraseOnly(*it,task.sent);
					RuleExtractor::TaskResult res;
					res.output_str = formated.first;
					res.output_inv = formated.second;
					res.sent_number = task.sent_number;
					results.push(res);
				}
			}else{
				RuleExtractor::sorted_const_iterator it;
				it = sorted_rule_begin();
				for (; it != sorted_rule_end(); it++) {
					pair<string,string> formated = SerializeSortedSCFGRule(*it);
					RuleExtractor::TaskResult res;
					res.output_str = formated.first;
					res.output_inv = formated.second;
					res.sent_number = task.sent_number;
					results.push(res);
				//	cerr << results.size() << endl;
				}
			}
		}

	}
}

void RuleExtractor::DoInitialization() { // Clean the intermediate data during last extraction
	initial_phrases_.clear();
	initial_ghkm_rules_.clear();
	subst_rules_.clear();
	filtered_rules_.clear();
	sorted_rules_.clear();
	mapped_srl_tags_.clear();
	mapped_srl_regions_.clear();
	srl_info_valid_.clear();
}

void RuleExtractor::DoMatchSRLInformation() { // Match SRL labels with the sentence
	const Sentence& fSent = sent_->french();
	const vector<SRLInformation>& srl = *srl_;
	vector<vector<string> >& tags = mapped_srl_tags_;
	vector<vector<pair<int,int> > >& regions=  mapped_srl_regions_;
	vector<bool>& valid = srl_info_valid_;


	//cerr << "SRLSize" << srl.size();
	tags.resize(srl.size());
	predicate_word_indice_.clear();
	predicate_segment_indice_.clear();
	predicate_word_indice_.resize(srl.size());
	predicate_segment_indice_.resize(srl.size());
	regions.resize(srl.size());
	valid.resize(srl.size(),true);
	srl_frames_.resize(srl.size());

	vector<string> sentlc;
	sentlc.resize(fSent.length());

	int nwordori = fSent.length();
	for(int i=0; i< nwordori; i++){
		sentlc[i] = to_lower_copy(fSent.GetWordAtIndex(i));
		//cerr << sentlc[i] << " " ;
	}
	//cerr << endl;

	for(int i=0; i < (int)srl.size();i++){
		vector<string>& ctag = tags[i];
		vector<pair<int,int> >& cregion = regions[i];
		vector<string> srlsentlc;

		const SRLInformation& csrl = srl[i];
		int nlabel = (int)csrl.GetNumberOfLabels();
		int nword = (int) csrl.GetNumberOfWords();
		if(nlabel==0 || nword==0){
			valid[i]==false;
			continue;
		}
		ctag.resize(nlabel);
		srlsentlc.resize(nword);
		for(int j=0; j < nlabel; j++){
			ctag[j] = csrl.GetLabelAtIndex(j);
			if(ctag[j] == "TARGET"){
				int frameidx = csrl.GetRegionAtIndex(j).first;
				srl_frames_[i] = "";
				for(;frameidx < csrl.GetRegionAtIndex(j).second;frameidx++){
					if(!skip_stemming_){
						srl_frames_[i] += this->do_stemming(csrl.GetWordAtIndex(frameidx));
					}else{
						srl_frames_[i] += csrl.GetWordAtIndex(frameidx);
					}
					if(frameidx < csrl.GetRegionAtIndex(j).second-1){
						srl_frames_[i] += "+";
					}
				}
//				predicate_word_indice_[i].first = csrl.GetRegionAtIndex(j).first;
//				predicate_word_indice_[i].second = csrl.GetRegionAtIndex(j).second-1;
				predicate_segment_indice_[i] = j;
			}
		}
		for(int j=0; j<nword;j++){
			srlsentlc[j] = to_lower_copy(csrl.GetWordAtIndex(j));
			//cerr << srlsentlc[j] << " ";
		}
		//cerr << endl;

		// map words, assumption is SRL only kick out words, never add words
		int j,k=0;
		vector<int> index_map;
		index_map.resize(nword);

		for(j =0; j< nword && k < nwordori; j++,k++){
			int orik= k;
			while(k<nwordori){
				if(srlsentlc[j] == sentlc[k]){
					index_map[j] = k;
					//cerr << "  " << j << "," << k << endl;
					break;
				}
				k++;
			}
			if(k==nwordori){
				cerr << "Found an unmatched SRL alignment, trying to recover " << endl;

				string sword = "";
				int orj = j;
				while(j<nword){
					sword += srlsentlc[j];
					if(sword > sentlc[orik]){
						j = nword;
						break;
					}
					if(sword == sentlc[orik]){
						for(int l = orj; l <= j; l++){
							index_map[l] = orik;
						}
						k = orik;
						break;
					}
					j++;
				}
				if(j==nword){
					cerr << "Cannot recover, below is the sentence";
					for(int k = 0; k < (int)srlsentlc.size(); k++)
						cerr << srlsentlc[k] << " ";
					cerr << endl;
					valid[i] = false;
					break;

				}
			}
		}
		if(!valid[i])
			continue;
		cregion.resize(nlabel);
		for(j=0; j< nlabel ; j++){
			const pair<int,int>& rg = csrl.GetRegionAtIndex(j);
			cregion[j].first = index_map[rg.first];
			cregion[j].second = index_map[rg.second-1];
			if(j==predicate_segment_indice_[i]){
				predicate_word_indice_[i].first = cregion[j].first;
				predicate_word_indice_[i].second = cregion[j].second;
			}
		}


	}
}

void RuleExtractor::DoInitalPhraseExtraction() { // Do initial extraction based on Franz Och standard
	int startE, startF, endE, endF; // The indice of start and end indices of the phrase, inclusive
	const Sentence& eSent = sent_->english();
	const Sentence& fSent = sent_->french();
	const Alignment& align = *align_;
	for (startE = 0; startE < (int)eSent.length(); startE++) {
		int eBound = startE + length_initial_phrase_;
		eBound = eBound < (int)eSent.length() ? eBound : eSent.length(); // eBound is exclusive
		int min_aligned = fSent.length();
		int max_aligned = -1; // Max and min aligned words
		for (endE = startE; endE < eBound; endE++) {
			const set<int>& a2e = align.AlignedToEnglish(endE);
			if (a2e.size() > 0) {
				int new_min_aligned = *(a2e.begin());
				int new_max_aligned = *(a2e.rbegin());
				if (new_min_aligned < min_aligned)
					min_aligned = new_min_aligned;
				if (new_max_aligned > max_aligned)
					max_aligned = new_max_aligned;
			}
			// Early termination if the span is either too large or no alignment link
			if (max_aligned - min_aligned - 1 > length_initial_phrase_
					|| max_aligned == -1 || min_aligned >= (int)fSent.length())
				continue;
			int startFLow = min_aligned - length_initial_phrase_;
			startFLow = startFLow >=0 ? startFLow : 0;
			int empUpper = -1;
			for (startF = startFLow; startF < (int)fSent.length(); startF++) {
				// Another early termination possiblity: if startF is aligned to some word, but out
				// of the boundary of min/max aligned, we continue
				if(startF > min_aligned) break;
				if (startF < min_aligned && align.AlignedToFrench(startF).size())
					continue;
				int fBound = startF + length_initial_phrase_;
				fBound = fBound < (int)fSent.length() ? fBound : fSent.length();
				int fBoundLow = max_aligned; // should cover all the aligned word
				int terminateAtE = -1;
				for (endF = fBoundLow; endF < fBound; endF++) { //
					for (int i = startF; i <= endF; i++) {
						const set<int>& a2f = align.AlignedToFrench(i);
						if (a2f.size() && (*(a2f.begin()) < startE
								|| *(a2f.rbegin()) > endE)) {
							terminateAtE = i;
							empUpper = i;
							break; // Don't need to go on
						}
					}//for (endF = fBoundLow; endF < fBound; endF++)
					if (terminateAtE > -1)
						break;
					// If reached here, a valid phrase is formed
					add_initial_phrase(startE, startF, endE, endF);
				}
			} //for(startF = 0; startF < fSent.length(); startF++){
		} // for(endE = startE ; endE < eBound ; endE ++)
	}//for(startE = 0 ; startE < eSent.length() ; startE++)
}

void RuleExtractor::DoGHKMPhraseExtraction() { // Do GHKM phrase extraction on SRL labels
	/*
	 * The algorithm: For every set of SRL label, do the following
	 * S1. Pick a set of SRL regions, get their names
	 * S2. Find a phrase pair that covers the region and optionally some words not in any region
	 * S3. Add them to GHKM rules
	 * The predicate argument structure must contain at least the predicate
	 */
	vector<vector<string> >& tags = mapped_srl_tags_;
	vector<vector<pair<int,int> > >& regions=  mapped_srl_regions_;
	vector<bool>& valid = srl_info_valid_;


	for(int i=0; i < (int)tags.size();i++){
		if(!valid[i]) continue;
		vector<string>& ctag = tags[i];
		vector<pair<int,int> >& cregion = regions[i];
		int startR, endR;
		string tagname;
		for(startR = 0; startR < (int)ctag.size() ; startR ++){
			for(endR = startR ; endR < (int)ctag.size(); endR++){
				if(startR> this->predicate_segment_indice_[i] || endR < this->predicate_segment_indice_[i]){
					continue; // contain the predicate.
				}
				tagname = build_srl_tag_name(ctag,srl_frames_[i],startR, endR);
				find_and_add_ghkm_phrase(cregion,startR, endR, tagname,i);
			}
		}
		tagname = build_srl_tag_name(ctag,srl_frames_[i],0,ctag.size()-1);
		completed_srl_structures_.insert(tagname);
	}
}

void RuleExtractor::DoHieroSubstitution() { // Do Hiero substition based on David Chiang Standard
	list<SCFGRule>::const_iterator it = initial_phrases_.begin();
	list<SCFGRule> substitued_rules;

	// Replacing GHKM rules inside initial phrases are forbidden
	for(; it != initial_phrases_.end() ; it++){
		if(this->disallow_unaligned_on_boundary_ && !it->boudnary_aligned)
			continue;
		list<SCFGRule*> feasible; // All the feasible rules
		// For each initial phrase we try to replace
		list<SCFGRule>::iterator itp = initial_phrases_.begin();
		for(; itp != initial_phrases_.end() ; itp++){
			if((this->disallow_unaligned_on_boundary_ && !it->boudnary_aligned) || itp == it)
				continue;
			// determine if they are feasible parent/child
			if(is_feasible_sub_phrase(*it, *itp))
				feasible.push_back(&(*itp));
		}

		// Now try to substitute one
		list<SCFGRule*>::iterator itq = feasible.begin();
		list<SCFGRule> substitued_rules_one;
		for(;itq != feasible.end(); itq++){
			try_substitute_one_sub_phrase(*it, **itq,substitued_rules_one);
		}
		list<SCFGRule> substitued_rules_two = substitued_rules_one;
		list<SCFGRule>::const_iterator it2 = substitued_rules_two.begin();
		for(; it2 != substitued_rules_two.end() ; it2++){
			if(this->disallow_unaligned_on_boundary_ && !it2->boudnary_aligned)
				continue;
			// Now try to substitute another one
			list<SCFGRule*>::iterator itq = feasible.begin();
			for(;itq != feasible.end(); itq++){
				if(is_feasible_sub_phrase(*it2,**itq))
					try_substitute_one_sub_phrase(*it2, **itq,substitued_rules_one);
			}
		}
		// Now add all the rules
		float total_new = substitued_rules_one.size()+1;
		float new_weight = it->weight / total_new;
		subst_rules_.push_back(*it);
		subst_rules_.back().weight = new_weight;
		for(list<SCFGRule>::iterator iit = substitued_rules_one.begin(); iit!= substitued_rules_one.end();iit++){
			subst_rules_.push_back(*iit);
			subst_rules_.back().weight = new_weight;
		}
	}


	it = initial_ghkm_rules_.begin();
	/* But it is OK to replace initial phrases inside GHKM phrases, given that it does not eliminate the predicate region
	 * also, the indice of the SRL structure should be the same when replacing GHKM phrases
	 *
	 */
	for(; it != initial_ghkm_rules_.end() ; it++){
//		if(this->disallow_unaligned_on_boundary_ && !it->boudnary_aligned)
//			continue;
		list<SCFGRule*> feasible; // All the feasible rules
		// For each initial phrase we try to replace
		list<SCFGRule>::iterator itp = initial_phrases_.begin();
		for(; itp != initial_phrases_.end() ; itp++){
			if((this->disallow_unaligned_on_boundary_ && !it->boudnary_aligned) || itp == it)
				continue;
			// determine if they are feasible parent/child
			if(is_feasible_sub_phrase(*it, *itp))
				feasible.push_back(&(*itp));
		}

		itp = initial_ghkm_rules_.begin();
		for (; itp != initial_ghkm_rules_.end(); itp++) {
//			if ((this->disallow_unaligned_on_boundary_ && !it->boudnary_aligned)
//					|| itp == it)
//				continue;
			// determine if they are feasible parent/child
			if (is_feasible_sub_phrase(*it, *itp))
				feasible.push_back(&(*itp));
		}

		// Now try to substitute one
		list<SCFGRule*>::iterator itq = feasible.begin();
		list<SCFGRule> substitued_rules_one;
		for(;itq != feasible.end(); itq++){
			try_substitute_one_sub_phrase(*it, **itq,substitued_rules_one);
		}
		list<SCFGRule> substitued_rules_two = substitued_rules_one;
		list<SCFGRule>::const_iterator it2 = substitued_rules_two.begin();
		for(; it2 != substitued_rules_two.end() ; it2++){
//			if(this->disallow_unaligned_on_boundary_ && !it2->boudnary_aligned)
//				continue;
			// Now try to substitute another one
			list<SCFGRule*>::iterator itq = feasible.begin();
			for(;itq != feasible.end(); itq++){
				if(is_feasible_sub_phrase(*it2,**itq))
					try_substitute_one_sub_phrase(*it2, **itq,substitued_rules_one);
			}
		}
		// Now add all the rules
		float total_new = substitued_rules_one.size()+1;
		float new_weight = it->weight / total_new;
		subst_rules_.push_back(*it);
		subst_rules_.back().weight = new_weight;
		for(list<SCFGRule>::iterator iit = substitued_rules_one.begin(); iit!= substitued_rules_one.end();iit++){
			subst_rules_.push_back(*iit);
			subst_rules_.back().weight = new_weight;
		}
	}


}

void RuleExtractor::DoOutputSRLConstituents(list<string>& output, int contextLen){// Output SRL constituents (of monolingual 
	output.clear();
	string buffer;
	for(size_t i = 0 ; i < srl_frames_.size(); i++){
		string& frame = srl_frames_[i];
		if(!this->srl_info_valid_[i])
			continue;

		for(size_t j = 0 ; j < mapped_srl_tags_[i].size() ; j ++){
			string& role = mapped_srl_tags_[i][j];
			buffer = frame + " ||| " + role + " ||| ";
			for(size_t k  = (size_t) mapped_srl_regions_[i][j].first; k<= (size_t) mapped_srl_regions_[i][j].second  ; k++ ){
				buffer+=sent_->french().GetWordAtIndex(k);
				buffer += " ";
			}
			buffer += "||| ";
			int s =  mapped_srl_regions_[i][j].first -  contextLen;
			s = s < 0 ? 0 : s;
			for(int k = s ; k <  mapped_srl_regions_[i][j].first ; k++){
				buffer += sent_->french().GetWordAtIndex(k);
				buffer += " ";
			}
			buffer += "||| ";
			s =  mapped_srl_regions_[i][j].second +  contextLen;
			s = s >= sent_->french().length() ? sent_->french().length() - 1: s;
			for(int k = mapped_srl_regions_[i][j].second + 1; k <= s; k++){
				buffer += sent_->french().GetWordAtIndex(k);
				buffer += " ";
			}

			output.push_back(buffer);
		}
	}
}

void RuleExtractor::DoFilterRule() { // Filter out the rules
	vector<bool> word_coverage;
	if(track_uncovered_words_)
		word_coverage.resize(sent_->english().length(),false);
	for(list<SCFGRule>::iterator it = subst_rules_.begin(); it!= subst_rules_.end();it++){
		if(it->num_non_terminal&& (int)it->rhs_source.size()> this->max_tokens_) continue;
		if((!it->num_non_terminal)&&
				((int)it->rhs_source.size()> this->max_tokens_no_terminal_
						||(int)it->rhs_target.size()> this->max_tokens_no_terminal_)) continue;

		if (it->num_non_terminal > 1) {
			int first = -1, second = -1;
			for (int i = 0; i < (int) it->rhs_source.size(); i++) {
				if (it->rhs_source[i].is_non_terminal) {
					if (first == -1)
						first = it->rhs_source[i].index_end;
					else
						second = it->rhs_source[i].index_start;
				}
			}
			if (second - first <= min_gap_source_)
				continue;
			first = -1, second = -1;
			for (int i = 0; i < (int) it->rhs_target.size(); i++) {
				if (it->rhs_target[i].is_non_terminal) {
					if (first == -1)
						first = it->rhs_target[i].index_end;
					else
						second = it->rhs_target[i].index_start;
				}
			}
			if (second - first <= min_gap_target_)
				continue;
		}
		if(track_uncovered_words_){
			for(int i = 0 ; i < (int)it->rhs_source.size();i++){
				if(!it->rhs_source[i].is_non_terminal){
					word_coverage[it->rhs_source[i].index_start] = true;
				}
			}
		}
		this->filtered_rules_.push_back(*it);
	}
	if(track_uncovered_words_){
		// remove covered words, add uncovered words
		for(int i =0 ; i< (int) word_coverage.size(); i++){
			if(word_coverage[i]){
				map<string,map<string, float> >::iterator it =
						uncovered_words_.find(sent_->english().GetWordAtIndex(i));
				if(it!=uncovered_words_.end()){
					uncovered_words_.erase(it);
				}
				covered_words_.insert(sent_->english().GetWordAtIndex(i));
			}else{
				set<string>::const_iterator it =
						covered_words_.find(sent_->english().GetWordAtIndex(i));
				if(covered_words_.end() == it){
					map<string,map<string, float> >::iterator it1 =
							uncovered_words_.find(sent_->english().GetWordAtIndex(i));
					if(it1==uncovered_words_.end()){
						uncovered_words_[sent_->english().GetWordAtIndex(i)] = map<string,float>();
						it1 =uncovered_words_.find(sent_->english().GetWordAtIndex(i));
					}
					for(set<int>::const_iterator it2 = align_->AlignedToEnglish(i).begin();
							it2 != align_->AlignedToEnglish(i).end(); it2++){
						const string& target = sent_->french().GetWordAtIndex(*it2);
						map<string, float>::iterator it3 = it1->second.find(target);
						if(it3==it1->second.end())
							it1->second[target] = 1;
						else
							it3->second +=1;
					}
				}
			}
		}
	}
}

void RuleExtractor::DoSortAndUniq() { // Filter out the rules
	list<SortingSCFGRule> rules;
	for(list<SCFGRule>::iterator it = filtered_rules_.begin(); it!= filtered_rules_.end();it++){
		rules.push_back(SortingSCFGRule(*it, *sent_));
	}
	rules.sort();

	for(list<SortingSCFGRule>::iterator it = rules.begin(); it!= rules.end();it++){
		if(!(sorted_rules_.size())||!(sorted_rules_.back() == *it)){
			sorted_rules_.push_back(*it);
		}else{
			sorted_rules_.back().weight_ += it->weight_;
		}
	}
}

void RuleExtractor::ApplyEventStrings(){
	for(list<SCFGRule>::iterator it = filtered_rules_.begin(); it!= filtered_rules_.end();it++){
		SCFGRule& rule = *it;
		int startF = rule.rhs_target.front().index_start;
		int endF = rule.rhs_target.back().index_end;
		set<string> events = get_events(startF, endF);
		rule.events.insert(events.begin(),events.end());
		bool is_second = false;
		for(int i = 0; i < (int)rule.rhs_source.size(); i++){
			if(rule.rhs_source[i].is_non_terminal){
				int j = *rule.terminal_alignment[i].begin();
				startF = rule.rhs_target[j].index_start;
				endF = rule.rhs_target[j].index_end;
				set<string> events1 = get_events(startF, endF);
				if(is_second){
					rule.nt_events_2.insert(events1.begin(),events1.end());
				}else{
					is_second = true;
					rule.nt_events_1.insert(events1.begin(),events1.end());
				}
			}
		}
	}
}

void RuleExtractor::NormalizeUncoveredWords() {
	map<string, map<string, float> >::iterator it =
			uncovered_words_.begin();
	for (; it != uncovered_words_.end(); it++) {
		float count = 0;
		for (map<string, float>::iterator it1 = it->second.begin(); it1
				!= it->second.end(); it1++)
			count += it1->second;
		for (map<string, float>::iterator it1 = it->second.begin(); it1
				!= it->second.end(); it1++)
			it1->second/=count;
	}
}

void RuleExtractor::add_initial_phrase(int startE, int startF, int endE, int endF) {

	initial_phrases_.push_back(SCFGRule());
	SCFGRule& new_rule = initial_phrases_.back();
	new_rule.lhs_ = "X"; // Initial phrase always X
	new_rule.weight = 1;
	new_rule.rhs_source.resize(endE - startE + 1);
	new_rule.rhs_target.resize(endF - startF + 1);
	new_rule.terminal_alignment.resize(endE-startE +1);
	new_rule.is_srl_rule = false;
	bool startFAligned = false, endFAligned = false, startEAligned = false, endEAligned = false;
	for (int i = startE; i <= endE; i++) {
		new_rule.rhs_source[i - startE].is_non_terminal = false;
		new_rule.rhs_source[i - startE].index_start =
				new_rule.rhs_source[i - startE].index_end = i;

		set<int>::const_iterator it = align_->AlignedToEnglish(i).begin();
		for(;it!=align_->AlignedToEnglish(i).end();it++){
			new_rule.terminal_alignment[i-startE].insert(*it - startF);
			if(*it == startF)
				startFAligned = true;
			if(*it == endF)
				endFAligned = true;
			if(i==startE)
				startEAligned = true;
			if(i==endE)
				endEAligned = true;
		}
	}

	for (int i = startF; i <= endF; i++) {
		new_rule.rhs_target[i - startF].is_non_terminal = false;
		new_rule.rhs_target[i - startF].index_start =
				new_rule.rhs_target[i- startF].index_end = i;
	}
	new_rule.boudnary_aligned = startEAligned && startFAligned && endEAligned && endFAligned;
	new_rule.num_non_terminal = 0;

}

void RuleExtractor::add_ghkm_phrase(int startE, int startF, int endE, int endF, const string& lhs, int predicate_index) {

	initial_ghkm_rules_.push_back(SCFGRule());
	SCFGRule& new_rule = initial_ghkm_rules_.back();
	new_rule.lhs_ = lhs;
	new_rule.weight = 1;
	new_rule.rhs_source.resize(endE - startE + 1);
	new_rule.rhs_target.resize(endF - startF + 1);
	new_rule.terminal_alignment.resize(endE-startE +1);
	new_rule.is_srl_rule = true;
	new_rule.predicate_region = this->predicate_word_indice_[predicate_index];
	new_rule.predicate_index = predicate_index;

	bool startFAligned = false, endFAligned = false, startEAligned = false, endEAligned = false;
	for (int i = startE; i <= endE; i++) {
		new_rule.rhs_source[i - startE].is_non_terminal = false;
		new_rule.rhs_source[i - startE].index_start =
				new_rule.rhs_source[i - startE].index_end = i;

		set<int>::const_iterator it = align_->AlignedToEnglish(i).begin();
		for(;it!=align_->AlignedToEnglish(i).end();it++){
			new_rule.terminal_alignment[i-startE].insert(*it - startF);
			if(*it == startF)
				startFAligned = true;
			if(*it == endF)
				endFAligned = true;
			if(i==startE)
				startEAligned = true;
			if(i==endE)
				endEAligned = true;
		}
	}

	for (int i = startF; i <= endF; i++) {
		new_rule.rhs_target[i - startF].is_non_terminal = false;
		new_rule.rhs_target[i - startF].index_start =
				new_rule.rhs_target[i- startF].index_end = i;
	}
	new_rule.boudnary_aligned = startEAligned && startFAligned && endEAligned && endFAligned;
	new_rule.num_non_terminal = 0;

}


bool RuleExtractor::is_feasible_sub_phrase(const SCFGRule& parent, const SCFGRule& child){
	int parent_start = parent.rhs_source.front().index_start;
	int parent_end = parent.rhs_source.back().index_end;
	int parent_start_f = parent.rhs_target.front().index_start;
	int parent_end_f = parent.rhs_target.back().index_end;
	int child_start = child.rhs_source.front().index_start;
	int child_end = child.rhs_source.back().index_end;
	int child_start_f = child.rhs_target.front().index_start;
	int child_end_f = child.rhs_target.back().index_end;

	if( parent_start > child_start)	return false;
	if( parent_end < child_end) return false;
	if( parent_start_f > child_start_f)	return false;
	if( parent_end_f < child_end_f) return false;


	// Make sure we must replace the same predicate argument structure
	if(parent.is_srl_rule && child.is_srl_rule){
		if(parent.predicate_index != child.predicate_index) return false;
	}

	if(parent.num_non_terminal > 0){
		// Make sure : 1. child do not overlap with the non-terminals 2. child do not cause adjacent non-terminals
		for(int i = 0; i< (int)parent.rhs_source.size(); i++){
			if(parent.rhs_source[i].is_non_terminal){
				int parent_start = parent.rhs_source[i].index_start;
				int parent_end = parent.rhs_source[i].index_end;
				if(parent_start >= child_start
					&&	parent_start  <= child_end)
					return false;
				if(parent_end>= child_start
						&& parent_end <= child_end)
					return false;
				if(parent_start < child_start
						&& parent_end > child_end)
					return false;
				if (parent_end == child_start -1
						|| parent_start == child_end +1)
					return false; // cannot adjacent
			}
		}

		for(int i = 0; i< (int)parent.rhs_target.size(); i++){
			if(parent.rhs_target[i].is_non_terminal){
				int parent_start = parent.rhs_target[i].index_start;
				int parent_end = parent.rhs_target[i].index_end;
				if(parent_start >= child_start_f
					&&	parent_start  <= child_end_f)
					return false;
				if(parent_end>= child_start_f
						&& parent_end <= child_end_f)
					return false;
				if(parent_start < child_start_f
						&& parent_end > child_end_f)
					return false;
			}
		}
	}

	if(parent.is_srl_rule && !child.is_srl_rule){
		// Parent is srl_rule, but children is not srl, then we need to make sure the predicate are not replaced
		if(child_end_f >= parent.predicate_region.second &&
				child_start_f <= parent.predicate_region.second)
			return false;
		if(child_end_f >= parent.predicate_region.first &&
				child_start_f <= parent.predicate_region.first)
			return false;

	}
	return true;
}

void RuleExtractor::try_substitute_one_sub_phrase(const SCFGRule& parent, const SCFGRule& child, list<SCFGRule>& pool){
	// After removal, the source side and target side's length can be calculated
	int length_source = parent.rhs_source.size() - child.rhs_source.size() + 1;
	int length_target = parent.rhs_target.size() - child.rhs_target.size() + 1;

	if(length_source > max_tokens_ /*|| length_target > max_tokens_*/)
		return;
	SCFGRule new_rule;
	new_rule.boudnary_aligned = parent.boudnary_aligned;
	new_rule.lhs_ = parent.lhs_;
	new_rule.rhs_source.resize(length_source);
	new_rule.rhs_target.resize(length_target);
	new_rule.non_terminal_indices_source =parent.non_terminal_indices_source;
	new_rule.terminal_alignment.resize(length_source);
	new_rule.num_non_terminal = parent.num_non_terminal+ 1;
	new_rule.is_srl_rule = parent.is_srl_rule;
	new_rule.predicate_region = parent.predicate_region;
	new_rule.predicate_index = parent.predicate_index;

	// Replace the source
	int i = 0, current_pos = 0;
	int child_start = child.rhs_source.front().index_start;
	int child_end = child.rhs_source.back().index_end;

	bool has_terminal_alignment = false; // need to make sure some word is aligned
	int child_start_f = child.rhs_target.front().index_start;
	int child_end_f = child.rhs_target.back().index_end;
	int child_nt_length = child_end_f - child_start_f + 1;

	current_pos = 0;

	int index_new_non_terminal = -1;
	// Replace the target first, record the new non-terminal's index
	for (i = 0; i < (int)parent.rhs_target.size(); i++) {
		int start = parent.rhs_target[i].index_start;
		if (start == child_start_f) { // replace it
			new_rule.rhs_target[current_pos].index_start = child_start_f;
			new_rule.rhs_target[current_pos].index_end = child_end_f;
			new_rule.rhs_target[current_pos].is_non_terminal = true;
			new_rule.rhs_target[current_pos].non_terminal_tag = child.lhs_;
			index_new_non_terminal = current_pos;
			current_pos++;
		} else {
			if (start > child_start_f && start <= child_end_f) // substitued words
				continue;
			else {
				new_rule.rhs_target[current_pos].index_start = start;
				new_rule.rhs_target[current_pos].index_end	= parent.rhs_target[i].index_end;
				new_rule.rhs_target[current_pos].is_non_terminal= parent.rhs_target[i].is_non_terminal;
				new_rule.rhs_target[current_pos].non_terminal_tag
						= parent.rhs_target[i].non_terminal_tag;
				current_pos++;
			}
		}
	}

	assert(index_new_non_terminal > -1);
//	if(index_new_non_terminal == -1){
//		this->is_feasible_sub_phrase(parent, child);
//		try_substitute_one_sub_phrase(parent, child, pool);
//	}
	current_pos = 0 ;
	for(i = 0; i < (int)parent.rhs_source.size(); i++){
		int start = parent.rhs_source[i].index_start;
		if(start == child_start){ // replace it
			new_rule.rhs_source[current_pos].index_start = child_start;
			new_rule.rhs_source[current_pos].index_end = child_end;
			new_rule.rhs_source[current_pos].is_non_terminal = true;
			//new_rule.rhs_source[current_pos].non_terminal_tag = child.lhs_;
			new_rule.rhs_source[current_pos].non_terminal_tag = "X";
			new_rule.terminal_alignment[current_pos].insert(index_new_non_terminal);
			current_pos ++;

		}else{
			if(start > child_start && start <= child_end) // substitued words
				continue;
			else{
				new_rule.rhs_source[current_pos].index_start = start;
				new_rule.rhs_source[current_pos].index_end = parent.rhs_source[i].index_end;
				new_rule.rhs_source[current_pos].is_non_terminal = parent.rhs_source[i].is_non_terminal;
				new_rule.rhs_source[current_pos].non_terminal_tag = parent.rhs_source[i].non_terminal_tag;
				set<int>::const_iterator it = parent.terminal_alignment[i].begin();
				for(;it!=parent.terminal_alignment[i].end();it++){
					// once reach here, we have at least one concrete word aligned
					has_terminal_alignment = true;
					int ori = *it;
					if(ori < index_new_non_terminal )
						new_rule.terminal_alignment[current_pos].insert(ori);
					else
						new_rule.terminal_alignment[current_pos].insert(ori - (child_nt_length) + 1); // removed child_nt_length words, added one non-terminal

				}
				current_pos++;
			}
		}
	}



	if(has_terminal_alignment){
		pool.push_back(new_rule);
	}
}

string RuleExtractor::build_srl_tag_name(const vector<string>& tagnames, const string& frame,int startR, int endR){
	string tag = "";
	string prefix = "#";
	switch(initial_flag_){
	case EITPredArgCombination:
		prefix = "#"+ frame + "/";
	case EITArgumentCombination:
		for(int i = startR; i<=endR; i++){
			string vname;
			if(tagnames[i]=="TARGET")
				continue;
			if(tagnames[i].length() > 3)
				vname = tagnames[i].substr(3, tagnames[i].length()-3);
			else
				vname = tagnames[i];
			tag += vname;
			if(i <= endR){
				tag += "_";
			}
		}
		return prefix + tag;
	case EITGeneralX:
		return "X";
	default:
		cerr << "Error you should not go here";
		exit(1);
	}
	return "";
}

bool RuleExtractor::find_and_add_ghkm_phrase(const vector<pair<int,int> >& regions, int startR, int endR, const string& tag,int predicate_index){
/*
 * do this iteratively:
 * 1. Initialize the source side phrase, covering the all the regions
 * 2. Determine the minimal target side phrase, using the alignment information
 * 3. Check the target side phrase, if all its alignment falls in the source side phrase, add the newly extracted
 * 4. Otherwise, add the propose the new phrase boundary, which covers all words the target side aligns to.
 * 5. If the boundary exceed the adjacent regions, return fasle
 * 6. Continue to step 2
 */

	int sbound_l = regions[startR].first, sbound_r = regions[endR].second;
	int tbound_l, tbound_r, sbound_lm, sbound_rm, sbound_nl, sbound_nr;
	const Alignment& aln = *align_;

	sbound_lm = startR == 0 ? 0 : regions[startR - 1].second + 1; // sbound_lm is inclusive and rm is exclusive
	sbound_rm = endR == ((int)regions.size()) -1 ? (int)sent_->english().length() : regions[endR + 1].first;
	for(;;){
		int l = sent_->french().length(), r = -1;
		for(int i = sbound_l ; i<=sbound_r; i++){
			for(set<int>::iterator it = aln.AlignedToFrench(i).begin();
					it != aln.AlignedToFrench(i).end(); it++){
				if(*it > r)
					r = *it;
				if(*it < l)
					l = *it;
			}
		}
		if(r<l) return false; // nothing added because not a single alignment is seen
		tbound_l = l;
		tbound_r = r;
		l = sbound_l, r = sbound_r;
		for (int i = tbound_l; i <= tbound_r; i++) {
			for (set<int>::iterator it = aln.AlignedToEnglish(i).begin();
					it != aln.AlignedToEnglish(i).end(); it++) {
				if (*it > r)
					r = *it;
				if (*it < l)
					l = *it;
			}
		}
		sbound_nl = l;
		sbound_nr = r;
		if(sbound_nl >= sbound_l && sbound_nr <= sbound_r){ // An allowed
			add_ghkm_phrase(tbound_l, sbound_l,tbound_r,sbound_r,tag, predicate_index);
			//cerr << "ADDED ONE: " << sbound_l << "," <<tbound_l<< ","<<sbound_r<< ","<<tbound_r<< "," << tag  << endl;
			return true;
		}
		// otherwise, check if we can extend?
		if(sbound_nl < sbound_lm || sbound_nr >= sbound_rm){
			// no we cannot
			return false;
		}
		sbound_l = sbound_nl;
		sbound_r = sbound_nr;
	}

	cerr << "You should not be here dude!" << endl ;
	return false;

}


string RuleExtractor::do_stemming(const string& ori){
	char* ch = new char[ori.length()+1];
	string lc = to_lower_copy(ori);
	strcpy(ch,lc.c_str());
	ch[stem(stemmer,ch,ori.length()-1)+1] = '\0';
	lc = ch;
	delete[] ch;
	return lc;
}

void RuleExtractor::MergetUncoveredWordsAndStructures(const RuleExtractor& other){
	// merge covered words

	// remove uncovered words
	for(set<string>::iterator it = other.covered_words_.begin(); it!=other.covered_words_.end(); it++){
		map<string, map<string,float> >::iterator it_find = uncovered_words_.find(*it);
		if(it_find != uncovered_words_.end()){
			uncovered_words_.erase(it_find);
		}
		covered_words_.insert(*it);
	}
	for(map<string, map<string,float> >::const_iterator it = other.uncovered_words_.begin();
			it!=other.uncovered_words_.end();it++){
		set<string>::iterator it_find = covered_words_.find(it->first);
		if(it_find==covered_words_.end()){
			map<string, map<string,float> >::iterator it_find_u = uncovered_words_.find(it->first);
			if(it_find_u == uncovered_words_.end()){
				uncovered_words_.insert(*it);
			}else{
				for(map<string,float>::const_iterator it2 = it->second.begin(); it2!=it->second.end(); it2++){
					map<string,float>::iterator it3 = it_find_u->second.find(it2->first);
					if(it3 == it_find_u->second.end()){
						it_find_u->second.insert(*it2);
					}else{
						it3->second += it2->second;
					}
				}
			}
		}
	}
	completed_srl_structures_.insert(other.completed_srl_structures_.begin(),other.completed_srl_structures_.end());

}

list<SRLGenRuleWithFeature > RuleExtractor::replace_one_argument(int structidx, int regionidx, SRLGenRuleSet* ruleset,
	int max_expanded, int min_words){
	const SentencePair& sent = *this->sent_;
	const Alignment & alignment = *this->align_;
//	const vector<SRLInformation> &srl = *this->srl_;
	vector<vector<string> >& tags = mapped_srl_tags_;
	vector<vector<pair<int, int> > >& regions = mapped_srl_regions_;
	vector<bool>& valid = srl_info_valid_;
	list<SRLGenRuleWithFeature > ret;

	int i = structidx;

	if (!valid[i])
		return ret;

	set<int> target_words, source_words;
	vector<pair<int, int> >& cregion = regions[i];

	int target_insert_point_l = cregion[regionidx].first, target_insert_point_r = cregion[regionidx].second;

	for (int j = 0; j < (int) cregion.size(); j++) {
		if (j != regionidx)
			for (int k = cregion[j].first; k <= cregion[j].second; k++) {
				target_words.insert(k);
			}
	}
	int originalsize = (int) target_words.size();
	int lastsize = originalsize;
	// Find all sourcd words
	do {
		lastsize = target_words.size();
		for (set<int>::iterator it = target_words.begin(); it
				!= target_words.end(); it++) {
			const set<int>& al = alignment.AlignedToFrench(*it);
			source_words.insert(al.begin(), al.end());
		}
		for (set<int>::iterator it = source_words.begin(); it
				!= source_words.end(); it++) {
			const set<int>& al = alignment.AlignedToEnglish(*it);
			target_words.insert(al.begin(), al.end());
		}
	} while ((int) target_words.size() < lastsize);

	set<int> exp = source_words;
	// Add internal source word, if it is not aligned
	for (set<int>::iterator it = exp.begin(); it != exp.end(); it++) {
		int left = *it - 1;
		int right = *it + 1;
		while (left >= 0) {
			if (source_words.find(left) != source_words.end()) {
				break;
			}
			if (alignment.AlignedToEnglish(left).size() == 0) {
				source_words.insert(left--);
			} else {
				break;
			}
		}

		while (right < (int) sent.english().length()) {
			if (source_words.find(right) != source_words.end()) {
				break;
			}
			if (alignment.AlignedToEnglish(right).size() == 0) {
				source_words.insert(right++);
			} else {
				break;
			}
		}
	}

	if (originalsize - (int) target_words.size() < max_expanded
			&& (int) target_words.size() > min_words
			&& (int) source_words.size() > min_words) {
		string src, tgt;
		for (set<int>::iterator it = target_words.begin(); it!= target_words.end(); it++) {
			if(*it < target_insert_point_r && *it > target_insert_point_l){
				if(target_insert_point_r - *it > *it - target_insert_point_l){
					target_insert_point_l = *it+1;
				}else{
					target_insert_point_r = *it-1;
				}
			}
		}
		int source_insert_point_l = sent.english().length(), source_insert_point_r = -1;
		for(int k = target_insert_point_l; k<= target_insert_point_r;k++){
			if(alignment.AlignedToFrench(k).size()==0) continue;
			int start = *alignment.AlignedToFrench(k).begin();
			int end = *alignment.AlignedToFrench(k).rbegin();
			if(start < source_insert_point_l) source_insert_point_l = start;
			if( end  > source_insert_point_r) source_insert_point_r = end;
		}
		if(source_insert_point_l <0 || source_insert_point_r >= sent.english().length()){
			return ret;
		}

//		for (set<int>::iterator it = source_words.begin(); it!= source_words.end(); it++) {
//			if(*it < source_insert_point_r && *it > source_insert_point_l){
//				if(source_insert_point_r - *it > *it - source_insert_point_l){
//					source_insert_point_l = *it+1;
//				}else{
//					source_insert_point_r = *it-1;
//				}
//			}
//
//		}
		
		const vector<SRLGenRuleWithFeature> & rep = ruleset->GetRules(srl_frames_[structidx], tags[structidx][regionidx]);
		//cerr << rep.size() << srl_frames_[structidx] << tags[structidx][regionidx] << endl;
		for(int i=0;i<(int)rep.size();i++){

			const string& src_replacement = rep[i].source;
			const string& tgt_replacement = rep[i].target;

			if(!this->output_all_text_){
				bool replaced = false;
				string rtgt="";
				tgt = "";
				for (set<int>::iterator it = target_words.begin(); it
					!= target_words.end(); it++) {
					if(*it > target_insert_point_l && ! replaced){
						tgt = glue_strings(tgt,tgt_replacement);
						replaced = true;
					}
					if(!replaced)
						tgt += sent.french().GetWordAtIndex(*it) + " ";
					else
						rtgt += sent.french().GetWordAtIndex(*it) + " ";
				}
				if(!replaced){
					tgt = glue_strings(tgt,tgt_replacement);
				}else{
					tgt = glue_strings(tgt,rtgt);
				}
				replaced = false;
				string rsrc="";
				src = "";
				for (set<int>::iterator it = source_words.begin(); it
					!= source_words.end(); it++) {
					if(*it > source_insert_point_l && ! replaced){
						//cerr << src << "|||" << src_replacement ;
						src = glue_strings(src, src_replacement );
						replaced  = true;
					}
					if(!replaced)
						src += sent.english().GetWordAtIndex(*it) + " ";
					else
						rsrc += sent.english().GetWordAtIndex(*it) + " ";

				}
				if(!replaced){
					src = glue_strings(src,src_replacement);
				}else{
					src = glue_strings(src,rsrc);
				}
			}else{
				bool replaced = false;
				string rtgt="";
				tgt = "";
				for (int j = 0; j < (int)sent.french().length();j++) {
					if(j >= target_insert_point_l && ! replaced){
						tgt = glue_strings(tgt,tgt_replacement);
						replaced = true;
					}
					if (j<= target_insert_point_r && j >= target_insert_point_l){
						continue;
					}
					if(!replaced)
						tgt += sent.french().GetWordAtIndex(j) + " ";
					else
						rtgt += sent.french().GetWordAtIndex(j) + " ";
				}
				if(!replaced){
					tgt = glue_strings(tgt,tgt_replacement);
				}else{
					tgt = glue_strings(tgt,rtgt);
				}
				replaced = false;
				string rsrc="";
				src = "";
				for (int j = 0; j<(int)sent.english().length();j++) {
					if(j >= source_insert_point_l && ! replaced){
						//cerr << src << "|||" << src_replacement ;
						src = glue_strings(src, src_replacement );
						replaced  = true;
					}
					if (j<= source_insert_point_r && j >= source_insert_point_l){
						continue;
					}
					if(!replaced)
						src += sent.english().GetWordAtIndex(j) + " ";
					else
						rsrc += sent.english().GetWordAtIndex(j) + " ";

				}
				if(!replaced){
					src = glue_strings(src,src_replacement);
				}else{
					src = glue_strings(src,rsrc);
				}				
			}
			trim(tgt);
			trim(src);
			SRLGenRuleWithFeature nfeat;
			nfeat.source = src;
			nfeat.target = tgt;
			nfeat.features = rep[i].features;
			float feat = 0;
			for(int k = 0 ; k < (int)nfeat.features.size() ; k++){
				feat += log(nfeat.features[k]);
			}
			nfeat.sum_feature = feat;
			ret.push_back(nfeat);
		}
	}

	return ret;
}

string RuleExtractor::glue_strings(const string& r1, const string& r2){
	vector<string> w1, w2,o1,o2;

	string v1=r1,v2=r2;
	trim(v1),trim(v2);
	split(o1,v1,is_any_of(" \t"), token_compress_on);
	split(o2,v2,is_any_of(" \t"), token_compress_on);
	w1.resize(o1.size());
	w2.resize(o2.size());
	for(int i = 0; i< (int)w1.size(); i++){
		w1[i] = do_stemming(o1[i]);
	}
	for(int i = 0; i< (int)w2.size(); i++){
		w2[i] = do_stemming(o2[i]);
	}
	int w1l = w1.size();
	int maxgs = 0;
	for(int gs= 1; gs < (int)w1.size() && gs < (int)w2.size() ; gs++){
		bool mismatch = false;
		for(int i = 0; i< gs ; i++){
			if(w1[w1l-gs + i] != w2[i]){
				mismatch = true;
				break;
			}
		}
		if(!mismatch){
			maxgs = gs;
		}
	}

	string ret = v1 + " |||";
	for(int i = maxgs ; i< (int)o2.size(); i++){
		ret += " " + o2[i];
	}

	return ret;

}

set<string> RuleExtractor::get_events(int start, int end){
	set<string> ret;
	const vector<SRLInformation>& srl = *this->srl_;
	vector<vector<pair<int, int> > >& regions = mapped_srl_regions_;
	for(int i = 0; i< (int)srl.size();i++){
		bool is_in_frame = false;
		for(int j = 0; j < (int)regions[i].size();j++){
			if(overlap(regions[i][j].first,regions[i][j].second,start,end)){
				// add the event
				ret.insert(srl_frames_[i]);
				is_in_frame = true;
				break;
			}
		}
		if(!is_in_frame)
			ret.insert("G_S"); // If not in the frame, add a general tag
	}
	if(ret.size()==0)
		ret.insert("G_S");
	return ret;
}




}
