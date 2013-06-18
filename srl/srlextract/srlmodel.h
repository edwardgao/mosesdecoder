#ifndef __SRL_MODEL_H
#define __SRL_MODEL_H
/// Overview: This file is for SRL Model storage and representation
#include <vector>
#include <set>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <memory>
#include "srlinfo.h"

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
		we always assume the tokens are mapped to some sort of ids to avoid dealing with different types. It should
		contain all the tokens including those provided by non-terminals*/
		virtual std::vector<int>& GetHypothesisTokenSequence() = 0;

		/*! It should return all the non-terminals that DIRECTLY (not nested) provided in the hypothesis, see definition of NT for
		reference*/
		virtual std::vector<NT>& GetAllNonTerminals() = 0;
		
		/*! Get all the new tokens provided directly in the hypothesis, the returned value should be an array of indices, pointing
		to relative position in the array returned by GetHypothesisTokenSequence*/
		virtual std::vector<size_t>& GetNewTokenSequence() = 0;

		/*! Get the SRL structures provided by **New TOKENS** */
		virtual std::vector<SRLFrame>& GetProvidedFrames() = 0;
	};

	/*! The concept of SRL event is that given an SRL frame, extract corresponding value of a particular event.
	It is very like a dep tree, but it is a tree-structure back-off, pointing from leaf to the root
	Beware: Everything assumed log domain, so backoff.
	*/
	class SRLEventModel{
	private:
		SRLEventModel* m_backoff;
	public:
		static double c_flooredScore;
		// Get back-off event, if no backoff event, return NULL. A stub is provided
		virtual SRLEventModel* GetBackoffEvent(); 
		virtual void SetBackoffEvent(SRLEventModel* bof);
		
		// Cntr/Dstr
		SRLEventModel() : m_backoff(NULL) {};
		SRLEventModel(SRLEventModel* backoff) : m_backoff(backoff) {};
		virtual ~SRLEventModel(){};


		// Given an SRL Hypothesis, it should provide two scores: direct scored and backed off
		// These two are not usually called separately. Note that only direct score is made public
		virtual double GetModelScore(const SRLHypothesis& hyp);

		// These are for the trainers:
		virtual int GetEventID(const SRLHypothesis& hyp) = 0; // Get distinguishable Event ID, this is used to collect statistics
		virtual int GetEventMarginalID(const SRLHypothesis& hyp)  = 0; // Get distinguishable Event ID, this is used to collect statistics
		// The statistics works like this: the MLE collects count of each EventMarginalID as #EM, and for each event id #id, 
		// an unique event will be the combination of EventId and EventMarginal ID.
		// And the marginal likelihood will be #id/#em.

		virtual std::string& EventTypeName(); // Type name of the event


	protected:
		// Get backoff discount, this is usually called by children
		virtual double GetBackOffDiscount(const SRLHypothesis& hyp) = 0;

		// Get direct score, if need to backoff, return false
		virtual bool GetDirectScore(const SRLHypothesis& hyp, double& score) = 0;
	};


	/*!
	A set of SRL Event models and its tree structure. The objects are stored in auto_ptr, therefore, make sure 
	it is created using "new" operator and do not delete unless it is released
	*/
	class SRLEventModelSet{
	protected:

		friend class SRLEventModelTrainer;
		boost::unordered_map<std::string, std::auto_ptr<SRLEventModel> > m_srlmodels;
		std::set<SRLEventModel* > m_leaves;
		std::set<std::string> m_names;
	public:
		/// Release 
		SRLEventModel* ReleaseModel(const std::string& name);
		/// Only Get
		SRLEventModel* GetModel(const std::string& name);
		/// Add, release the old model if there is
		SRLEventModel* SetModel(SRLEventModel* em, bool isLeaf);

		inline const std::set<std::string> GetNames() const {return m_names;}
		inline const std::set<SRLEventModel* > GetLeaves() const {return m_leaves;};
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
			SRLEventModel* Model;
			boost::unordered_map<int, MarginalStatistics> ModelCount;
			ModelStatistics(SRLEventModel* model): Model(model) {};

			void AddCount(const SRLHypothesis& h, double count);
		};
	public:
		// Configures
		std::string SortedInputFile;
		std::string SortedReversedInputFile;
		std::string OutModelFile;

		SRLEventModelSet* ModelSet;

		// Do training
		bool Train();
	
	protected:
		/*TODO: Implement IO
		bool Normalize(); // Normalize all models
		bool WriteModelFile();*/
	};
}


#endif