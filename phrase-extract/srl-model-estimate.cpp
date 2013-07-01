#include <iostream>
#include "srlmodel.h"
#include "SimpleLogger.h"
#include "Params.h"
#include "srlbridge.h"


using namespace std;

using namespace srl;



int main(int argc, const char** argv){
	init_logging(0);
	BEGIN_PARAMETERS;

	REQ_PARAM(string, extracted , "extracted,e", "Extracted phrase table with SRL information", "Input");
	DEF_PARAM(string, rev_extracted ,"", "reversed-extracted,r", "Reversed phrase table with SRL information", "Input");
	REQ_PARAM(string, output_file , "output,o", "Output Model File", "Output");
	
	DEF_PARAM(string, model_definination,"","model-def,m","Model definitions (string format separated with |||)","Options");
	DEF_PARAM(string, model_def_file,"","model-def-file,f","Model definition file","Options");
	DEF_SWITCH(bool,help,"help,h","Show help.","Options");
	END_PARAMETERS;

	PARSE_COMMANDLINE(argc,argv);

	if (help) {
		PRINT_HELP;
		exit(1);
	}

	if ( ! (model_definination.length() || model_def_file.length()) ){
		PRINT_HELP;
		P_FATAL("You must either specify model definition or model definition file");
	}

	PRINT_CONFIG;


	boost::shared_ptr<SRLEventModelSet> pModelSet;

	if(model_definination.length()){
		pModelSet = SRLEventModelSet::Construct(model_definination);
	}else{
		ifstream modeldef(model_def_file.c_str());
		pModelSet = SRLEventModelSet::Construct(modeldef);
	}

	if(pModelSet.get() == NULL || pModelSet->GetNames().size() == 0){
		PRINT_HELP;
		P_FATAL("Cannot read model definition, please check you config");
	}

	SRLEventModelTrainer forward_trainer, backward_trainer;
	
	if(forward_trainer.InitTraining(true, pModelSet)){
		ifstream rd(extracted.c_str());
		if(!rd)
			P_FATAL("Cannot open %s for input", extracted.c_str());
		srl::TrainSRLModel(rd, forward_trainer);
		forward_trainer.Normalize();
	}else{
		P_WARN("No forward model defined, skipping");
	}
	
	if(rev_extracted.length() && backward_trainer.InitTraining(false, pModelSet)){
		ifstream rd(rev_extracted.c_str());
		if(!rd)
			P_FATAL("Cannot open %s for input", rev_extracted.c_str());
		srl::TrainSRLModel(rd, forward_trainer);
		backward_trainer.Normalize();
	}else{
		P_WARN("No forward model defined, skipping");
	}

	ofstream ofs(output_file.c_str());

	if(!ofs){
		P_FATAL("Cannot open %s for input", output_file.c_str());
	}

	pModelSet->SerializeAll(ofs);
	
    return 0;
}
