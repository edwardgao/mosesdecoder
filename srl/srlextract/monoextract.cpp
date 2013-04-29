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

/**
This program extracts all SRL roles and output information. The output format is as follows:

PREDICATE ||| ROLE ||| PHRASE

*/

int main(int argc, const char** argv) {
	init_logging(0);
	BEGIN_PARAMETERS;

	REQ_PARAM(string, sourceSent , "src,s", "Source corpus file", "Input");
	REQ_PARAM(string, srlCorpus,"label,l","The SRL parse (ASSERT format)","Input");
	REQ_PARAM(string, outputFile , "output,o", "Output file", "Output");

	DEF_PARAM(int, contextLen , 3, "context,c", "The length of stored contex", "Options");
	DEF_SWITCH(bool,verbose,"verbose","Display a sh*t load of information","Options");

	DEF_SWITCH(bool,help,"help,h","Show help.","Options");

	END_PARAMETERS;
	PARSE_COMMANDLINE(argc,argv);

	if (help) {
		PRINT_HELP;
		exit(1);
	}

	PRINT_CONFIG;

	ofstream output(outputFile.c_str());

	if (!output) {
		P_FATAL("Cannot open file %s for output", outputFile.c_str());
		exit(1);
	}

	ifstream src(sourceSent.c_str());

	if (!src) {
		P_FATAL("Cannot open file %s for input", sourceSent.c_str());
	}

	ifstream* srl_c = NULL;
	SRLInfoReader *srl_r = NULL;

	if (srlCorpus.length() > 0) {
		srl_c = new ifstream(srlCorpus.c_str());
		if (!(*srl_c)) {
			P_FATAL("Cannot open srl file %s for input",srlCorpus.c_str());
		}
		srl_r = new SRLInfoReader(*srl_c);
	}

	// Monolingual extraction happens here:

	int sent_num = 0;
	string sent;

	RuleExtractor extractor;
	for(;;){
		sent_num++;
		if(!src)
			break;
		sent = "";
		getline(src,sent);
		if(sent.length() == 0)
			continue;
		vector<SRLInformation> srl = srl_r->ReadSentence(sent_num);
		if(srl.size() == 0)
			continue;
		list<string> constit = extractor.DoMonolingualSRLExtraction(sent,srl,contextLen);
		for(list<string>::iterator it = constit.begin(); it!= constit.end(); it++){
			output<< *it << endl;
		}
	}

}
