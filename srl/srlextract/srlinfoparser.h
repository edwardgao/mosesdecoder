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
		void ReadSentence(int sent_num, std::vector<SRLInformation>& ret);
		// Create a new vector and return, supposed to work with auto_ptr, no deallocation here!
		void ReadSentence(int sent_num, std::vector<SRLInformation>*& ret); 
		SRLInfoReader(std::istream& input): input_(input){};

	protected:
		virtual SRLInformation ParseSRLInfo(const std::string& /*input*/);
	private:
		SRLInformation cache_;
		std::istream& input_;

	public:

	};


}


#endif /* SRLINFOPARSER_H_ */
