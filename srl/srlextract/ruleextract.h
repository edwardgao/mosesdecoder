/*
 * ruleextract.h
 *
 *  Created on: Nov 7, 2010
 *      Author: qing
 *
 *      Rule Extraction with SRL Labels
 *
 *      Basic idea:
 *
 *      1. Extract all initial phrases, up to a certain length, with Franz Och standard
 *      2. Extract all feasible GHKM rules given SRL labels (as one-layer trees)
 *      	The standard is :
 *          A. Phrase covers a whole SRL Tag (i.e. the boundary is not inside the SRL tag)
 *          B. The source side does not have alignment out of the boundary of the SRL tag
 *          C. (optional) The source side boundary words must be aligned to some word in the target side
 *      For all the FO phrases, give them a tag X
 *      For a SRL tag, give them one of: (parameterized)
 *       	1. the argument tag, combining all arguments it have
 *          2. the predicate tag together with the argument tag
 *          3. general X
 *
 *         Remember all different predicate-argument structures must be included to build this initial rules.
 *
 *      3. By now we have no non-terminals, start replacing the rules with rules in the pool, and substract the occurrence,
 *         the replacement is subject to David Chiang's criteria.
 *
 *      4. Further replace the second non-terminal
 *
 *      5. Output all allowed rules.
 *
 *      For test, a sentence must output all standard Hiero rule when no SRL information is there.
 *
 *
 */

#ifndef RULEEXTRACT_H_
#define RULEEXTRACT_H_
#include <string>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <queue>
#include <boost/thread/thread.hpp>



extern "C"{
	struct stemmer;
}
namespace srl{

class SentencePair;
class Alignment;
class SRLInformation;
class SRLGenRuleSet;
struct SRLGenRuleWithFeature;


enum EInitialTagType{
	EITPredArgCombination  = 0,  // The LHS will be a combination of arguments, ignoring including predicate X
	EITArgumentCombination = 1,  // The LHS will be a combination of arguments, ignoring predicate X
	EITGeneralX            = 2,  // General X
};




struct SCFGToken{
	bool is_non_terminal;
	int index_start;
	int index_end; // All indices inclusive, and is the indices in the ORIGINAL sentence
	std::string non_terminal_tag;
};

struct SCFGRule{
	std::string lhs_;
	std::vector<SCFGToken> rhs_source;
	std::vector<SCFGToken> rhs_target;
	std::vector<std::set<int> > terminal_alignment; // The indices here, both the vector and the value, are absolute, corresponding to rhs_source/rhs_target
	std::map<int,int> non_terminal_indices_source; // Contain all the indices of non-terminals, and the key is source, the value is target
	float weight;
	// For determining gap
	bool boudnary_aligned;
	int num_non_terminal;
	bool is_srl_rule;
	std::pair<int,int> predicate_region;
	int predicate_index;

	// Additional Event List
	std::set<std::string> events;
	std::set<std::string> nt_events_1;
	std::set<std::string> nt_events_2;
};

struct SRLGenRule{
	std::string frame;
	std::string argument;
	std::vector<std::string> source;
	std::vector<std::string> target;
};

struct SRLGenStruct{
	std::string frame;
	std::vector<std::string> arguments;
};





class SortingSCFGRule{
	friend class RuleExtractor;
public:
	inline const std::string& source() const{return source_;};
	inline const std::string& target() const{return target_;};
	inline const std::string& align() const{return align_;};
	inline const std::string& align_reversed() const{return align_reversed_;};
	inline const std::string& event_string() const{return event_string_;};
	inline float weight() const{return weight_;};

	inline void set_source(const std::string& x){source_ = x;};
	inline void set_target(const std::string& x){target_ = x;};
	inline void set_align(const std::string& x){align_ = x;};
	inline void set_align_reversed(const std::string& x){align_reversed_ = x;};
	inline void set_weight(float w) const{weight_ =w;};
	inline void set_event_string(const std::string& event){event_string_ = event;};

	// for sort
	inline bool operator<(const SortingSCFGRule& rt) const{
		int v = source_.compare(rt.source_);
		if(v<0) return true;
		if(v>0) return false;
		v = target_.compare(rt.target_);
		if(v<0) return true;
		if(v>0) return false;
		v = align_.compare(rt.align_);
		if(v<0) return true;
		return false;
	};

	// for comparison
	inline bool operator==(const SortingSCFGRule& rt) const{
		return source_ == rt.source_ && target_ == rt.target_ && align_ == rt.align_;
	};

	SortingSCFGRule(const SCFGRule& /*conv*/,const SentencePair& sent);
private:
	std::string source_, target_, align_, align_reversed_;
	mutable float weight_;
	// Knowing the fact: For a single sentence, the event string IS THE SAME for any rule
	mutable std::string event_string_;
};

// Convert SCFGRule to output, sentence will be the sentence number, if it is -1, then don't output, and can output accessory string
std::pair<std::string,std::string> SerializeSCFGRule(const SCFGRule& scfg, const SentencePair& sent, int sentence = -1, const char* accessory = NULL);
std::pair<std::string,std::string> SerializeSCFGRule_PhraseOnly(const SCFGRule& scfg, const SentencePair& sent, int sentence = -1, const char* accessory = NULL);
std::pair<std::string,std::string> SerializeSortedSCFGRule(const SortingSCFGRule& scfg, int sentence = -1, const char* accessory = NULL);

std::pair<std::string,std::string> SerializeUncovered_PhraseOnly(const std::string& source, const std::string& target);
std::pair<std::string,std::string> SerializeUncovered(const std::string& source, const std::string& target, float weight);


/**
 * Configurations for the rule extractor:
 *
 * 1. How many tokens allowed for final output?
 * 2. How long is the initial phrases?
 * 3. What kind of initial tags should we assign to the SRL rules?
 */
class RuleExtractor{

public:

	// For threading
	struct Task{
		int sent_number;
		SentencePair sent;
		Alignment align;
		std::vector<SRLInformation> srl;
	};

	struct TaskResult{
		int sent_number;
		std::string output_str, output_inv;
	};


	// Iterators to enable navigating the rules
	typedef std::list<SCFGRule>::iterator iterator;
	typedef std::list<SCFGRule>::const_iterator const_iterator;

	typedef std::list<SortingSCFGRule>::iterator sorted_iterator;
	typedef std::list<SortingSCFGRule>::const_iterator sorted_const_iterator;

	sorted_iterator sorted_rule_begin();
	sorted_iterator sorted_rule_end();

	sorted_const_iterator sorted_rule_begin() const;
	sorted_const_iterator sorted_rule_end() const;


	iterator filtered_rule_begin();
	iterator filtered_rule_end();

	const_iterator filtered_rule_begin() const;
	const_iterator filtered_rule_end() const;

	iterator rule_begin();
	iterator rule_end();

	const_iterator rule_begin() const;
	const_iterator rule_end() const;

	iterator initial_phrase_begin();
	iterator initial_phrase_end();

	const_iterator initial_phrase_begin() const;
	const_iterator initial_phrase_end() const;

	iterator ghkm_rule_begin();
	iterator ghkm_rule_end();

	const_iterator ghkm_rule_begin() const;
	const_iterator ghkm_rule_end() const;

	// Call it to perform actual extraction
	bool DoExtraction(
			const SentencePair& /*sent*/,
			const Alignment& /*alignment*/,
			const std::vector<SRLInformation>& /*srl*/,
			int sent_number
			);

	std::list<SRLGenRuleWithFeature > DoSentenceShortening(
			const SentencePair& /*sent*/,
			const Alignment& /*alignment*/,
			const std::vector<SRLInformation>& /*srl*/,
			int sent_number,int max_expanded,int min_words,
			SRLGenRuleSet* ruleset = NULL,
			int max_replacement = 0
			);

	std::pair<std::list<SRLGenRule>, std::list<SRLGenStruct> > DoExtractSRLGenerationRules(
			const SentencePair& /*sent*/,
			const Alignment& /*alignment*/,
			const std::vector<SRLInformation>& /*srl*/,
			int sent_number,int max_expanded,int max_words
			);

	/// Do Monolingual extraction
	std::list<std::string> DoMonolingualSRLExtraction (
		const std::string& /*sent*/,
		const std::vector<SRLInformation>&, /*srl*/
		int /*context_len*/
		);

	void RunDaemon(std::queue<Task>& tasks, std::queue<TaskResult>& results,
			boost::mutex& tasks_mutex, boost::mutex& results_mutex, bool& quit_flag
			);

	// Utility interfaces
	RuleExtractor();
	~RuleExtractor();

	inline EInitialTagType initial_flag() const{return initial_flag_;};
	inline int length_initial_phrase() const{return length_initial_phrase_;};
	inline int max_tokens() const{return max_tokens_;};
	inline bool bypass_ghkm() const{return bypass_ghkm_;};
	inline bool bypass_hiero() const{return bypass_hiero_;};
	inline int max_tokens_no_terminal() const{return max_tokens_no_terminal_;};
	inline bool disallow_unaligned_on_boundary() const{return disallow_unaligned_on_boundary_;};
	inline bool track_uncovered_words() const{return track_uncovered_words_;};
	inline const std::set<std::string>& completed_srl_structures() const{return completed_srl_structures_;};
	inline const bool skip_stemming() const{return skip_stemming_;};
	inline const int min_gap_source() const{return min_gap_source_;};
	inline const int min_gap_target() const{return min_gap_target_;};
	inline const bool extract_events() const{return extract_events_;};
	inline const bool output_all_text() const{return output_all_text_;};


	inline void set_initial_flag(EInitialTagType e){initial_flag_ = e;};
	inline void set_length_initial_phrase(int e){length_initial_phrase_ = e;};
	inline void set_max_tokens(int e){max_tokens_ = e;};
	inline void set_bypass_ghkm(bool b){bypass_ghkm_ = b;};
	inline void set_bypass_hiero(bool b){bypass_hiero_ = b;};
	inline void set_max_tokens_no_terminal(int b){max_tokens_no_terminal_ = b;};
	inline void set_disallow_unaligned_on_boundary(bool b){disallow_unaligned_on_boundary_ = b;};
	inline void set_track_uncovered_words(bool b){track_uncovered_words_ = b;};
	inline void set_skip_stemming(bool b){skip_stemming_ = b;};
	inline void set_min_gap_source(int l) {min_gap_source_=l;};
	inline void set_min_gap_target(int l) {min_gap_target_=l;};
	inline void set_extract_events(bool b) {extract_events_=b;};
	inline void set_output_all_text(bool b){output_all_text_ = b;};

	inline std::map<std::string,std::map<std::string, float> >::const_iterator uncovered_words_begin()const{return uncovered_words_.begin();};
	inline std::map<std::string,std::map<std::string, float> >::const_iterator uncovered_words_end()const{return uncovered_words_.end();};
	// Special function, normalize weights of uncovered words
	void MergetUncoveredWordsAndStructures(const RuleExtractor& rt);
	void NormalizeUncoveredWords();

protected:

	// All top-level functions for the algorithm
	virtual void DoInitialization(); // Clean the intermediate data during last extraction
	virtual void DoMatchSRLInformation(); // Match SRL labels with the sentence
	virtual void DoInitalPhraseExtraction(); // Do initial extraction based on Franz Och standard
	virtual void DoGHKMPhraseExtraction();  // Do GHKM phrase extraction on SRL labels
	virtual void DoHieroSubstitution();  // Do Hiero substitution based on David Chiang Standard
	virtual void DoFilterRule(); // Filter out the rules
	virtual void DoSortAndUniq();
	virtual void ApplyEventStrings();  // Add Event Strings to the rules
	// For monolingual
	virtual void DoOutputSRLConstituents(std::list<std::string>& /*output*/, int /*contextLen*/);

protected:
	// utility classes


	// Configuration flags:
	EInitialTagType initial_flag_;
	int length_initial_phrase_;
	int max_tokens_;
	int max_tokens_no_terminal_; // maximum tokens without terminal
	bool bypass_ghkm_, bypass_hiero_;
	bool disallow_unaligned_on_boundary_;
	bool track_uncovered_words_;
	bool extract_events_;
	bool skip_stemming_;
	bool output_all_text_;
	int sent_number_;
	int min_gap_source_, min_gap_target_; // min and max gap between two non-terminals


	// Data to be carried over on the extraction
	const SentencePair* sent_;
	const Alignment* align_;
	const std::vector<SRLInformation>* srl_;


	// SRL Tag mappings
	std::vector<std::vector<std::string> > mapped_srl_tags_; // SRL tags mapped to current sentence
	std::vector<std::vector<std::pair<int,int> > > mapped_srl_regions_; // And the indices mapped
	std::vector<bool> srl_info_valid_;
	std::vector<std::string> srl_frames_;
	std::set<std::string> completed_srl_structures_; // SRL structures that are complete
	std::vector<std::pair<int,int> > predicate_word_indice_; //Current predicate's indice
	std::vector<int> predicate_segment_indice_; // current predicate's segment indice

	// Intermediate rules
	std::list<SCFGRule> initial_phrases_, initial_ghkm_rules_, subst_rules_, filtered_rules_;
	std::list<SortingSCFGRule> sorted_rules_;

	// Utility variables
	std::map<std::pair<int,int>, std::pair<int,int> > phrase_span_; // The alignment span of each source phrase
	// to recover uncovered words
	std::map<std::string,std::map<std::string, float> >uncovered_words_;
	std::set<std::string> covered_words_;

	// utility functions
	void add_initial_phrase(int /*startE*/, int /*startF*/, int /*endE*/, int /*endF*/);
	void add_ghkm_phrase(int /*startE*/, int /*startF*/, int /*endE*/, int /*endF*/, const std::string& /*lhs*/,int predicate_index);
	bool is_feasible_sub_phrase(const SCFGRule& /*parent*/, const SCFGRule& /*child*/);
	void try_substitute_one_sub_phrase(const SCFGRule& /*parent*/, const SCFGRule& /*child*/, std::list<SCFGRule>& /*pool*/);
	std::string build_srl_tag_name(const std::vector<std::string>& /*tagnames*/,const std::string& /*frame*/, int /*startR*/, int /*endR*/);
	bool find_and_add_ghkm_phrase(const std::vector<std::pair<int,int> >&/* regions*/, int /*startR*/, int /*endR*/,const std::string& /*tag*/,int predicate_index);
	std::list<SRLGenRuleWithFeature > replace_one_argument(int /*structidx*/, int /*regionidx*/, SRLGenRuleSet* /*ruleset*/,int max_expanded, int min_words);
	std::string glue_strings(const std::string& , const std::string&);
	std::set<std::string> get_events(int /*start*/, int /*end*/);


private:
	// porter stemmer
	struct stemmer* stemmer;
	// nasty porter stemmer
	std::string do_stemming(const std::string& /*ori*/);


};




}


#endif /* RULEEXTRACT_H_ */
