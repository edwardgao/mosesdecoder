/*
 * srlinfoparser.h
 *
 *  Created on: Nov 6, 2010
 *      Author: qing
 */

#ifndef SRLINFOPARSER_H_
#define SRLINFOPARSER_H_


#include<string>
#include<vector>
#include<iostream>

namespace srl{



class SRLInformation;



class SRLInfoReader{
public:
	std::vector<SRLInformation> ReadSentence(int /*sent_num*/);
	SRLInfoReader(std::istream& input): input_(input){};

protected:
	virtual SRLInformation ParseSRLInfo(const std::string& /*input*/);
private:
	SRLInformation cache_;
	std::istream& input_;

};


}


#endif /* SRLINFOPARSER_H_ */
