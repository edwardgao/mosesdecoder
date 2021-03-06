
#include "srlmodel.h"
#include <cmath>
#include <assert.h>
#include <algorithm>
#include <sstream>
#include "boost/tuple/tuple.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/compare.hpp>
#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <limits>
using namespace std;
namespace srl
{


	boost::shared_ptr<SRLEventModel> SRLEventModel::GetBackoffEvent(){
		return m_backoff;
	}
	void SRLEventModel::SetBackoffEvent(boost::shared_ptr<SRLEventModel> bof){
		m_backoff = bof;
	}

	double SRLEventModel::GetModelScore(const SRLHypothesis& hyp){
		double score;
		switch (ScoreCombineType)
		{
		case srl::SRLEventModel::COMBINE_ADD:
		case srl::SRLEventModel::COMBINE_MEAN:
			score = 0;
			break;
		case srl::SRLEventModel::COMBINE_GEOMEAN:
		case srl::SRLEventModel::COMBINE_MUL:
			score = 1.0;
			break;
		case srl::SRLEventModel::COMBINE_MAX:
			score = numeric_limits<double>::min();;
			break;
		case srl::SRLEventModel::COMBINE_MIN:
			score = numeric_limits<double>::max();
			break;
		default:
			throw srl_model_exception("Unknow combine type");
			break;
		}

		
		double tempscore;
		vector<EventID_TYPE> seq = GetEventID(hyp);

		for(int i = 0; i < seq.size(); i++){
			if(!GetDirectScore(seq[i].first, seq[i].second, tempscore)){
				//
			}else{
				if(!m_backoff){
					tempscore =  c_flooredScore;
				}else{
					tempscore = m_backoff->GetModelScore(hyp) + m_backoff->GetBackOffDiscount(hyp);
				}
			}
			switch (ScoreCombineType)
			{
			case srl::SRLEventModel::COMBINE_ADD:
			case srl::SRLEventModel::COMBINE_MEAN:
				score += tempscore;
				break;
			case srl::SRLEventModel::COMBINE_GEOMEAN:
			case srl::SRLEventModel::COMBINE_MUL:
				score *= tempscore;
				break;
			case srl::SRLEventModel::COMBINE_MAX:
				score = max(score, tempscore);
				break;
			case srl::SRLEventModel::COMBINE_MIN:
				score = min(score, tempscore);
				break;
			default:
				throw srl_model_exception("Unknow combine type");
				break;
			}
			if(score < c_flooredScore)
				score = c_flooredScore;
		}
		return score;
	}


	boost::shared_ptr<SRLEventModel> SRLEventModelSet::ReleaseModel(const std::string& name){
		boost::unordered_map<string, boost::shared_ptr<SRLEventModel> >::iterator it = m_srlmodels.find(name);
		if(it == m_srlmodels.end()){
			return boost::shared_ptr<SRLEventModel>();
		}
		boost::shared_ptr<SRLEventModel> ret(it->second);
		for(set<boost::shared_ptr<SRLEventModel> >::iterator i = m_leaves.begin() ; i != m_leaves.end() ; i++){
			if(*i == ret){
				m_leaves.erase(i);
				break;
			}
		}
		m_names.erase(name);
		return ret;
	}

	boost::shared_ptr<SRLEventModel> SRLEventModelSet::GetModel(const std::string& name){
		boost::unordered_map<string, boost::shared_ptr<SRLEventModel> >::iterator it = m_srlmodels.find(name);
		if(it == m_srlmodels.end()){
			return boost::shared_ptr<SRLEventModel>();
		}
		return it->second;
	}

	boost::shared_ptr<SRLEventModel> SRLEventModelSet::SetModel(boost::shared_ptr<SRLEventModel> em, bool isLeaf){
		string name = em->EventTypeName();
		boost::shared_ptr<SRLEventModel> ret = ReleaseModel(name);
		m_srlmodels[name] = boost::shared_ptr<SRLEventModel>(em);
		m_names.insert(name);
		if(isLeaf){
			m_leaves.insert(boost::shared_ptr<SRLEventModel>(em));
		}
		return ret;
	}


	void SRLEventModelSet::SerializeAll(ostream& ofs){
		for(boost::unordered_map<std::string, boost::shared_ptr<SRLEventModel> >::iterator it
			= m_srlmodels.begin(); it != m_srlmodels.end(); it++){
				it->second->Serialize(ofs);
		}
	}

	void SRLEventModelTrainer::ModelStatistics::AddCount(const SRLHypothesis* h, double count){

		std::vector<std::pair<int, int> > eventId = Model->GetEventID(*h);
		for(std::vector<std::pair<int, int> >::iterator jt  = eventId.begin(); jt!= eventId.end(); jt++){
			int marginalId = jt->second;
			int eventid = jt->first;

			boost::unordered_map<int, MarginalStatistics>::iterator it = ModelCount.find(marginalId);
			if(it == ModelCount.end()){
				pair<boost::unordered_map<int, MarginalStatistics>::iterator, bool> ipa = ModelCount.insert(make_pair(marginalId, MarginalStatistics(marginalId)));
				it = ipa.first;
				it->second.Count = count;
			}else{
				it->second.Count += count;
			}
			boost::unordered_map<int, double>::iterator iit = it->second.MarginalCount.find(eventid); 
			if(iit == it->second.MarginalCount.end()){
				it->second.MarginalCount.insert(make_pair(eventid, count));
			}else{
				iit->second += count;
			}

		}

		
	}

	void SRLEventModelTrainer::ModelStatistics::Normalize(){
		boost::unordered_map<int, MarginalStatistics>::iterator it;
		for(it = ModelCount.begin(); it != ModelCount.end(); it++){
			double total = 0;
			boost::unordered_map<int, double>::iterator jt;
			for(jt = it->second.MarginalCount.begin(); jt!= it->second.MarginalCount.end() ; jt++){
				total += jt->second;
			}
			assert(total>0);
			for(jt = it->second.MarginalCount.begin(); jt!= it->second.MarginalCount.end() ; jt++){
				jt->second /= total;
				this->Model->SetDirectScore(it->first, jt->first, jt->second);
			}
		}

	}

	bool SRLEventModelTrainer::InitTraining(bool forward,boost::shared_ptr<SRLEventModelSet> models){
		m_stats.clear();
		m_origin_model = models;
		bool bHasModel = false;
		for(set<string>::const_iterator it = models->GetNames().begin(); it!= models->GetNames().end(); it++){
			boost::shared_ptr<SRLEventModel> model_ptr = models->GetModel(*it);
			if(model_ptr.get() && (forward == model_ptr->IsForwardFeature()) ){
				m_stats.push_back(ModelStatistics(model_ptr));
				bHasModel = true;
			}
		}
		return bHasModel;
	}

	void SRLEventModelTrainer::AddCount(const SRLHypothesis* h, double count){
		for(vector<ModelStatistics>::iterator it = m_stats.begin(); it!= m_stats.end(); it++){
			it->AddCount(h, count);
		}
	}

	void SRLEventModelTrainer::Normalize(){
		for(vector<ModelStatistics>::iterator it = m_stats.begin(); it!= m_stats.end(); it++){
			it->Normalize();
		}
	}

	inline bool _less_than_tuple(const boost::tuple<int, int, double>& x, const boost::tuple<int, int, double>& y){
		if(x.get<0>() < y.get<0>())
			return true;
		else if(x.get<0>() == y.get<0>() && x.get<1>() < y.get<1>())
			return true;
		return false;
	}
	// Serialize the model
	void SRLEventModel::Serialize(std::ostream &ostr){
		ostr << "@@BEGIN:" <<this->EventTypeName() << "@@" << endl;

		ostr << "@@HEADER:" <<this->EventTypeName() << "@@" << endl;

		this->SerializeAssociated(ostr);

		ostr << "@@DATA:" <<this->EventTypeName() << "@@" << endl;
		// Sort
		vector<boost::tuple<int, int, double> > entries;
		entries.reserve(m_ModelStore.size());
		for(ModelStoreType::iterator it = m_ModelStore.begin(); it!= m_ModelStore.end(); it++){
			entries.push_back(boost::tuple<int,int,double>(it->first.first, it->first.second, it->second));			
		}
		sort(entries.begin(), entries.end(), _less_than_tuple);

		for(vector<boost::tuple<int, int, double> >::iterator jt = entries.begin(); jt!=entries.end(); jt++){
			ostr << jt->get<0>() << " " << jt->get<1>() << " " << jt->get<2>() << endl;
		}
		
		ostr << "@@END:" <<this->EventTypeName() << "@@" << endl;
	}

	//
	void SRLEventModel::Deserialize(std::istream &istr){
	}

	boost::shared_ptr<SRLEventModelSet> SRLEventModelSet::Construct(std::istream& strModelDef)
	{
		string full;
		string line;
		while(strModelDef){
			line = "";
			std::getline(strModelDef, line);
			if(line.length()){
				if(full.length())
					full += "|||";
				full+=line;
			}
		}

		return Construct(full);
	}

	boost::shared_ptr<SRLEventModelSet> SRLEventModelSet::Construct(std::string strModelDef){
		boost::shared_ptr<SRLEventModelSet> modelset(new SRLEventModelSet());
		typedef boost::split_iterator<string::iterator> string_split_iterator;
		vector<string> modelparts;

		map<string, string> modelnames;

		for(string_split_iterator it=
			boost::make_split_iterator(strModelDef, boost::first_finder("|||", boost::is_iequal()));
			it!=string_split_iterator();	++it)
		{
			string imodel = "";
            string temp = boost::copy_range<string>(*it);
			for(string_split_iterator jt=
				boost::make_split_iterator(temp, boost::first_finder("-->", boost::is_iequal()));
				jt!=string_split_iterator();	++jt)
			{
				if(imodel.length() == 0){
					imodel = boost::trim_copy(boost::copy_range<string>(*jt));
					continue;
				}
				string nmodel = boost::trim_copy(boost::copy_range<string>(*jt));

				map<string, string>::iterator kt = modelnames.find(imodel);
				if(kt!=modelnames.end()){
					if(kt->second == nmodel)
						continue;
					else
					{
						string emessage = "Invalid model parameter for SRL modelset: Duplicated model with different back-offs";
						emessage += imodel;
						throw invalid_argument("Invalid model parameter for SRL modelset");
					}
				}else{
					modelnames.insert(make_pair(boost::trim_copy(imodel), boost::trim_copy(nmodel)));
				}
			}
			if(imodel.size()){
				string nmodel = "";

				map<string, string>::iterator it = modelnames.find(imodel);
				if(it==modelnames.end()){
					modelnames.insert(make_pair(boost::trim_copy(imodel), boost::trim_copy(nmodel)));
				}				
			}
		}

		map<string, boost::shared_ptr<SRLEventModel> > models;
		// Create all
		boost::shared_ptr<SRLEventModel> model;
		for(map<string, string>::iterator it = modelnames.begin() ; it!=modelnames.end() ;it++){
			// params
			boost::split(modelparts, it->first, boost::is_any_of(" \t"), boost::algorithm::token_compress_on);			
			if(it->first == "PredicateGivenSourceWord")  // Create models
				model.reset(new PredicateGivenSourceWordModel(false));
			models[it->first] = model;			
		}
		//(Link)
		set<string> non_leaf;
		for(map<string, string>::iterator it = modelnames.begin() ; it!=modelnames.end() ;it++){
			if(it->second.length() > 0){
				map<string, boost::shared_ptr<SRLEventModel> >::iterator jt = models.find(it->second);
				if(jt == models.end()){
					string emessage = "Error, the model is not defined: " ;
					emessage = emessage + it->second;
					throw invalid_argument(emessage);
				}
				map<string, boost::shared_ptr<SRLEventModel> >::iterator kt = models.find(it->first);
				assert(kt != models.end());
				kt->second->SetBackoffEvent(jt->second);
				non_leaf.insert(jt->first);
			}
		}

		for(map<string, boost::shared_ptr<SRLEventModel> >::iterator kt = models.begin(); kt!= models.end(); kt++){

			bool is_leaf = (non_leaf.find(kt->first) == non_leaf.end());
			modelset->SetModel(kt->second, is_leaf);	
		}

		return modelset;
	}












	//////////////// Model implementations //////////////

	void PredicateGivenSourceWordModel::SerializeAssociated(ostream &ostr){
		m_dict.Serialize(ostr, "DICT");
	}

	void PredicateGivenSourceWordModel::DeSerializeAssociated(ostream &ostr){
	}
		
	vector<PredicateGivenSourceWordModel::EventID_TYPE> PredicateGivenSourceWordModel::GetEventID(const SRLHypothesis& hyp){
		vector<EventID_TYPE> ret;
		const vector<string>& freshtoken = hyp.GetSourceTokenSequnceWithoutNT();
		const vector<SRLFrame>& srlframes = hyp.GetProvidedFrames();

		for(vector<string>::const_iterator it = freshtoken.begin(); it!= freshtoken.end(); it++){
			int wid = m_dict.GetWordID_add(*it);
			for(vector<SRLFrame>::const_iterator jt = srlframes.begin(); jt!= srlframes.end(); jt++){
				int pid = jt->PredicateName;
				ret.push_back(EventID_TYPE(pid, wid));// count of #(PID | WID)
			}
		}
		return ret;
	}


}


