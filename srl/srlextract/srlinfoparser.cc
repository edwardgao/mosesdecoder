/*
* srlinfoparser.cc
*
*  Created on: Nov 6, 2010
*      Author: qing
*/

#include <vector>
#include <iostream>
#include <list>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include "srlinfo.h"
#include "srlinfoparser.h"


namespace srl {

	using namespace boost;
	using namespace std;



	SRLInformation SRLInfoReader::ParseSRLInfo(const string& input){
		SRLInformation info;
		list<string> tokens;



		split(tokens,input,is_any_of(" \t"), token_compress_on);
		int iw=0,  tag_start,tag_end;
		bool inside_srl=false;
		string tagname = "";

		for(list<string>::const_iterator it = tokens.begin(); it!= tokens.end(); it++){

			const string& token = *it;
			int last = ((int)token.size()) -1;
			if(last<0) continue;
			if(token[last]==':'){
				info.sent_number_ = lexical_cast<int>(token.substr(0,last));
				continue;
			}
			if(token[0] == '['){ // begin tag
				if(inside_srl){
					cerr <<"Nested SRL Info" << endl;
					return SRLInformation();
				}
				inside_srl = true;
				if(token.length()>1)
					tagname = token.substr(1,last);
				else
					tagname = "";
				tag_start = iw;
			}else if(token[last]==']'){ // end tag
				if(!inside_srl){ // error
					cerr << "Unmatched parenthese!" << endl;
					return SRLInformation();
				}
				inside_srl = false;
				if(token.length()>1){
					string word = token.substr(0,last);
					iw++;
					info.words_.push_back(word);
				}
				tag_end = iw;
				info.label_.push_back(tagname);
				info.regions_.push_back(pair<int,int>(tag_start,tag_end));
			}else{
				if(inside_srl && tagname.length()==0){
					tagname = token;
				}else{
					info.words_.push_back(token);
					iw++;
				}
			}
		}
		if(inside_srl){
			cerr << "Unmatched paraenthes in the end!" << endl;
			return SRLInformation();
		}
		return info;
	}

	std::vector<SRLInformation> SRLInfoReader::ReadSentence(int sent_num){
		std::vector<SRLInformation> ret;
		ReadSentence(sent_num, ret);
		return ret;

	}

	void SRLInfoReader::ReadSentence(int sent_num, std::vector<SRLInformation>*& ret){
		ret = new std::vector<SRLInformation>();
		ReadSentence(sent_num, *ret);
	}

	void SRLInfoReader::ReadSentence(int sent_num, std::vector<SRLInformation>& ret){
		if(cache_.GetSentenceNumber()==sent_num){
			ret.push_back(cache_);
		}
		std::string str;
		while (true) {
			str = "";
			getline(input_, str);
			if (str.length() == 0) {
				return ;
			}
			cache_ = ParseSRLInfo(str);

			if (cache_.GetSentenceNumber() > sent_num)
				break;
			ret.push_back(cache_);
		}
	}

	

} // namespace srl

