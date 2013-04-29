#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <string>

#include "SimpleLogger.h"
#include "invdoc.hpp"
#include "Params.h"


using namespace std;
using namespace boost;
using namespace cindex;

template<typename _Pred>
bool indexFile(InvertedDocument& indexer, const char* fname, _Pred pred){
	ifstream ifs(fname);
	if(!ifs){
		P_ERROR("Cannot find file %s", fname);
		return false;
	}
	string line;
	while(ifs){
		line = "";
		getline(ifs,line);
		line = pred(line);
		if(line.length()){
			indexer.IndexNewDoc(line.c_str());
		}
	}
	return true;
}

struct no_process_f{
	inline string operator()(const string& input){
		return input;
	}
};

struct lower_case_f{
	inline string operator()(const string& input){
		try{
			return boost::to_lower_copy(input);
		}catch(boost::exception &){
			P_ERROR("Failed to lowercase %s, index the sentence as it is ", input.c_str());
			return input;
		}
		
	}
};

int main(int argc, const char** argv){
	init_logging(0);
	BEGIN_PARAMETERS;

	REQ_PARAM(string, inputList , "input,i", "A list containing the input file", "Input");
	REQ_PARAM(string, outputDir , "output,o", "A directory for output", "Output");
	DEF_PARAM(string, rootDir,"","root,r","The root directory of input files","Input");
	DEF_PARAM(int, maxsplit,50,"max-split,m","The max number of splits","Input");
	DEF_PARAM(int, buffersize,20,"buffer-size,b","The size of buffer, million entries","Options");
	DEF_SWITCH(bool,lowercase,"lower-case,l","Lower case the input before indexing","Options");
	DEF_SWITCH(bool,help,"help,h","Show help.","Options");
	DEF_SWITCH(bool,forced,"forced,f","Ignore errors","Options");
	DEF_SWITCH(bool,verbose,"verbose","Display a sh*t load of information","Options");

	END_PARAMETERS;
	PARSE_COMMANDLINE(argc,argv);

	if (help) {
		PRINT_HELP;
		exit(1);
	}
	PRINT_CONFIG;

	InvertedDocument indexer(true,outputDir.c_str(),maxsplit,buffersize * 1000000);

	boost::filesystem::path root(rootDir);

	ifstream fls(inputList.c_str());

	if(!fls){
		P_FATAL("Cannot open input list file %s", inputList.c_str());
	}
	string fname;
	while(fls){
		fname = "";
		getline(fls,fname);
		if(fname.length() > 0){
			if(rootDir.length()>0){
				boost::filesystem::path fpath = root / fname.c_str();				
				fname = fpath.string();
			}
		}else{
			continue;
		}
		P_INFO("Indexing file %s", fname.c_str());
		int docs = indexer.GetTotalDocuments();
		

		if(lowercase){
			if(!indexFile(indexer,fname.c_str(),lower_case_f() )){
				if(!forced){
					P_FATAL("Cannot continue (use --forced to ignore errors), quitting");
				}
			}
		}else{
			if(!indexFile(indexer,fname.c_str(),no_process_f() )){
				if(!forced){
					P_FATAL("Cannot continue (use --forced to ignore errors), quitting");
				}
			}
		}
		docs = indexer.GetTotalDocuments() - docs;
		P_INFO("Finished indexing file %s, indexed %d lines. Now there are totally %d lines indexed",fname.c_str(), docs, indexer.GetTotalDocuments());
	}
	indexer.Close();

	return 0;
}
