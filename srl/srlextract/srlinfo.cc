#include <string>
#include <vector>
#include <map>
#include <boost/algorithm/string.hpp>
#include <boost/unordered_map.hpp>
#include <boost/thread.hpp>

#include "srlinfo.h"

namespace srl {

	using namespace std;
    using namespace boost;

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
		word_to_region_map_.clear();
		predicate_index = -1;
	}

	int SRLInformation::GetRegionCoveringWord(int word_index){
		if(word_to_region_map_.size()){
			return word_to_region_map_[word_index];
		}else{// Rebuild
			word_to_region_map_.resize(words_.size(),-1);
			for(int i=0; i< regions_.size(); i++){
				for(int j = regions_[i].first; j<= regions_[i].second; j++){
					word_to_region_map_[j] = i;
				}
			}
		}
		return word_to_region_map_[word_index];
	}

	set<int> SRLInformation::GetRegionCoveringWords(int start, int end){
		set<int> ret;
		for(int i = start; i<=end ; i++){
			int regionidex = GetRegionCoveringWord(i);
			if(regionidex > -1)
				ret.insert(regionidex);
		}
		return ret;
	}

	int SRLInformation::GetPredicateIndex(const TagClassifier& cls){
		if(predicate_index==-1){
			for(int i = 0; i< this->GetNumberOfLabels() ; i++){
				if(cls.IsPredicate(this->GetLabelAtIndex(i))){
					predicate_index = i;
					break;
				}
			}
		}
		return predicate_index;
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

	TPredicateType SRLFrame::MapPredicateName(std::string& pstr){
		static boost::mutex scoped_mutex;
		boost::mutex::scoped_lock(scoped_lock);
		static unordered_map<string, TPredicateType> id_map;
		unordered_map<string, TPredicateType>::iterator it = id_map.find(pstr);
		if(it!=id_map.end())
			return it->second;
		TPredicateType ret ;
		id_map[pstr] = (ret = id_map.size());
		return ret;
	}

	TArgumentType SRLArgument::MapAugumentName(std::string& pstr){
		static boost::mutex scoped_mutex;
		boost::mutex::scoped_lock(scoped_lock);
		static unordered_map<string, TArgumentType> id_map;
		unordered_map<string, TArgumentType>::iterator it = id_map.find(pstr);
		if(it!=id_map.end())
			return it->second;
		TArgumentType ret ;
		id_map[pstr] = (ret = id_map.size());
		return ret;
	}

	string SRLRuleExtractor::GetRep(SRLInformation &srlInfo, const pair<int,int>& region, bool lc, bool stemming)
	{
		string fullrep;
		for(int j = region.first; j<=region.second; j++){
			if(j>region.first)
				fullrep+="_";
			if(stem)
				fullrep += do_stemming(srlInfo.GetWordAtIndex(j));
			else
				fullrep += srlInfo.GetWordAtIndex(j);
		}
		if(lc)
			boost::to_lower(fullrep);
		return fullrep;
	}

	SRLFrame SRLRuleExtractor::ExtractSRLInfo(SRLInformation &srlInfo, 
		const std::vector<int>& indexMap, const std::vector<int>& indexRevMap, const std::vector<std::pair<int, int> >& HolesInclusive, 
		int start, int end, const TagClassifier& cls, bool lc, bool stem)	{
			SRLFrame ret ;
			int rStart, rEnd;
			// Map the indexes
			if(indexMap.size() <= start || indexMap.size() <= end || indexMap[start] == -1 || indexMap[end] == -1)		{
				cerr << "[WARNING] No map found";
				return ret; 
			}else{
				rStart = indexMap[start];
				rEnd = indexMap[end];
			}
			set<int> regions = srlInfo.GetRegionCoveringWords(rStart, rEnd);
			bool predicate_provide = false;

			for(set<int>::iterator i = regions.begin() ; i != regions.end(); i++){
				const pair<int, int>& region = srlInfo.GetRegionAtIndex(*i);
				const string& tag = srlInfo.GetLabelAtIndex(*i);
				if(cls.IsPredicate(tag)){ // Predicate
					string fullrep =GetRep(srlInfo, region, lc, stem);
					predicate_provide = true;
					// Is it in the hole?
					int predicate_in_hole = -1;
					for(int j = 0; j < HolesInclusive.size() ; j++){
						if(region.second <= HolesInclusive[j].second && region.first>= HolesInclusive[j].first){
							predicate_in_hole = j;
							break;
						}
					}
					ret.PredicateIndex = predicate_in_hole < 0 ? indexRevMap[region.first] - start : -1; // Relative index, in original phrase
					ret.PredicateLocation = (PredicatePlacement) (predicate_in_hole < 0 ? PP_PROVIDED : (PP_NT_PROVIDED + predicate_in_hole));
					ret.PredicateName = SRLFrame::MapPredicateName(fullrep);
				}else{
					string tagname = cls.MapToStandardName(tag);
					int tagid =SRLArgument::MapAugumentName	(tagname);
					ret.Arguments.push_back(SRLArgument());
					SRLArgument& arg = ret.Arguments.back(); // Last one
					arg.ArgumentName = tagid;
					// determine placements
					if(region.first<= rStart)
						arg.ArgumentLocation = (ArgumentPlacement) (arg.ArgumentLocation | AP_LEFT_COMPLETE);
					if(region.second>= rEnd)
						arg.ArgumentLocation = (ArgumentPlacement) (arg.ArgumentLocation | AP_RIGHT_COMPLETE);
					for(int j = 0; j < HolesInclusive.size() ; j++){
						if(region.second <= HolesInclusive[j].second && region.first>= HolesInclusive[j].first){
							arg.ArgumentLocation = SetHoleNonterminalIndex(arg.ArgumentLocation, j);
						}
					}
				}

			}

			if(!predicate_provide){ // Predicate not provided, let's see where it is
				int pred_idx = srlInfo.GetPredicateIndex(cls);
				const pair<int,int>& region = srlInfo.GetRegionAtIndex(pred_idx);
				string fullrep =GetRep(srlInfo, region, lc, stem);
				ret.PredicateIndex = -1;
				ret.PredicateLocation = region.first < rStart ? PP_LEFT_EXPECT : PP_RIGHT_EXPECT;
				ret.PredicateName = SRLFrame::MapPredicateName(fullrep);				
			}

			return ret;
	}

	string SRLRuleExtractor::do_stemming(const string& ori){
		boost::mutex::scoped_lock(m_stemming);
		char* ch = new char[ori.length()+1];
		string lc = boost::to_lower_copy(ori);
		strcpy(ch,lc.c_str());
		ch[stem(stemmer,ch,ori.length()-1)+1] = '\0';
		lc = ch;
		delete[] ch;
		return lc; 
	}

	TagClassifier TagClassifier::Instance;

	bool TagClassifier::IsPredicate(const string& str) const{
		return str == "TARGET";
	}

	string TagClassifier::MapToStandardName(const string& str) const{
		return str.substr(3,str.length()-3);
	}
}
