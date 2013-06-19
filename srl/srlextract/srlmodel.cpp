
#include "srlmodel.h"
#include <cmath>

using namespace std;
namespace srl
{

	double SRLEventModel::c_flooredScore = -500;

	SRLEventModel* SRLEventModel::GetBackoffEvent(){
		return m_backoff;
	}
	void SRLEventModel::SetBackoffEvent(SRLEventModel* bof){
		m_backoff = bof;
	}

	double SRLEventModel::GetModelScore(const SRLHypothesis& hyp){
		double score = 0;
		if(GetDirectScore(hyp, score)){
			return score;
		}else{
			if(!m_backoff){
				return c_flooredScore;
			}else{
				return m_backoff->GetModelScore(hyp) + m_backoff->GetBackOffDiscount(hyp);
			}
		}
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
	
    boost::shared_ptr<SRLEventModel> SRLEventModelSet::SetModel(SRLEventModel* em, bool isLeaf){
		string name = em->EventTypeName();
        boost::shared_ptr<SRLEventModel> ret = ReleaseModel(name);
		m_srlmodels[name] = boost::shared_ptr<SRLEventModel>(em);
		m_names.insert(name);
		if(isLeaf){
			m_leaves.insert(boost::shared_ptr<SRLEventModel>(em));
		}
		return ret;
	}


	bool SRLEventModelTrainer::Train(){

		// Read all events and "AddCount"
		return true; // Read file etc
	}

	void SRLEventModelTrainer::ModelStatistics::AddCount(const SRLHypothesis& h, double count){
		int marginalId = Model->GetEventMarginalID(h);
		int eventid = Model->GetEventID(h);
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
