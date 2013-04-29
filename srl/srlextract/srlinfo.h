/*
 * srcheader.h
 *
 *  Created on: Nov 5, 2010
 *      Author: qing
 */

#ifndef SRLHEADER_H_
#define SRLHEADER_H_

#include <string>
#include <vector>

namespace srl{

class SRLInfoReader;

class SRLInformation{
	friend class SRLInfoReader;
public:
	// Public accessors
	inline size_t GetNumberOfWords()const {return words_.size();};
	inline const std::string& GetWordAtIndex(size_t index)const{return words_[index];};
	inline size_t GetNumberOfLabels() const{return label_.size();};
	inline const std::string& GetLabelAtIndex(size_t index)const{return label_[index];};
	inline const std::pair<int,int>& GetRegionAtIndex(size_t index) const {return regions_[index];};
	inline int GetSentenceNumber() const{return sent_number_;};
	inline bool error() const{return error_;};


	// Constructors
	SRLInformation();
	~SRLInformation();


protected:
	// Actual initializer
	void Init();

private:
	std::vector<std::string> words_;
	std::vector<std::string> label_;
	// The regions should be INCLUSIVE
	std::vector<std::pair<int,int> > regions_;
	bool error_;
	int sent_number_;
};

}

#endif /* SRCHEADER_H_ */
