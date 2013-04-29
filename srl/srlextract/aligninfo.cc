#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "aligninfo.h"

namespace srl {



using namespace std;
using namespace boost;
using namespace boost::tuples;

AlignmentInfoReader::AlignmentInfoReader(std::istream& english,
		std::istream& french, std::istream& align) :
	english_(english), french_(french), align_(align) {
}

tuple<bool, SentencePair, Alignment> AlignmentInfoReader::Read() {
	string eng, fre, aln;
	vector<string> idx;
	SentencePair sent;
	Alignment alignment;
	getline(english_, eng);
	getline(french_, fre);
	getline(align_, aln);
	if (eng.length() == 0 || fre.length() == 0 || aln.length() == 0) {
		return tuple<bool, SentencePair, Alignment> (false, sent,
				alignment);
	}
	trim(eng);
	trim(fre);
	trim(aln);
	split(sent.english_.words_, eng, is_any_of(" \t"), token_compress_on);
	split(sent.french_.words_,fre,is_any_of(" \t"), token_compress_on);
	split(idx, aln, is_any_of(" \t"), token_compress_on);

	alignment.aligned_to_french_.resize(sent.french_.words_.size());
	alignment.aligned_to_english_.resize(sent.english_.words_.size());
	for (size_t i = 0; i < idx.size(); i++) {
		vector<string> nv;
		split(nv, idx[i], is_any_of("-"), token_compress_on);
		if (nv.size() < 2) {
			cerr << "Warning, bad alignment " << idx[i] << endl;
			continue;
		}
		int eindex = boost::lexical_cast<int>(nv[0]);
		int findex = boost::lexical_cast<int>(nv[1]);
		if(eindex>=(int)sent.english_.length() || findex>=(int)sent.french_.length()||eindex<0 || findex<0){
			cerr << "Warning, bad alignment, out of boundary\n";
		}
		alignment.aligned_to_english_[eindex].insert(findex);
		alignment.aligned_to_french_[findex].insert(eindex);
	}
	return tuple<bool, SentencePair, Alignment> (true, sent,
			alignment);
}

}

