#include "srlbridge.h"
#include <cmath>
#include <iostream>
#include "SafeGetline.h"
#include "tables-core.h"
#include "PhraseAlignment.h"
#include "srlinfo.h"

using namespace std;

#define LINE_MAX_LENGTH 100000

using namespace MosesTraining;

namespace srl{

	void TrainSRLModel(istream& extractFileP, SRLEventModelTrainer& trainer){
		float lastCount = 0.0f;
		int i=0;
		char line[LINE_MAX_LENGTH];
		boost::shared_ptr<TProvidedFramesForPhrases> phraseframes(new TProvidedFramesForPhrases);
		boost::shared_ptr<SRLHypothesisFromPhraseTableFrame> hyp;
		while(true) {
			if (extractFileP.eof()) break;
			if (++i % 100000 == 0) cerr << "." << flush;
			SAFE_GETLINE((extractFileP), line, LINE_MAX_LENGTH, '\n', __FILE__);
			if (extractFileP.eof())	break;
			// create new phrase pair
			PhraseAlignment phrasePair;
			phrasePair.create( line, i, false, true);
			phraseframes->clear();
			StringToFrame(phrasePair.phraseSRLFrames, *phraseframes);

			hyp.reset(new SRLHypothesisFromPhraseTableFrame(phraseframes, phrasePair));

			trainer.AddCount(hyp.get(), phrasePair.count);
		}
		//processPhrasePairs( phrasePairsWithSameF, *phraseTableFile, isSingleton, featureManager, maybeLogProb );
	}


	inline void GetNTIndices(const vector<string>& tokens, const vector<size_t>& nts, vector<string>& tokNoNT, vector<SRLHypothesis::NT>& vnts){
		SRLHypothesis::NT nnt;
		set<int> ntpos;

		for(int j = 0; j< nts.size(); j++){
			ntpos.insert(nts[j]);
			nnt.start = nnt.end = j;
			nnt.Expanded = false;
			nnt.NTTag = 0; // Defautl
			nnt.Reserved = NULL;
			vnts.push_back(nnt);
		}

		for(int i = 0; i< tokens.size() ; i++){
			if(ntpos.find(i) == ntpos.end()){
				tokNoNT.push_back(tokens[i]);			
			}
		}
	}

	void SRLHypothesisFromPhraseTableFrame::generateAll() const{
		m_src_phrases.reset(new vector<string>());
		m_tgt_phrases.reset(new vector<string>());
		m_src_phrases_noNT.reset(new vector<string>());
		m_tgt_phrases_noNT.reset(new vector<string>());
		m_src_phrases_nt.reset(new std::vector<NT>());
		m_tgt_phrases_nt.reset(new std::vector<NT>());

		vector<size_t> nts;
		m_phrase_aligment.GetSourceRep(*m_src_phrases, nts);
		GetNTIndices(*m_src_phrases, nts, *m_src_phrases_noNT, *m_src_phrases_nt);
		m_phrase_aligment.GetTargetRep(*m_tgt_phrases, nts);
		GetNTIndices(*m_tgt_phrases, nts, *m_tgt_phrases_noNT, *m_tgt_phrases_nt);
	}
	
	const vector<string>& SRLHypothesisFromPhraseTableFrame::GetHypothesisTokenSequence() const {
		if(!m_tgt_phrases.get()){
			generateAll();
		}
		return *m_tgt_phrases;
	}
	const vector<SRLHypothesis::NT>& SRLHypothesisFromPhraseTableFrame::GetAllNonTerminals() const{
		if(!m_tgt_phrases_nt.get())
			generateAll();
		return *m_tgt_phrases_nt;
	}

	const vector<string>& SRLHypothesisFromPhraseTableFrame::GetTokenSequenceWithoutNT() const{
		if(!m_tgt_phrases_noNT.get()){
			generateAll();
		}
		return *m_tgt_phrases_noNT;
	}
	
	const vector<string>& SRLHypothesisFromPhraseTableFrame::GetSourceTokenSequnce() const {
		if(!m_src_phrases.get()){
			generateAll();
		}
		return *m_src_phrases;
	}

	const vector<string>& SRLHypothesisFromPhraseTableFrame::GetSourceTokenSequnceWithoutNT() const {
		if(!m_src_phrases_noNT.get()){
			generateAll();
		}
		return *m_src_phrases_noNT;
	}
}

