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
		int j,k=0;
		index_map.clear();
		index_map.resize(nwordori,-1);
		if(reversed_map){
			reversed_map->clear();
			reversed_map->resize(nwordsrl,-1);
		}

		for(j =0; j< nwordsrl && k < nwordori; j++,k++){
			int orik= k;
			while(k<nwordori){
				if(srlsentlc[j] == sentlc[k]){
					index_map[k] = j;
					if(reversed_map)
						(*reversed_map)[j] = k;
					break;
				}
				k++;
			}
			if(k==nwordori){
				cerr << "Found an unmatched SRL alignment, trying to recover " << endl;

				string sword = "";
				int orj = j;
				while(j<nwordsrl){
					sword += srlsentlc[j];
					if(sword > sentlc[orik]){
						j = nwordsrl;
						break;
					}
					if(sword == sentlc[orik]){
						index_map[orik] = orj;
						for(int l = orj; l <= j; l++){
							if(reversed_map)
								(*reversed_map)[l] = orik;
						}
						k = orik;
						break;
					}
					j++;
				}
				if(j==nwordsrl){
					cerr << "Cannot recover, below is the sentence";
					for(int k = 0; k < (int)srlsentlc.size(); k++)
						cerr << srlsentlc[k] << " ";
					cerr << endl;
					return false;
				}
			}
		}
		return true;
	}
}
