/*
 * =====================================================================================
 *
 *       Filename:  rescore-nbest-srl.cpp
 *
 *    Description:  Rescore N-Best with SRL Information 
 *
 *        Version:  1.0
 *        Created:  07/08/2013 10:13:02 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Qin Gao (qing), qing@cs.cmu.edu
 *        Company:  LTI, SCS, CMU
 *
 * =====================================================================================
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
#include "InputFileStream.h"
#include "OutputFileStream.h"

using namespace std;
using namespace srl;
using namespace boost::tuples;
using namespace boost;

int main(int argc, const char** argv){
	init_logging(0);
	BEGIN_PARAMETERS;
	
	REQ_PARAM(string, nbest_input , "nbest,n", "NBest Input File", "Input");
	REQ_PARAM(string, srl_input , "srl,s", "SRL Input File", "Input");
	
	/*
	DEF_SWITCH(bool,monoRules,"mono-rule","Instead of doing bilingual SRL extraction, only extract monolingual SRL rules","Commands");
	DEF_SWITCH(bool,extractSRLComp,"srl-gen","Instead of doing phrase extraction, extract bilingual SRL Substitution rule","Commands");
	REQ_PARAM(string, sourceSent , "src,s", "Source corpus file", "Input");
	DEF_SWITCH(bool,withoutHiero,"no-hiero","Do not do Hiero extraction, therefore, standard phrase extraction.","Options");*/
	DEF_SWITCH(bool,help,"help,h","Show help.","Options");
	END_PARAMETERS;
	PARSE_COMMANDLINE(argc,argv);

	if (help) {
		PRINT_HELP;
		exit(1);
	}
	PRINT_CONFIG;
	
	Moses::InputFileStream srlFile(srl_input) ;
	srl::SRLInfoReader srlReader(*((istream*)&srlFile));
	
	// TODO: Read N-Best

}
