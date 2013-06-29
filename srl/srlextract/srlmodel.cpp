
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
using namespace std;
namespace srl
{

	double SRLEventModel::c_flooredScore = -500;

	boost::shared_ptr<SRLEventModel> SRLEventModel::GetBackoffEvent(){
		return m_backoff;
	}
	void SRLEventModel::SetBackoffEvent(boost::shared_ptr<SRLEventModel> bof){
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

	void SRLEventModelTrainer::InitTraining(bool forward,SRLEventModelSet & models){
		m_stats.clear();
		for(set<string>::const_iterator it = models.GetNames().begin(); it!= models.GetNames().end(); it++){
			boost::shared_ptr<SRLEventModel> model_ptr = models.GetModel(*it);
			if(model_ptr.get() && model_ptr->IsForwardFeature() == forward ){
				m_stats.push_back(ModelStatistics(model_ptr));
			}
		}
	}

	void SRLEventModelTrainer::AddCount(const SRLHypothesis& h, double count){
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
		vector<string> modeldefs;
		vector<string> modeldeps;
		vector<string> modelparts;

		map<string, string> modelnames;

		if(!modeldefs.size()){
			throw invalid_argument("Invalid model parameter for SRL modelset");
		}
		for(string_split_iterator it=
			boost::make_split_iterator(strModelDef, boost::first_finder("|||", boost::is_iequal()));
			it!=string_split_iterator();	++it)
		{
			string imodel = "";
			for(string_split_iterator jt=
				boost::make_split_iterator(boost::copy_range<string>(*it), boost::first_finder("-->", boost::is_iequal()));
				it!=string_split_iterator();	++jt)
			{
				if(imodel.length() == 0){
					imodel = boost::trim_copy(boost::copy_range<string>(*jt));
					continue;
				}
				string nmodel = boost::trim_copy(boost::copy_range<string>(*jt));

				map<string, string>::iterator it = modelnames.find(imodel);
				if(it!=modelnames.end()){
					if(it->second == nmodel)
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
		for(map<string, string>::iterator it = modelnames.begin() ; it!=modelnames.end() ;it++){
			// params
			boost::split(modelparts, it->first, boost::is_any_of(" \t"), boost::algorithm::token_compress_on);
			boost::shared_ptr<SRLEventModel> model;
			if(modeldefs.size() == 0){
				// Creat
			}
			models[it->first] = model;
			//boost::shared_ptr<SRLEventModel> model(new SRLEventModel());
		}
		//(Link)
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
			}
		}

		for(map<string, boost::shared_ptr<SRLEventModel> >::iterator kt; kt!= models.end(); kt++){
			modelset->SetModel(kt->second, kt->second.get() ? false : true);			
		}

		return modelset;
	}












	//////////////// Model implementations //////////////

	void PredicateGivenSourceWordModel::SerializeAssociated(ostream &ostr){
	}

	void PredicateGivenSourceWordModel::DeSerializeAssociated(ostream &ostr){
	}
		
	int PredicateGivenSourceWordModel::GetEventID(const SRLHypothesis& hyp){
	}

	int PredicateGivenSourceWordModel::GetEventMarginalID(const SRLHypothesis& hyp){
	}
}


