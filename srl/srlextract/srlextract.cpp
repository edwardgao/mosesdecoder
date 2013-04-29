/*
 * libsrltest.cpp
 *
 *  Created on: Nov 6, 2010
 *      Author: qing
 */
#include <string>
#include <iostream>
#include <fstream>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <queue>
#include <set>

#include "srlinfo.h"
#include "aligninfo.h"
#include "srlinfoparser.h"
#include "ruleextract.h"
#include "SimpleLogger.h"
#include "Params.h"
#include "srlruleset.h"

using namespace std;
using namespace srl;
using namespace boost::tuples;
using namespace boost;
/*
 * Threading support
 */
void OutputFunc(queue<RuleExtractor::TaskResult> & results, boost::mutex& results_mutex,
		boost::mutex& vset_mutex, set<int> visited, ostream& output, ostream& outputinv, bool& quit_flag){
	string ori, inv;
	int sent_num;
	boost::mutex::scoped_lock set_lock(vset_mutex);
	while(!quit_flag){
		while(!results.empty()){
			{
				boost::mutex::scoped_try_lock results_lock(results_mutex);
				RuleExtractor::TaskResult result = results.front();
				//cerr << results.size()<<endl;
				ori = result.output_str;
				inv = result.output_inv;
				sent_num = result.sent_number;
			}
			output << ori << endl;
			outputinv << inv << endl;
			{
				boost::mutex::scoped_lock results_lock(results_mutex);
				results.pop();
			}
//			{
//				boost::mutex::scoped_lock set_lock(vset_mutex);
//				visited.erase(sent_num);
//			}
		}
	}
	//cerr <<"I QUIT\n";
}

void ShortenSentence(AlignmentInfoReader& alreader, SRLInfoReader& srl_r,
		ostream& output, ostream& outputinv, ostream& outputfeat, int max_unaligned_source, int min_words,
		const string& srlRules, int maxReplacement, bool outputFullSent, int maxreplacement) {
	RuleExtractor extractor;
	int sent_number=0;

	SRLGenRuleSet *rules = NULL;
	if(srlRules.length()){
		rules = new SRLGenRuleSet(srlRules.c_str());
	}


	for (;;) {
		sent_number++;
		boost::tuples::tuple<bool, SentencePair, Alignment> sentinfo = alreader.Read();
		if (!get<0> (sentinfo))
			break;
		SentencePair& sentpair = get<1> (sentinfo);
		Alignment& aligninfo = get<2> (sentinfo);

		stringstream srcsent;
		stringstream tgtsent;

		for(size_t i =0; i< sentpair.english().length(); i++){
			srcsent << sentpair.english().GetWordAtIndex(i);
			if(i<sentpair.english().length()-1){
				srcsent << " ";
			}
		}
		for (size_t i = 0; i < sentpair.french().length(); i++) {
			tgtsent << sentpair.french().GetWordAtIndex(i);
			if (i < sentpair.french().length() - 1) {
				tgtsent << " ";
			}
		}

		string srcs = srcsent.str();
		string tgts = tgtsent.str();

		vector<SRLInformation> srl = srl_r.ReadSentence(sent_number);

		extractor.set_output_all_text(outputFullSent);


		list<SRLGenRuleWithFeature > ext =
				extractor.DoSentenceShortening(sentpair,aligninfo,srl,sent_number,max_unaligned_source,min_words,
				rules, maxReplacement);

		if(ext.size()>=maxreplacement){
			ext.sort();
		}

		list<SRLGenRuleWithFeature >::iterator it;
		SRLGenRuleWithFeature last_output;
		int count = 0;
		for(it = ext.begin(); it !=ext.end() && count < maxreplacement;it++, count ++){
			if(it->source!=last_output.source&& it->target!=last_output.target){
//				output << it->source << " ||| " << srcs << endl;
//				outputinv << it->target << " ||| " << tgts << endl;
				output << it->source << endl;
			    outputinv <<  it->target << endl;
			    outputfeat << sent_number << " ";
				for(size_t i = 0 ; i< it->features.size(); i++){
					outputfeat << it->features[i];
					i < it->features.size() -1 ? ( outputfeat << " ") :  (outputfeat << "\n");
				}
				last_output = *it;
			}
		}

		if (sent_number % 10000 == 0)
			cerr << "." << flush;
		if (sent_number % 100000 == 0)
			cerr << "[" << sent_number << "]" << flush;
	}
	if(rules)
		delete rules;
}

void ExtractSRLGenerationRules(AlignmentInfoReader& alreader, SRLInfoReader& srl_r,
		ostream& output, ostream& outputinv, int max_unaligned_source, int max_words) {
	RuleExtractor extractor;
	int sent_number=0;

	for (;;) {
		sent_number++;
		boost::tuples::tuple<bool, SentencePair, Alignment> sentinfo = alreader.Read();
		if (!get<0> (sentinfo))
			break;
		SentencePair& sentpair = get<1> (sentinfo);
		Alignment& aligninfo = get<2> (sentinfo);

		vector<SRLInformation> srl = srl_r.ReadSentence(sent_number);

		pair<list<SRLGenRule>, list<SRLGenStruct> >  ext = extractor.DoExtractSRLGenerationRules(sentpair,aligninfo,srl,sent_number,max_unaligned_source,max_words);


		for(list<SRLGenRule>::iterator it = ext.first.begin(); it !=ext.first.end();it++){
			if((int)it->source.size()>max_words) continue;
			if((int)it->target.size()>max_words) continue;
			output << it->frame << " ||| " << it->argument << " ||| ";
			for(int i=0; i<(int)it->source.size();i++)
				output << it->source[i] << " ";
			output << "|||";
			for(int i=0; i<(int)it->target.size();i++)
				output << " " << it->target[i] ;
			output << endl;
		}

		for(list<SRLGenStruct>::iterator it = ext.second.begin(); it!=ext.second.end(); it++){

			outputinv << it->frame << " ||| ";
			for(int i ; i<(int)it->arguments.size();i++){
				outputinv << it->arguments[i] << " ";
			}
			outputinv << endl;
		}


		if (sent_number % 10000 == 0)
			cerr << "." << flush;
		if (sent_number % 100000 == 0)
			cerr << "[" << sent_number << "]" << flush;
	}
}




int main(int argc, const char** argv) {
	init_logging(0);
	BEGIN_PARAMETERS;

	DEF_SWITCH(bool,sentenceShortten,"short-sent","Instead of doing phrase extraction, do sentence shortening","Commands");
	DEF_SWITCH(bool,monoRules,"mono-rule","Instead of doing bilingual SRL extraction, only extract monolingual SRL rules","Commands");
	DEF_SWITCH(bool,extractSRLComp,"srl-gen","Instead of doing phrase extraction, extract bilingual SRL Substitution rule","Commands");


	REQ_PARAM(string, sourceSent , "src,s", "Source corpus file", "Input");
	REQ_PARAM(string, targetSent , "tgt,t", "Target corpus file", "Input");
	REQ_PARAM(string, alignFile , "align,a", "Alignment file", "Input");
	DEF_PARAM(string, srlCorpus,"","label,l","The SRL parse (ASSERT format)","Input");
	DEF_PARAM(string, srlReplacement,"","rep-rule,r","SRL Replacement rules in sentence shortening","Input");

	REQ_PARAM(string, outputFile , "output,o", "Output file", "Output");
	DEF_PARAM(string, outputEvent ,"", "event,e", "Output file for events", "Output");
	DEF_PARAM(string, outputGlue, "" , "glue,g", "Output file of all possible glue rules", "Output");


	DEF_PARAM(int, initialPhraseLength,10,"initial-phrase-length","Maximum length of the phrases extracted","Options");
	DEF_PARAM(int, srlTagType,0,"srl-tag-type","Token type of SRL rules: 0: Frame+Arguments 1: Arguments 2: General X","Options");
	DEF_PARAM(int, maxsourcetoken,5,"max-source-token","Maximum length of source token","Options");
	DEF_PARAM(int, maxtoken,7,"max-token","Maximum length tokens, source and target, should be equal or larger than max source token","Options");
	DEF_SWITCH(bool,skipStemming,"skip-stemming","Don't do stemming on the verbs","Options");
	DEF_PARAM(int, numthread,1,"num-thread","Number of threads in multi-threading","Options");
	DEF_PARAM(int, min_source_gap,1,"min-src-gap","Minimum gap between two non-terminals in the source side","Options");
	DEF_PARAM(int, min_target_gap,1,"min-tgt-gap","Minimum gap between two non-terminals in the target side","Options");
	DEF_SWITCH(bool,withoutHiero,"no-hiero","Do not do Hiero extraction, therefore, standard phrase extraction.","Options");
	DEF_SWITCH(bool,allowunaligned,"allow-unaligned","Allow phrases whose boundary are not aligned to be extracted as initial phrases for Hiero substitution","Options");
	DEF_SWITCH(bool,trackuncovered,"track-uncovered","Track uncovered source word, and output word pairs as phrase pairs","Options");
	DEF_SWITCH(bool,verbose,"verbose","Display a sh*t load of information","Options");


	DEF_PARAM(int, shMaxUnalign,4,"ss-max-unaligned","Maximum number of words that are not aligned to any words in SRL","Options For Sentence Shortening");
	DEF_PARAM(int, shMinWords,4,"ss-min-words","Minimum number of words in both side","Options For Sentence Shortening");
	DEF_PARAM(int, shMaxWords,10,"ss-max-words","Maximum number of words in both side when generating generation rules","Options For Sentence Shortening");

	DEF_PARAM(int, shMaxReplaceEntry,100,"ss-max-replace-entry","Maximum number of entries per replacement","Options For SRL Substitution Rules");
	DEF_PARAM(int, shMaxReplace,4,"ss-max-replace","Maximum number of SRL role replacement","Options For SRL Substitution Rules");
	DEF_SWITCH(bool, shFullSent,"ss-output-full-sent","Output full sentence instead of the SRL structure","Options For SRL Substitution Rules");


	DEF_SWITCH(bool,help,"help,h","Show help.","Options");
	END_PARAMETERS;
	PARSE_COMMANDLINE(argc,argv);

	if (help) {
		PRINT_HELP;
		exit(1);
	}
	PRINT_CONFIG;



	string outputFileInv = outputFile + ".inv";
	string outputFeature = outputFile + ".feat";
	if(extractSRLComp) outputFileInv = outputFile + ".struct";

	ofstream output(outputFile.c_str());

	if (!output) {
		P_FATAL("Cannot open file %s for output", outputFile.c_str());
		exit(1);
	}

	ofstream outputinv(outputFileInv.c_str());

	if (!outputinv) {
		P_FATAL("Cannot open file %s for output", outputFileInv.c_str());
		exit(1);
	}


	ifstream src(sourceSent.c_str());

	if(!src){
		P_FATAL("Cannot open file %s for input", sourceSent.c_str());
	}

	ifstream tgt(targetSent.c_str());

	if(!tgt){
		P_FATAL("Cannot open file %s for input", targetSent.c_str());
	}

	ifstream aln(alignFile.c_str());

	if(!aln){
		P_FATAL("Cannot open file %s for input", alignFile.c_str());
	}

	ifstream* srl_c = NULL;
	SRLInfoReader *srl_r = NULL;

	if(srlCorpus.length()>0){
		srl_c = new ifstream(srlCorpus.c_str());
		if(!(*srl_c)){
			P_FATAL("Cannot open srl file %s for input",srlCorpus.c_str());
		}
		srl_r = new SRLInfoReader(*srl_c);
	}

	ofstream* glue = NULL;
	if(outputGlue.length()){
		glue = new ofstream(outputGlue.c_str());
		if(!(*glue)){
			P_FATAL("Cannot open file %s for output", outputGlue.c_str());
		}
	}

	ofstream* event = NULL;
	if(outputEvent.length()){
		event = new ofstream(outputEvent.c_str());
		if(!(*event)){
			P_FATAL("Cannot open file %s for output", outputEvent.c_str());
		}
	}

	AlignmentInfoReader rd(src,tgt,aln);

	if(sentenceShortten ){
		if(!srl_r){
			cerr << "We need SRL information to do sentence shortening" << endl;
			exit(1);
		}
		ofstream outputfeat(outputFeature.c_str());

		if (!outputfeat) {
			P_FATAL("Cannot open file %s for output", outputFeature.c_str());
			exit(1);
		}

		ShortenSentence(rd,*srl_r,output,outputinv,outputfeat,shMaxUnalign,shMinWords,srlReplacement,shMaxReplace,shFullSent,shMaxReplaceEntry);
		cerr << "Done" << endl;
		return 0;
	}

	if(extractSRLComp ){
		if(!srl_r){
			cerr << "We need SRL information to extract SRL information" << endl;
			exit(1);
		}
		ExtractSRLGenerationRules(rd,*srl_r,output,outputinv,shMaxUnalign,shMaxWords);
		cerr << "Done" << endl;
		return 0;
	}

	/////////////////// THE EXTRATION HAPPENS BELOW

	std::vector<RuleExtractor* > extractor(numthread);

	for (int i = 0; i < numthread; i++) {
		extractor[i] = new RuleExtractor();
		extractor[i]->set_bypass_hiero(withoutHiero);
		extractor[i]->set_length_initial_phrase(initialPhraseLength);
		extractor[i]->set_disallow_unaligned_on_boundary(!allowunaligned);
		extractor[i]->set_max_tokens(maxsourcetoken);
		extractor[i]->set_max_tokens_no_terminal(maxtoken);
		extractor[i]->set_track_uncovered_words(trackuncovered);
		extractor[i]->set_initial_flag((srl::EInitialTagType) srlTagType);
		extractor[i]->set_skip_stemming(skipStemming);
		extractor[i]->set_min_gap_source(min_source_gap);
		extractor[i]->set_min_gap_target(min_target_gap);
		extractor[i]->set_extract_events(event);
	}


	std::vector<SRLInformation> srl;
	int sent_number = 0;

	queue<RuleExtractor::Task> tasks;
	queue<RuleExtractor::TaskResult> results;
	set<int> waiting_list;

	boost::mutex tasks_mutex, results_mutex, set_mutex;

	bool thread_quit_flag = numthread<=1;



	boost::thread outputThread(boost::bind(OutputFunc,boost::ref(results),boost::ref(results_mutex)
			,boost::ref(set_mutex),boost::ref(waiting_list),boost::ref(output),boost::ref(outputinv),boost::ref(thread_quit_flag)));

	std::vector<boost::thread* > procThreads;


	if(numthread>1){
		procThreads.resize(numthread);
		for (int i = 0; i < numthread; i++) {
			procThreads[i] = new boost::thread(boost::bind(&RuleExtractor::RunDaemon, boost::ref(extractor[i]),
					boost::ref(tasks),boost::ref(results),boost::ref(tasks_mutex),boost::ref(results_mutex),boost::ref(thread_quit_flag)
					));
		}
	}
	for (;;) {

		if(results.size()>1000000){
			boost::posix_time::millisec(1000);
			continue;
		}
		boost::tuples::tuple<bool, SentencePair, Alignment> sentinfo = rd.Read();
		if (!get<0> (sentinfo))
			break;
		SentencePair& sentpair =get<1>(sentinfo);
		Alignment& aligninfo = get<2>(sentinfo);

		if(srl_r){
			srl = srl_r->ReadSentence(sent_number+1);
			//cerr <<"Read srl: " << srl.size() << endl;
		}

		if(numthread>1){
			RuleExtractor::Task newTask;
			newTask.align = aligninfo;
			newTask.sent = sentpair;
			newTask.srl = srl;
			newTask.sent_number = ++sent_number;
			{
				boost::mutex::scoped_lock tasks_lock(tasks_mutex);
				tasks.push(newTask);
			}

		} else {

			extractor[0]->DoExtraction(sentpair, aligninfo, srl, ++sent_number);
			if (withoutHiero) {
				RuleExtractor::const_iterator it;
				it = extractor[0]->initial_phrase_begin();
				for (; it != extractor[0]->initial_phrase_end(); it++) {
					pair<string, string> formated =
							SerializeSCFGRule_PhraseOnly(*it, sentpair);
					output << formated.first << endl;
					outputinv << formated.second << endl;
				}
			} else {
				RuleExtractor::sorted_const_iterator it;
				it = extractor[0]->sorted_rule_begin();
				for (; it != extractor[0]->sorted_rule_end(); it++) {
					pair<string, string> formated =
							SerializeSortedSCFGRule(*it);
					output << formated.first << endl;
					outputinv << formated.second << endl;
					if(event)
						*event << formated.second << " ||| " << it->event_string() << endl;
				}
			}
		}
		if(verbose){
			cerr  << "\r"<< sent_number;
		}else{
			if (sent_number % 10000 == 0)
				cerr << "." << flush;
			if (sent_number % 100000 == 0)
				cerr << "[" << sent_number << "]" << flush;
		}



	}

	if(numthread>1){
		// Wait for all task to complete
		int timer = 0;
		while(true){
			{
				boost::mutex::scoped_try_lock tasks_lock(tasks_mutex);
				if(tasks_lock){
					if(tasks.empty()){
						boost::posix_time::millisec(50);
						break;
					}else{

					}
					timer++;
				}

			}
			//boost::this_thread::yield();
			boost::posix_time::millisec(50);
			//cerr << results.size();
		}
		//cerr <<"OK."<< results.size()<<endl;
		cerr << "Waiting jobs to complete...";
		while(true){
			//cerr <<"\n\n111\n";
			{
				boost::mutex::scoped_try_lock results_lock(results_mutex);
				//cerr <<"\n\n222\n";
				if(results_lock){
					if(results.empty()){
						boost::posix_time::millisec(50);
						break;
					}else{
						if(timer%10000==0)
							cerr << ".";
					}
					timer++;
				}

				boost::posix_time::millisec(50);
			}
		}
		cerr << endl;
	//	while(!waiting_list.empty()){
	//		boost::posix_time::millisec(50);
	//	}
		//cerr << "DONE\n";
		thread_quit_flag = true;
		{
			outputThread.join();
			for(int i=0 ; i<numthread ;i++){
				procThreads[i]->join();
			}
		}

	}

	if(trackuncovered){
		cout << "\nOutputting uncovered words\n" ;
		for (int i = 1; i < numthread; i++) {
			//cerr << "\nDDDD\n"<< i<<endl;
			extractor[0]->MergetUncoveredWordsAndStructures(*extractor[i]);
			//cerr << "\nHHH\n"<< i<<endl;
		}
		std::map<std::string,std::map<std::string, float> >::const_iterator it =
				extractor[0]->uncovered_words_begin();
		for(;it!=extractor[0]->uncovered_words_end();it++){
			for(std::map<std::string,float>::const_iterator it1 = it->second.begin();
					it1!=it->second.end(); it1++){
				std::pair<string,string> formated ;
				if(withoutHiero){
					formated = SerializeUncovered_PhraseOnly(it->first,it1->first);
					output << formated.first << endl;
					outputinv << formated.second << endl;
				} else {
					formated = SerializeUncovered(it->first, it1->first, it1->second);
					output << formated.first << endl;
					outputinv << formated.second << endl;
				}
			}
		}
	}
	//cerr << "\nHHH\n";
	if(glue){
		*glue << "<s> [X] ||| <s> [S] ||| 1 ||| ||| 0" << endl
			 << "[X][S] </s> [X] ||| [X][S] </s> [S] ||| 1 ||| 0-0 ||| 0" << endl
			 << "[X][S] [X][X] [X] ||| [X][S] [X][X] [S] ||| 2.718 ||| 0-0 1-1 ||| 0" << endl;
		std::set<std::string>::const_iterator it = extractor[0]->completed_srl_structures().begin();
		for(; it!=  extractor[0]->completed_srl_structures().end();it++){
			if(*it=="X") continue;
			*glue << "<s> [X][" << *it << "] </s> [X] ||| <s> [X][" <<*it <<"] </s> [S] ||| 1 ||| 1-1" << endl;
		}
		it = extractor[0]->completed_srl_structures().begin();
		for(; it!=  extractor[0]->completed_srl_structures().end();it++){
			*glue << "[X][S] [X][" << *it << "] [S] ||| [X][S] [X][" <<*it <<"] [S] ||| 2.718 ||| 0-0 1-1" << endl;
			if(*it=="X") continue;
		}
		delete glue;
	}
	if(srl_c)
		delete srl_c;
	if(srl_r)
		delete srl_r;

	for(int i=0; i< numthread; i++){
		delete extractor[i];
	}
	for(int i=0; i<(int)procThreads.size();i++){
		delete procThreads[i];
	}
	if(event)
		delete event;

return 0;
}
