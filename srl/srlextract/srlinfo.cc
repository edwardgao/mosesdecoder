#include <string>
#include <vector>
#include <list>
#include <map>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include "stringalgo.hh"

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
				for(int j = regions_[i].first; j< regions_[i].second; j++){ // The region, however is not inclusive, what a mess
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

	bool SRLInformation::MapIndices(const vector<std::string>& origin_sentence, vector<int>& index_map, vector<int>& reversed_map, bool conservative){
		if(conservative)
			return MapIndices_conserv(origin_sentence, index_map, reversed_map);
		else // Just do string distance
		{
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
			levenshtein_distance(srlsentlc, sentlc, reversed_map,index_map);
			
		}
	}

	bool SRLInformation::MapIndices_conserv(const vector<std::string>& origin_sentence, vector<int>& index_map, vector<int>& reversed_map){
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
		reversed_map.clear();
		reversed_map.resize(nwordsrl,-1);
		

		for(j =0; j< nwordsrl && k < nwordori; j++,k++){
			int orik= k;
			while(k<nwordori){
				if(srlsentlc[j] == sentlc[k]){
					index_map[k] = j;
					reversed_map[j] = k;
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
							reversed_map[l] = orik;
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

	unordered_map<string, TPredicateType> SRLFrame::id_map;

	TPredicateType SRLFrame::MapPredicateName(std::string& pstr){
		static boost::mutex scoped_mutex;
		boost::mutex::scoped_lock(scoped_lock);
		
		unordered_map<string, TPredicateType>::iterator it = id_map.find(pstr);
		if(it!=id_map.end())
			return it->second;
		TPredicateType ret ;
		id_map[pstr] = (ret = id_map.size());
		return ret;
	}


	unordered_map<string, TArgumentType> SRLArgument::id_map;

	TArgumentType SRLArgument::MapAugumentName(std::string& pstr){
		static boost::mutex scoped_mutex;
		boost::mutex::scoped_lock(scoped_lock);
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
		for(int j = region.first; j<region.second; j++){
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
		const std::vector<int>& indexMap, const std::vector<int>& indexRevMap, const std::vector<std::pair<int, int> >* HolesInclusive, 
		int start, int end, const TagClassifier& cls, bool lc, bool stem)	{
			SRLFrame ret ;
			int rStart = -1, rEnd = -1;
			// Map the indexes
			if(indexMap.size() <= start || indexMap.size() <= end ){
				cerr << "[WARNING] No map found\n";
				return ret; 
			}else if(indexMap[start] == -1 || indexMap[end] == -1){
				int i;
				for(i = start; i <= end; i++){
					if(indexMap[i] != -1){
						rStart = indexMap[i];
						break;
					}
				}
				for(int j = end; j >= i; j--){
					if(indexMap[j] != -1){
						rEnd = indexMap[j];
						break;
					}
				}
				if(rStart == -1){
					cerr << "[WARNING] No map found\n";
					return ret;
				}
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
					if(HolesInclusive){
						for(int j = 0; j < HolesInclusive->size() ; j++){
							if(region.second <= (*HolesInclusive)[j].second && region.first>= (*HolesInclusive)[j].first){
								predicate_in_hole = j;
								break;
							}
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
					if(HolesInclusive){
						for(int j = 0; j < HolesInclusive->size() ; j++){
							if(region.second <= (*HolesInclusive)[j].second && region.first>= (*HolesInclusive)[j].first){
								arg.ArgumentLocation = SetHoleNonterminalIndex(arg.ArgumentLocation, j);
							}
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


	string FramesToString(const TProvidedFramesForPhrases& srls){
		stringstream ret;
		for(int i = 0; i< srls.size() ; i++){
			const SRLFrame &frame = srls[i];
			ret << frame.PredicateName << " " << frame.PredicateIndex << " "
				<< frame.PredicateLocation << " ";
			for(int j = 0; j<frame.Arguments.size(); j++){
				ret << frame.Arguments[j].ArgumentName << " " << frame.Arguments[j].ArgumentLocation<<" ";
			}
			if(i<srls.size()-1)
				ret << ",,, ";
		}
		return ret.str();
	}

	void StringToFrame(const std::string str, TProvidedFramesForPhrases& srls){
		list<string> tokens;
		boost::split(tokens, str, boost::is_any_of(" "), boost::algorithm::token_compress_on);
		int status = 0;
		for(list<string>::iterator it = tokens.begin(); it!= tokens.end(); it++){
			if(*it == ",,,"){
				status = 0;
				continue;
			}
			switch(status){
			case 0:
				srls.push_back(SRLFrame());
				srls.back().PredicateName = lexical_cast<int>(*it);
				status = 1;
				break;
			case 1:
				srls.back().PredicateIndex = lexical_cast<int>(*it);
				status = 2;
				break;
			case 2:
				srls.back().PredicateLocation = (PredicatePlacement)lexical_cast<int>(*it);
				status = 3;
				break;
			case 3:
				srls.back().Arguments.push_back(SRLArgument());
				srls.back().Arguments.back().ArgumentName = lexical_cast<int>(*it);
				status = 4;
				break;
			case 4:
				srls.back().Arguments.back().ArgumentLocation = (ArgumentPlacement)lexical_cast<int>(*it);
				status = 3;
				break;
			}
		}
	}

	template<typename T>
	void SaveMapToFile(ostream& ofs, const T& vmap){
		map<int, string> revMap;
		for(typename T::const_iterator it = vmap.begin(); it!=vmap.end(); it++){
			revMap[it->second] = it->first;
		}
		for(map<int, string>::iterator it = revMap.begin(); it!=revMap.end(); it++){
			ofs << it->second << " " << it->first << endl;
		}
	}

	template<typename T>
	bool LoadMapFromFile(istream& ifs, T& vmap){
		string tag;
		int id;
		while(ifs){
			ifs >> tag >> id;
			vmap[tag] = id;
		}
		return true;
	}

	void SRLArgument::SaveMapping(std::ostream& ofs){
		SaveMapToFile(ofs, id_map);
	}

	void SRLFrame::SaveMapping(std::ostream& ofs){
		SaveMapToFile(ofs, id_map);
	}

	bool SRLArgument::LoadMapping(std::istream& ifs){
		return LoadMapFromFile(ifs, id_map);
	}

	bool SRLFrame::LoadMapping(std::istream& ifs){
		return LoadMapFromFile(ifs, id_map);
	}
}
