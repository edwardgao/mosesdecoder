
#ifndef __SRL_BRIDGE_H
#define __SRL_BRIDGE_H
/// Overview: This file is for SRL Model storage and representation
#include <vector>
#include <set>
#include <iostream>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/shared_ptr.hpp>
#include <memory>
#include "srlinfo.h"
#include "srlmodel.h"
#include "tables-core.h"
#include "PhraseAlignment.h"



namespace srl{

	void TrainSRLModel(std::istream& extractFileP, SRLEventModelTrainer& trainer);


	/// A bridge of SRL frame & SRL Hypothesis
	class SRLHypothesisFromPhraseTableFrame : public SRLHypothesis{
	protected:
		mutable boost::shared_ptr<TProvidedFramesForPhrases> m_frames;
		mutable boost::shared_ptr<std::vector<std::string> > m_src_phrases, m_tgt_phrases, m_tgt_phrases_noNT, m_src_phrases_noNT;
		mutable boost::shared_ptr<std::vector<NT> > m_src_phrases_nt, m_tgt_phrases_nt;
		MosesTraining::PhraseAlignment& m_phrase_aligment;

		void generateAll() const;
	public:
		SRLHypothesisFromPhraseTableFrame(boost::shared_ptr<TProvidedFramesForPhrases> &frames, 
			MosesTraining::PhraseAlignment& pl) : m_frames(frames), m_phrase_aligment(pl) {};
		
		virtual const std::vector<std::string>& GetHypothesisTokenSequence() const ;

		virtual const std::vector<SRLHypothesis::NT>& GetAllNonTerminals() const;

		virtual const std::vector<std::string>& GetTokenSequenceWithoutNT() const;

		virtual inline const std::vector<SRLFrame>& GetProvidedFrames() const {return *m_frames;};

		virtual const std::vector<std::string>& GetSourceTokenSequnce() const;

		virtual const std::vector<std::string>& GetSourceTokenSequnceWithoutNT() const;

	};
}

#endif