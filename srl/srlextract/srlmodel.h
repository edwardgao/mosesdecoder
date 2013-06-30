#ifndef __SRL_MODEL_H
#define __SRL_MODEL_H
/// Overview: This file is for SRL Model storage and representation
#include <vector>
#include <set>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/shared_ptr.hpp>
#include <memory>
#include "srlinfo.h"
#include "hdict.h"

namespace srl{

	/*! This interface is a contract between SRL event extractor and decoder/nbest hypothesis
	the decoder/nbest rescorer should subclass it and provide necessary information for SRL event
	classifier.	*/
	class SRLHypothesis{
	public:
		/*!A structure to represent a non-terminal*/
		struct NT{
			bool Expanded; // Whether it has been expanded?
			int  start; // Index of the starting word (inclusive)
			int  end; // Index of the ending word (inclusive)
			int  NTTag; // The non-terminal tag, again it is mapped to an integer;
			void* Reserved; // Reserved for additional information
		};

	public:
		/*! The implementation of the interface should return the token	sequence of the whole phrase/span it covers
		NOT NOW(we always assume the tokens are mapped to some sort of ids to avoid dealing with different types.) It should
		contain all the tokens including those provided by non-terminals*/
		virtual const std::vector<int>& GetHypothesisTokenSequence() const= 0;

		/*! It should return all the non-terminals that DIRECTLY (not nested) provided in the hypothesis, see definition of NT for
		reference*/
		virtual const std::vector<NT>& GetAllNonTerminals() const = 0;

		/*! Get all the new tokens provided directly in the hypothesis, the returned value should be an array of indices, pointing
		to relative position in the array returned by GetHypothesisTokenSequence*/
		virtual const std::vector<size_t>& GetTokenSequenceWithoutNT() const = 0;

		/*! Get the SRL structures provided by **New TOKENS** */
		virtual const std::vector<SRLFrame>& GetProvidedFrames() const = 0;


		/*! Get the source token sequence */
		virtual const std::vector<std::string>& GetSourceTokenSequnce() const = 0;

		virtual const std::vector<std::string>& GetSourceTokenSequnceWithoutNT() const = 0;
	};

	/*! The concept of SRL event is that given an SRL frame, extract corresponding value of a particular event.
	It is very like a dep tree, but it is a tree-structure back-off, pointing from leaf to the root
	Beware: Everything assumed log domain, so backoff.
	*/
	class SRLEventModel{
	private:
		boost::shared_ptr<SRLEventModel> m_backoff;
	public:

		enum TScoreCombineType{
			COMBINE_ADD,
			COMBINE_MUL,
			COMBINE_MAX,
			COMBINE_MIN,
			COMBINE_MEAN,
			COMBINE_GEOMEAN,
		} ;

		TScoreCombineType ScoreCombineType;
		typedef std::pair<int, int> EventID_TYPE;

		double c_flooredScore;
		// Get back-off event, if no backoff event, return NULL. A stub is provided
		virtual boost::shared_ptr<SRLEventModel> GetBackoffEvent(); 
		virtual void SetBackoffEvent(boost::shared_ptr<SRLEventModel> bof);

		// Cntr/Dstr
		SRLEventModel() : m_backoff() {};
		SRLEventModel(boost::shared_ptr<SRLEventModel> backoff) : m_backoff(backoff) {};
		virtual ~SRLEventModel(){};


		// Given an SRL Hypothesis, it should provide two scores: direct scored and backed off
		// These two are not usually called separately. Note that only direct score is made public
		virtual double GetModelScore(const SRLHypothesis& hyp);

		// These are for the trainers:
		
		// The statistics works like this: the MLE collects count of each EventMarginalID as #EM, and for each event id #id, 
		// an unique event will be the combination of EventId and EventMarginal ID.
		// And the marginal likelihood will be #id/#em.
		// The first number returned will be the event id, and the second event marginal id
		virtual std::vector<EventID_TYPE> GetEventID(const SRLHypothesis& hyp) = 0; // Get distinguishable Event ID, this is used to collect statistics


		virtual const std::string& EventTypeName() const {return m_eventName;}; // Type name of the event

		// Return whether this is a forward or backward feature (i.e. whether we should give a forward or backward phrase to it)
		virtual bool IsForwardFeature() const {return m_forward_feature;};

	protected:
		typedef boost::unordered_map<std::pair<int, int>, double> ModelStoreType;
		ModelStoreType m_ModelStore;
		std::string m_eventName;
		bool m_forward_feature;

	protected:
		// Get backoff discount, this is usually called by children
		virtual double GetBackOffDiscount(const SRLHypothesis& hyp) {return 0;}

		// Get direct score, if need to backoff, return false
		virtual bool GetDirectScore(int eventID, int margId, double& score){
			ModelStoreType::iterator it = m_ModelStore.find(std::make_pair(eventID,margId));
			if(it == m_ModelStore.end()){
				return false;
			}else
			{
				score = it->second;
				return true;
			}
		}


	public:
		// Basic parameter update routines & data structures
		// Get direct score, if need to backoff, return false
		virtual void SetDirectScore(int eventId, int margId, double score){
			ModelStoreType::iterator it = m_ModelStore.find(std::make_pair(eventId,margId));
			if(it == m_ModelStore.end()){
				m_ModelStore[std::make_pair(eventId,margId)] = score;
			}else
			{
				it->second = score;
			}
		}

		// Serialize the model
		virtual void Serialize(std::ostream &ostr);

		// Deserialize
		virtual void Deserialize(std::istream &istr);

	protected:
		virtual void SerializeAssociated(std::ostream &ostr) = 0;

		virtual void DeSerializeAssociated(std::ostream &ostr) = 0;
	};


	/*!
	A set of SRL Event models and its tree structure. The objects are stored in an array and WILL BE DELETED when destructed, therefore, make sure 
	it is created using "new" operator and do not delete unless it is released
	*/
	class SRLEventModelSet{
	protected:

		friend class SRLEventModelTrainer;
		boost::unordered_map<std::string, boost::shared_ptr<SRLEventModel> > m_srlmodels;
		std::set<boost::shared_ptr<SRLEventModel> > m_leaves;
		std::set<std::string> m_names;
	public:
		/// Release 
		boost::shared_ptr<SRLEventModel> ReleaseModel(const std::string& name);
		/// Only Get
		boost::shared_ptr<SRLEventModel> GetModel(const std::string& name);
		/// Add, release the old model if there is
		boost::shared_ptr<SRLEventModel> SetModel(boost::shared_ptr<SRLEventModel> em, bool isLeaf);

		inline const std::set<std::string> GetNames() const {return m_names;}
		inline const std::set<boost::shared_ptr<SRLEventModel> > GetLeaves() const {return m_leaves;};

	public:

		/// Create a modelset given string-based model definition, below is the definition of the model definition
		/// FEAT_NAME PARAMS --> BACK_OFF_FEAT_NAME PARAMS ||| FEAT_NAME PARAMS --> BACK_OFF_FEAT_NAME PARAMS ...
		static boost::shared_ptr<SRLEventModelSet> Construct(std::string strModelDef);

		static boost::shared_ptr<SRLEventModelSet> Construct(std::istream& strModelDef);

	};
	/*! The trainer for SRL (Tree)
	*/	
	class SRLEventModelTrainer{		
	private:
		// Statistics. All statistics are collected for each model, each marginal id and each marginal id + event id
		struct MarginalStatistics{
			int MarginalID;
			double Count;
			boost::unordered_map<int, double> MarginalCount;
			MarginalStatistics() : MarginalID(-1), Count(0) {};
			MarginalStatistics(int id) : MarginalID(id), Count(0) {};
		};

		struct ModelStatistics{
			boost::shared_ptr<SRLEventModel> Model;
			boost::unordered_map<int, MarginalStatistics> ModelCount;
			ModelStatistics(boost::shared_ptr<SRLEventModel>& model): Model(model) {};			
			void AddCount(const SRLHypothesis& h, double count);
			// Normalize and update the model parameters. After normalizing the associated model parameter will be updated too
			void Normalize();
		};

		std::vector<ModelStatistics> m_stats;
	public:

		// Initialize
		void InitTraining(bool forward,SRLEventModelSet & models);

		// Each event
		void AddCount(const SRLHypothesis& h, double count);

		// Normalize Model
		void Normalize();


	protected:

	};


	///////////////////////////////////
	// A number of events


	/// Most simple one, given source word, how likely it is associated with a certail predicate
	class PredicateGivenSourceWordModel : public SRLEventModel{

	protected:

		HDICT m_dict; 

	public:

		inline bool LockedVocab() const {return m_dict.LockedVocab();}

		virtual void SerializeAssociated(std::ostream &ostr);

		virtual void DeSerializeAssociated(std::ostream &ostr);
		
		virtual std::vector<EventID_TYPE> GetEventID(const SRLHypothesis& hyp); // Get distinguishable Event ID, this is used to collect statistics


		PredicateGivenSourceWordModel(bool bLockVocab): m_dict(bLockVocab)  {ScoreCombineType = COMBINE_MEAN; c_flooredScore = 1e-10;}

		virtual ~PredicateGivenSourceWordModel() {}
	};

}


#endif
