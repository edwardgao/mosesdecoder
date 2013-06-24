#include "srlbridge.h"
#include <cmath>
#include "SafeGetline.h"
#include "tables-core.h"
#include "PhraseAlignment.h"

using namespace std;

#define LINE_MAX_LENGTH 100000

using namespace MosesTraining;

namespace srl{
#if 0

	void SRLEventModelTrainer::Train(istream& extractFileP,  ostream& output, bool bIsInv)
	{
		float lastCount = 0.0f;
		float lastPcfgSum = 0.0f;
		vector<PhraseAlignment> phrasePairsWithSameF;
		bool isSingleton = true;
		int i=0;
		char line[LINE_MAX_LENGTH],lastLine[LINE_MAX_LENGTH];
		lastLine[0] = '\0';
		PhraseAlignment *lastPhrasePair = NULL;
		while(true) {
			if (extractFileP.eof()) break;
			if (++i % 100000 == 0) cerr << "." << flush;
			SAFE_GETLINE((extractFileP), line, LINE_MAX_LENGTH, '\n', __FILE__);
			if (extractFileP.eof())	break;

			// identical to last line? just add count
			if (strcmp(line,lastLine) == 0) {
				lastPhrasePair->count += lastCount;
				lastPhrasePair->pcfgSum += lastPcfgSum;
				continue;
			}
			strcpy( lastLine, line );

			// create new phrase pair
			PhraseAlignment phrasePair;
			phrasePair.create( line, i, true);
			lastCount = phrasePair.count;
			lastPcfgSum = phrasePair.pcfgSum;

			// only differs in count? just add count
			if (lastPhrasePair != NULL 
				&& lastPhrasePair->equals( phrasePair )
				&& featureManager.equals(*lastPhrasePair, phrasePair)) {
					lastPhrasePair->count += phrasePair.count;
					lastPhrasePair->pcfgSum += phrasePair.pcfgSum;
					continue;
			}

			// if new source phrase, process last batch
			if (lastPhrasePair != NULL &&
				lastPhrasePair->GetSource() != phrasePair.GetSource()) {
					//processPhrasePairs( phrasePairsWithSameF, *phraseTableFile, isSingleton, featureManager, maybeLogProb );

					phrasePairsWithSameF.clear();
					isSingleton = false;
					lastPhrasePair = NULL;
			}
			else
			{
				isSingleton = true;
			}

			// add phrase pairs to list, it's now the last one
			phrasePairsWithSameF.push_back( phrasePair );
			lastPhrasePair = &phrasePairsWithSameF.back();
		}
		//processPhrasePairs( phrasePairsWithSameF, *phraseTableFile, isSingleton, featureManager, maybeLogProb );

	}
#endif
}

