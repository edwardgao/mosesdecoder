#include <iostream>
#include <fstream>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <string>

#include "SimpleLogger.h"
#include "invdoc.hpp"
#include "Params.h"


using namespace std;
using namespace boost;
using namespace cindex;


int main(int argc, const char** argv){
	init_logging(0);
	BEGIN_PARAMETERS;

	REQ_PARAM(string, inputList , "input,i", "A list containing the queries, the left and right context are separated by |||", "Input");
	REQ_PARAM(string, outputFile , "output,o", "Output file", "Output");
	DEF_PARAM(string, rootDir,"","root,r","The root directory of the index directory","Input");	
	DEF_PARAM(int, maxContext,3,"max-context,m","The maximum length of context","Input");	
	DEF_SWITCH(bool,help,"help,h","Show help.","Options");
	DEF_SWITCH(bool,forced,"forced,f","Ignore errors","Options");
	DEF_SWITCH(bool,verbose,"verbose","Display a sh*t load of information","Options");
	DEF_SWITCH(bool,lowercase,"lower-case,l","Lower case the input before indexing","Options");

	END_PARAMETERS;
	PARSE_COMMANDLINE(argc,argv);
	if (help) {
		PRINT_HELP;
		exit(1);
	}
	PRINT_CONFIG;

	ifstream ifs(inputList.c_str());
	ofstream ofs(outputFile.c_str());

	InvertedDocument querier(false,rootDir.c_str());
	string line;

	vector<string> entries,lc(maxContext),rc(maxContext),ltxt,rtxt;
	
	entries.reserve(2);
	while(ifs && getline(ifs,line)){
		if(line.length()){
			if(lowercase){
				try{
					line = to_lower_copy(line);
				}catch(boost::exception &){

				}
			}
			boost::algorithm::split_regex(entries,line,regex(" \\|\\|\\| "));
			if(entries.size()>=2){
				ofs << line << endl;
				trim(entries[0]);
				trim(entries[1]);
				split(ltxt,entries[0],is_any_of(" \t"),token_compress_on);
				split(rtxt,entries[1],is_any_of(" \t"),token_compress_on);
				if(ltxt.size()>maxContext){
					copy(&(ltxt[ltxt.size()-maxContext]),(&ltxt[0])+ltxt.size(),lc.begin());
				}else{
					lc = ltxt;
				}
				if(rtxt.size()>maxContext){
					copy(&(rtxt[0]),&(rtxt[maxContext]),rc.begin());
				}else{
					rc = rtxt;
				}
				int maxlen = -1;
				int minlen = -1;
				if(entries.size()>2){
					maxlen = lexical_cast<int>(entries[2]);
				}
				if(entries.size()>3){
					minlen = lexical_cast<int>(entries[3]);
				}
				vector<string> results = querier.QueryContext(lc,maxContext,rc,maxContext,minlen,maxlen);
				
				for(vector<string>::iterator it = results.begin() ; it != results.end(); it++){
					ofs << *it << endl;
				}
			}else{
				ofs << "#ERROR_LINE#" << line << endl;
			}
		}else{
			ofs << "EMPTY_LINE#" << endl;
		}
		ofs << endl;
		line = "";
	}

	return 0;
}