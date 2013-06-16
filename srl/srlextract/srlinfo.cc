#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

#include "srlinfo.h"

namespace srl {

	using namespace std;

	SRLInformation::SRLInformation() {
		error_ = false;
		Init();
	}

	SRLInformation::~SRLInformation() {

	}

	void SRLInformation::Init() {
		error_ = false;
		label_.clear();
		regions_.clear();
		words_.clear();
	}

	bool SRLInformation::MapIndices(const vector<std::string>& origin_sentence, vector<int>& index_map, vector<int>* reversed_map){
		//cerr << "SRLSize" << srl.size();
		vector<string> sentlc, srlsentlc;		
		int nwordori = origin_sentence.size();
		int nwordsrl = (int) GetNumberOfWords();
		sentlc.resize(nwordori);
		srlsentlc.resize(nwordsrl);
		for(int i=0; i< nwordori; i++)
			sentlc[i] = boost::to_lower_copy(origin_sentence[i]);
		for(int j=0; j<nwordsrl;j++){
			srlsentlc[j] = boost::to_lower_copy(GetWordAtIndex(j));			
		}
		return true;
	}
}
