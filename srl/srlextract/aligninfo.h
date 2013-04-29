/**
 * This two interfaces: The sentence pair and the alignment
 */

#ifndef LIBSRL_ALIGNINFO_H_
#define LIBSRL_ALIGNINFO_H_

#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <boost/tuple/tuple.hpp>

namespace srl{


class AlignmentInfoReader;
class RuleExtractor;

/**
 * A sentence pair will contain two sentences, source and target
 */
class Sentence{

	friend class AlignmentInfoReader;
	friend class RuleExtractor;

public:
	inline const std::string& GetWordAtIndex(size_t i) const {return words_[i];};
	inline size_t length() const{return words_.size();};

	Sentence(){}
	
	~Sentence(){}

	inline const Sentence& operator=(const Sentence& n) {words_ = n.words_; return *this;};
protected:
	std::vector<std::string> words_;
};

/**
 * Now the sentence pair
 */

class SentencePair{
	friend class AlignmentInfoReader;
	friend class RuleExtractor;

public:
	inline const Sentence& english() const {return english_;};
	inline const Sentence& french() const{return french_;};
	SentencePair(){};
	~SentencePair(){};

private:
	Sentence english_, french_;
};

/**
 * And the alignment
 */

class Alignment{
	friend class AlignmentInfoReader;
public:
	inline const std::set<int>& AlignedToEnglish(size_t i)const{return aligned_to_english_[i];};
	inline const std::set<int>& AlignedToFrench (size_t i)const{return aligned_to_french_[i];};
	Alignment(){};
	~Alignment(){};
private:
	std::vector<std::set<int> > aligned_to_english_; // element i is the French words' indices aligned to the English word i
	std::vector<std::set<int> > aligned_to_french_; // element i is the French words' indices aligned to the English word i
};


class AlignmentInfoReader{
public:
	AlignmentInfoReader(std::istream& /*english*/, std::istream& /*french*/, std::istream& /*align*/);
	~AlignmentInfoReader(){}
	/**
	 * Read a new sentence pair, if failed, the first element will be false
	 */
	boost::tuples::tuple<bool,SentencePair,Alignment> Read();

private:
	std::istream  &english_, &french_, &align_;
};
}
#endif
