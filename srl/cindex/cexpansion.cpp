#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/tuple/tuple.hpp>
#include <string>
#include <set>

#include "SimpleLogger.h"
#include "invdoc.hpp"
#include "Params.h"

#include "srlinfo.h"
#include "aligninfo.h"
#include "srlinfoparser.h"
#include "ruleextract.h"
#include "srlruleset.h"
#include "IOUtils.h"
#include "t-table.h"

using namespace std;
using namespace boost;
using namespace cindex;
using namespace srl;
using namespace boost::tuples;



// Algorithm

namespace srl{
	class Expander : public srl::RuleExtractor{
	protected:

		int replace_mono(int structidx, int regionidx,	cindex::TTable& t_s2t, cindex::TTable& t_t2s, 
			SRLGenMonoRuleSet& monoRules, InvertedDocument& query_src,
			InvertedDocument& query_tgt, ostream &out, double probThreash, double maxFert, double minFert, int maxexpand, int evidence_len){
				int r_count =0;
				const SentencePair& sent = *this->sent_;
				const Alignment & alignment = *this->align_;
				//	const vector<SRLInformation> &srl = *this->srl_;
				vector<vector<string> >& tags = mapped_srl_tags_;
				vector<vector<pair<int, int> > >& regions = mapped_srl_regions_;
				vector<bool>& valid = srl_info_valid_;
				vector<string> target_evid_l, target_evid_r;
				vector<string> source_evid_l, source_evid_r;

				if(evidence_len < 2)
					evidence_len = 2;
				target_evid_l.reserve(evidence_len);
				target_evid_r.reserve(evidence_len);
				source_evid_l.reserve(evidence_len);
				source_evid_r.reserve(evidence_len);

				int i = structidx;

				if (!valid[i])
					return r_count;

				set<int> target_words, source_words;
				vector<pair<int, int> >& cregion = regions[i];

				int target_insert_point_l = cregion[regionidx].first, target_insert_point_r = cregion[regionidx].second;
				for (int k = target_insert_point_l; k <= target_insert_point_r; k++)
						target_words.insert(k);

				int originalsize = (int) target_words.size();
				int lastsize = originalsize;
				// Find all source words
				do {
					lastsize = target_words.size();
					for (set<int>::iterator it = target_words.begin(); it
						!= target_words.end(); it++) {
							const set<int>& al = alignment.AlignedToFrench(*it);
							source_words.insert(al.begin(), al.end());
					}
					for (set<int>::iterator it = source_words.begin(); it
						!= source_words.end(); it++) {
							const set<int>& al = alignment.AlignedToEnglish(*it);
							target_words.insert(al.begin(), al.end());
					}
				} while ((int) target_words.size() < lastsize);

				set<int> exp = source_words;
				// Add internal source word, if it is not aligned
				for (set<int>::iterator it = exp.begin(); it != exp.end(); it++) {
					int left = *it - 1;
					int right = *it + 1;
					while (left >= 0) {
						if (source_words.find(left) != source_words.end()) {
							break;
						}
						if (alignment.AlignedToEnglish(left).size() == 0) {
							source_words.insert(left--);
						} else {
							break;
						}
					}

					while (right < (int) sent.english().length()) {
						if (source_words.find(right) != source_words.end()) {
							break;
						}
						if (alignment.AlignedToEnglish(right).size() == 0) {
							source_words.insert(right++);
						} else {
							break;
						}
					}
				}

				if (originalsize - (int) target_words.size() < maxexpand
					&& (int) target_words.size() > 0
					&& (int) source_words.size() > 0) {
						string src, tgt;
						for (set<int>::iterator it = target_words.begin(); it!= target_words.end(); it++) {
							if(*it < target_insert_point_r && *it > target_insert_point_l){
								if(target_insert_point_r - *it > *it - target_insert_point_l){
									target_insert_point_l = *it+1;
								}else{
									target_insert_point_r = *it-1;
								}
							}
						}
						
						int source_insert_point_l = sent.english().length(), source_insert_point_r = -1;
						for(int k = target_insert_point_l; k<= target_insert_point_r;k++){
							if(alignment.AlignedToFrench(k).size()==0) continue;
							int start = *alignment.AlignedToFrench(k).begin();
							int end = *alignment.AlignedToFrench(k).rbegin();
							if(start < source_insert_point_l) source_insert_point_l = start;
							if( end  > source_insert_point_r) source_insert_point_r = end;
						}
						if(source_insert_point_l <0 || source_insert_point_r >= sent.english().length()){
							return r_count;
						}
						// AT THIS POINT WE GOT THE REGION FOR REPLACEMENT

						string s_left, s_right, t_left, t_right;

						for(int k = 0; k < source_insert_point_l ; k++){
							s_left += sent.english().GetWordAtIndex(k);
							if(k<source_insert_point_l-1)
								s_left += " ";
						}

						for(int k = 0; k < target_insert_point_l ; k++){
							t_left += sent.french().GetWordAtIndex(k);
							if(k<target_insert_point_l-1)
								t_left += " ";
						}
						for(int k = source_insert_point_r + 1; k < sent.english().length() ; k++){
							s_right += sent.english().GetWordAtIndex(k);
							if(k<sent.english().length()-1)
								s_right += " ";
						}

						for(int k = target_insert_point_r + 1; k < sent.french().length() ; k++){
							t_right += sent.french().GetWordAtIndex(k);
							if(k<sent.french().length()-1)
								t_right += " ";
						}

						P_INFO_VERBOSE(1,"Ready for replace : %s ___ %s <==> %s ___ %s",s_left.c_str(),s_right.c_str(),t_left.c_str(),t_right.c_str());

						// HERE WE FETCH ALL SOURCE CANDIDATE
						int sidx = source_insert_point_l - evidence_len;
						if(sidx < 0) sidx = 0;
						for(;sidx < source_insert_point_l ; sidx++){
							source_evid_l.push_back(sent.english().GetWordAtIndex(sidx));
						}
						sidx = source_insert_point_r + evidence_len+1;
						if(sidx > sent.english().length()) sidx = sent.english().length();
						for(int ss = source_insert_point_r+1; ss < sidx; ss++){
							source_evid_r.push_back(sent.english().GetWordAtIndex(ss));
						}

						vector<string> source_candid_str1 = query_src.QueryContext(source_evid_l,evidence_len,source_evid_r,evidence_len);

						set<string> source_candid_str;
						source_candid_str.insert(source_candid_str1.begin(),source_candid_str1.end());

						if(VERBOSE_LEVEL>3){
							string evdl,evdr;
							for(int k = 0; k<source_evid_l.size(); k++) evdl += source_evid_l[k];
							for(int k = 0; k<source_evid_r.size(); k++) evdr += source_evid_r[k];
							P_INFO_VERBOSE(3,"Query source evidence %s ____ %s, results [%d]",evdl.c_str(),evdr.c_str(),source_candid_str.size());
						}

						if(!source_candid_str.size())
							return r_count;

						vector<vector<string> > source_candid(source_candid_str.size());

						int iid = 0;
						for(set<string>::iterator it = source_candid_str.begin() ; it != source_candid_str.end(); it++){
							source_candid_str1[iid] = *it;
							split(source_candid[iid++],*it,is_any_of(" \t"),token_compress_on);
						}

						// AFTER THIS POINT, source_candid contains all the source candidates

						const vector<string> rep  = monoRules.GetRules(srl_frames_[structidx], tags[structidx][regionidx]);

						P_INFO_VERBOSE(2, "Query frame id %d [%s], role [%s], got %d candidates",structidx,srl_frames_[structidx].c_str(),tags[structidx][regionidx].c_str(),rep.size());

						//cerr << rep.size() << srl_frames_[structidx] << tags[structidx][regionidx] << endl;

						for(int l=0;l<(int)rep.size();l++){
							const string& tgt_replacement = rep[l];
							vector<string> target_candid;

							P_INFO_VERBOSE(3, "Trying to verify target replacement %d [%s]", l,tgt_replacement.c_str());

							split(target_candid,tgt_replacement,is_any_of(" \t"),token_compress_on);

							// AT THIS POINT Target replacement is proposed, we need to find evidence of it
							int idx = target_insert_point_l - evidence_len/2;

							target_evid_l.clear();
							target_evid_r.clear();

							for(; idx < target_insert_point_l; idx++)
								target_evid_l.push_back(sent.french().GetWordAtIndex(idx));

							int pos = 0;

							while(target_evid_l.size() < evidence_len && pos < target_candid.size()){
								target_evid_l.push_back(target_candid[pos++]);
							}

							int npos = target_candid.size() - evidence_len / 2;
							if(npos < pos){
								npos = pos;
							}
							
							for( ; npos < target_candid.size(); npos++)
								target_evid_r.push_back(target_candid[npos]);

							idx = target_insert_point_r;
							while(target_evid_r.size() < evidence_len && idx < sent.french().length()){
								target_evid_r.push_back(sent.french().GetWordAtIndex(idx));
								idx++;
							}
							

							if(VERBOSE_LEVEL>3){
								string evdl,evdr;
								for(int k = 0; k<target_evid_l.size(); k++) evdl += target_evid_l[k];
								for(int k = 0; k<target_evid_r.size(); k++) evdr += target_evid_r[k];
								P_INFO_VERBOSE(3,"Query target evidence %s ____ %s",evdl.c_str(),evdr.c_str());
							}


							if(!query_tgt.QueryEvidence(target_evid_l,evidence_len,target_evid_r,evidence_len)){ // If the target evidence don't exist, ignore it
								continue; // put it back
							}
							P_INFO_VERBOSE(3,"Query target evidence Found");
							// END VERIFY TARGET, from now on, the replacement is valid
							P_INFO_VERBOSE(3,"Found a valid replacement for predicate %s",this->srl_frames_[i].c_str());
							for(int j = 0; j< source_candid.size() ; j++){
								P_INFO_VERBOSE(3,"Verifying source candidate %d [%s]",j, source_candid_str1[j].c_str());
								double thresh = (double)source_candid[j].size() / target_candid.size();
								P_INFO_VERBOSE(3,"Fertility limit %lf",thresh);
								if(thresh < minFert || thresh > maxFert){
									P_INFO_VERBOSE(3,"Eliminated because of fertility");
									continue;
								}

								double pthresh1 = t_s2t.GetProb(source_candid[j],target_candid);
								double pthresh2 =  t_t2s.GetProb(target_candid,source_candid[j]);
								double pthresh = (pthresh1+pthresh2)/2;

								P_INFO_VERBOSE(3,"Prob[%lf] Source-to-target probability %lf <==> Target-to-source probability %lf ",pthresh,pthresh1, pthresh2);

								if(pthresh > probThreash){ // ACCEPTED, output
									P_INFO_VERBOSE(3,"Accepted %s <==> %s ", source_candid_str1[j].c_str(),tgt_replacement.c_str());
									out << s_left << " ||| " << source_candid_str1[j] << " ||| " << s_right << endl;
									out << t_left << " ||| " << tgt_replacement << " ||| " << t_right << endl;
									out << pthresh << endl;
									r_count++;
								}
							}
						}
				}
				P_INFO_VERBOSE(1, "Finished replacement for structure %d, generated %d new sentences", structidx, r_count);
				return r_count;
		}

		public:

		int TryExpansion(SentencePair& sent,Alignment& alignment,std::vector<SRLInformation>& srl,
			cindex::TTable& t_s2t, cindex::TTable& t_t2s, SRLGenMonoRuleSet& monoRules, InvertedDocument& query_src,
			InvertedDocument& query_t2s, ostream &out, double probThreash, double maxFert, double minFert, int maxexpand,
			int evidence_len
			){
				int rcount =0;
				this->sent_ = &sent;
				this->align_ = &alignment;
				this->srl_ = &srl;
				// Sequence:
				DoInitialization();
				DoMatchSRLInformation();
				vector<vector<string> >& tags = mapped_srl_tags_;
				vector<vector<pair<int, int> > >& regions = mapped_srl_regions_;
				vector<bool>& valid = srl_info_valid_;

				

				for (int i = 0; i < (int) tags.size(); i++) {
					if (!valid[i])
						continue;
					vector<string>& ctag = tags[i];
					P_INFO_VERBOSE(2,"Processing tag number %d ",i);
					
					for(int j=0; j<(int)ctag.size();j++){
						if(ctag[j]=="TARGET") continue;  // should we enable it?
						// ADD MORE DEBUG INFO!
						rcount += replace_mono(i, j, t_s2t, t_t2s, monoRules, query_src,
							query_t2s, out, probThreash, maxFert, minFert, maxexpand,evidence_len);
					}
				}
				return rcount;
		}
	};
};


int main(int argc, const char** argv){
	init_logging(0);
	BEGIN_PARAMETERS;

	REQ_PARAM(string, sourceSent , "src,s", "Source corpus file", "Input");
	REQ_PARAM(string, targetSent , "tgt,t", "Target corpus file", "Input");
	REQ_PARAM(string, alignFile , "align,a", "Alignment file", "Input");
	REQ_PARAM(string, srlCorpus,"label,l","The SRL parse (ASSERT format)","Input");
	REQ_PARAM(string, srlReplacement,"rep-rule,r","SRL Replacement rules (monolingual)","Input");
	REQ_PARAM(string, srcLextable,"srclex,c","Lexical translation probability (T-Table), SRC-TGT","Input");
	REQ_PARAM(string, tgtLextable,"tgtlex,g","Lexical translation probability (T-Table), TGT-SRC","Input");
	REQ_PARAM(string, srcIndex,"src-index,e","Source language index directory","Input");
	REQ_PARAM(string, tgtIndex,"tgt-index,x","Target language index directory","Input");

	REQ_PARAM(string, outputFile , "output,o", "Output file", "Output");

	DEF_SWITCH(bool,lowercase,"lower-case","Lower case the input before indexing","Options");
	
	DEF_PARAM(float, probThreshold,1e-6,"prob-thresh","The probability threshold for keeping the replacement, the probability is averaged bi-directional model 1, 1e-6 it the default","Options");
	DEF_PARAM(float, minFertilityRatio,0.3,"min-fert-ratio","The minimal replaced fertility ratio (Target / Source)","Options");
	DEF_PARAM(float, maxFertilityRatio,3,"max-fert-ratio","The maximal replaced fertility ratio (Target / Source)","Options");
	DEF_PARAM(int, shMaxUnalign,4,"ss-max-unaligned","Maximum number of words that are not aligned to any words in SRL","Options");
	DEF_PARAM(int, evidenceLen,2,"evidence-len","Length of words for evidence (on both left and right)","Options");

	DEF_SWITCH(bool,help,"help,h","Show help.","Options");
	DEF_SWITCH(bool,forced,"forced,f","Ignore errors","Options");
	DEF_PARAM(int,verbose,0,"verbose","Display a sh*t load of information","Options");

	END_PARAMETERS;
	PARSE_COMMANDLINE(argc,argv);

	if (help) {
		PRINT_HELP;
		exit(1);
	}
	PRINT_CONFIG;

	set_verbose_level(verbose);


	// Open All Files

	OPEN_STREAM_OR_DIE(ifstream, src, sourceSent.c_str());
	OPEN_STREAM_OR_DIE(ifstream, tgt, targetSent.c_str());
	OPEN_STREAM_OR_DIE(ifstream, aln, alignFile.c_str());
	OPEN_STREAM_OR_DIE(ifstream, srl, srlCorpus.c_str());
	OPEN_STREAM_OR_DIE(ifstream, ttable_s2t, srcLextable.c_str());
	OPEN_STREAM_OR_DIE(ifstream, ttable_t2s, tgtLextable.c_str());
	OPEN_STREAM_OR_DIE(ofstream, out, outputFile.c_str());

	
	AlignmentInfoReader rd(src,tgt,aln);
	SRLInfoReader srl_r(srl);

	InvertedDocument query_src(false,srcIndex.c_str());
	InvertedDocument query_tgt(false,tgtIndex.c_str());

	cindex::TTable t_s2t(1e-30), t_t2s(1e-30);

	P_INFO("Loading dictionary %s", srcLextable.c_str());
	t_s2t.LoadTable(ttable_s2t);
	P_INFO("\nLoading dictionary %s", tgtLextable.c_str());
	t_t2s.LoadTable(ttable_t2s);
	P_INFO("\nLoading SRLRules %s", srlReplacement.c_str());

	SRLGenMonoRuleSet monoRules(srlReplacement.c_str());
	// Done loading files, start iterate over all sentences
	P_INFO("Loading SRLRules %s done, start processing", srlReplacement.c_str());
	std::vector<SRLInformation> srli;
	int sent_number = 0;
	Expander exp;

	for (;;) {
		sent_number++;

		P_INFO_VERBOSE(1,"Loading sentence number %d",sent_number);
		boost::tuples::tuple<bool, SentencePair, Alignment> sentinfo = rd.Read();
		if (!get<0> (sentinfo))
			break;
		SentencePair& sentpair =get<1>(sentinfo);
		Alignment& aligninfo = get<2>(sentinfo);

		srli = srl_r.ReadSentence(sent_number);	

		exp.TryExpansion(sentpair,aligninfo,srli,t_s2t,t_t2s,monoRules,query_src,query_tgt,out,probThreshold,maxFertilityRatio,minFertilityRatio,shMaxUnalign,evidenceLen);
	
	}
}
