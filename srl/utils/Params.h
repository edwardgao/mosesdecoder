/***************************************************************************
 *            Params.h
 *
 *  Sun Jul 26 02:35:00 2009
 *  Copyright  2009  qing
 *  <qing@<host>>
 ****************************************************************************/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
#ifndef __CQPARAM_H__
#define __CQPARAM_H__
#include "SimpleLogger.h"
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <map>
#include <string>
#include <stdlib.h>
#include <set>
#include <vector>
namespace po = boost::program_options;
#include <iostream>
#include <iterator>
#include <fstream>
using namespace std;
using boost::format;
using boost::io::group;
class CQParam{
	private:
		// Mapping from name to session
		std::map<std::string,std::set<std::string> > sectionmap;
		std::map<std::string,std::string > nameswitchmap;
		std::map<std::string,std::string> descmap;
		// The parameters are grouped into sessions, map from session name to 
		// object
		std::map<std::string,boost::shared_ptr<po::options_description> > 
			sections;
		//We only support int, bool, string, float (for now)
		// Map from string to its storage
		std::map<std::string,int*> intmap;
		// Map from string to its storage, other types
		std::map<std::string,float*> floatmap; 
		std::map<std::string,std::string* > stringmap;
		std::map<std::string,bool* > boolmap;
		std::map<std::string,bool* > boolnmap;
		std::map<std::string,std::string> requiredmap;

		po::options_description toplevel;
	public:

		CQParam():toplevel("Allowed Parameters:"){};
		CQParam(const char* desc):toplevel(desc){};

		virtual void AddParam(const char* type, // type , be int, float etc
		                      const char* name, // the display name of the param
		                      void* storage,    // where the parameter is stored
		                      const char* def,  // The default value, (parsed)
		                      const char* arg,  // Argument name
		                      const char* desc, // Description
		                      const char* sect, // Section
		                      bool  required  , // Whether it is required
		                      const char* sep = NULL,// Separator,for future use
		                      void* otherinfo = NULL // For future use
		                      )
		{
			C_FATAL(type!=NULL);
			C_FATAL(storage!=NULL);			
			C_FATAL(arg!=NULL);
			C_FATAL(desc!=NULL);			
			C_FATAL(sect!=NULL);			
			std::string ssect = sect; // section
			std::string sname = name;
			std::map<std::string,boost::shared_ptr<po::options_description> >
				::iterator it
				= sections.find(ssect);
			if (it == sections.end()){
				sections[ssect] = 
					boost::shared_ptr<po::options_description>(
					    new po::options_description(sect)
					                  ); // create
				it = sections.find(ssect);
				C_FATAL(it!=sections.end());
				//toplevel.add(*(it->second));
			}
			C_FATAL(it!=sections.end());
			po::options_description &posec = *(it->second);
			std::string stype = type;
			P_DEBUG("stype: %s",type);
			if(stype=="int"){
				if(def!=NULL){
					int defl = atoi(def);
					D_DEBUG(DINT,defl);
					posec.add_options()
						(arg, 
						 po::value<int>((int*)storage)->default_value(defl), 
						 desc);			
				}else{
					posec.add_options()
						(arg, 
						 po::value<int>((int*)storage), 
						 desc);			

				}
				intmap[sname] = (int*)storage;
			}else if(stype=="float"){
				if(def!=NULL){
					float defl = atof(def);
					posec.add_options()
						(arg, 
						 po::value<float>((float*)storage)->default_value(defl), 
						 desc);			
				}else{
					posec.add_options()
						(arg, 
						 po::value<float>((float*)storage), 
						 desc);			

				}				
				floatmap[sname] = (float*)storage;
			}else if(stype=="string"){
				if(def!=NULL){
					std::string defl = def;
					if(defl[0]=='\"' && defl[defl.length()-1]=='\"'){
						defl = defl.substr(1,defl.length()-2);
					}
					posec.add_options()
						(arg, 
						 po::value<std::string>(
						   (std::string*)storage)->default_value(defl), 
						 desc);			
				}else{
					posec.add_options()
						(arg, 
						 po::value<std::string>((std::string*)storage), 
						 desc);			

				}
				stringmap[sname] = (std::string*)storage;
			}else if(stype=="bool"){
				posec.add_options()
					(arg,desc);
				std::vector<std::string> tokens;
				boost::split(tokens,arg,boost::is_any_of(",")
				             ,boost::token_compress_on);	
				boolmap[tokens[0]] = (bool*)storage;
				boolnmap[sname] = (bool*)storage;
			}

			if(required){
				std::vector<std::string> tokens;
				boost::split(tokens,arg,boost::is_any_of(",")
				             ,boost::token_compress_on);	
				requiredmap[tokens[0]]=sname;
			}

			// store information

			std::map<std::string,std::set<std::string> >::iterator sit 
				= sectionmap.find(ssect);
			if(sit==sectionmap.end()){
				sectionmap[ssect] = std::set<std::string>();
				sit = sectionmap.find(ssect);				
			}
			C_FATAL(sit!=sectionmap.end());
			sit->second.insert(sname);
			descmap[sname] = std::string(desc);
			nameswitchmap[sname] = string(arg);
					
		}

		virtual void prepareParse(){
			std::map<std::string,boost::shared_ptr<po::options_description> >
				::iterator ist;

			for(ist = sections.begin();ist!=sections.end();ist++){
				toplevel.add(*(ist->second));
			}			
		}

		virtual void parseCmdline(int ac, const char** av){
			char** avv = const_cast<char**>(av);
			po::variables_map vm;
			po::store(po::parse_command_line(ac, avv, toplevel), vm);
			po::notify(vm);
			dealwithVM(vm);
		}

		virtual void dealwithVM(po::variables_map& vm){
			std::map<std::string,bool* >::iterator it;
			for(it = boolmap.begin(); it!=boolmap.end(); it++){
				D_DEBUG(DINT,vm.count(it->first.c_str()));
				D_DEBUG(DSTRING,it->first.c_str());
				*(it->second) = vm.count(it->first.c_str());
			}

			std::map<std::string,std::string>::iterator iit;
			bool helpPrinted = false;
			for(iit=requiredmap.begin();iit!=requiredmap.end();iit++){
				if(!vm.count(iit->first)){//no required!
					if(!helpPrinted) {
						help(std::cerr); helpPrinted = true;
						std::cerr << "The following REQUIRED parameters are not "
							    "provided, please check your configuration \n";
						std::cerr << format("%1% %|20t|%2% %|44t|%3% \n")
					                    % "Name" % "Switches" % "Description" ;
					}
					std::cerr << format("%1% %|20t|%2% %|44t|%3% \n")
					     % iit->second % nameswitchmap[iit->second] % descmap[iit->second] ;

				}
			}
			if(helpPrinted)
				exit(-1);
		}

		virtual void parseConfigure(istream& is){
			C_ERROR(is);			
			po::variables_map vm;
			po::store(po::parse_config_file(is,toplevel), vm);
			po::notify(vm);
			dealwithVM(vm);
		}

		virtual void parseConfigure(const char* fname){			
			C_ERROR(fname!=NULL);			
			P_INFO_VERBOSE(2,"Reading configure file: fname %s", fname);			
			std::ifstream inp(fname);
			parseConfigure(inp);
		}

		virtual void help(std::ostream &os){
			os << toplevel;
		}

		virtual void printConfigure(std::ostream &os){
			os << 
" =============================================================================\n" <<
"|Configuration sheet                                                          |\n" <<
" =============================================================================\n";
			std::map<std::string,std::set<std::string> >::iterator it;
			bool first = true;
			for(it = sectionmap.begin(); it!=sectionmap.end();it++){
				os<<
(!first ?" -----------------------------------------------------------------------------\n" :"")<<
format("| Section : %1% %|78t|%2%\n") % it->first % "|" <<
" -----------------------------------------------------------------------------\n";
				first = false;
				std::set<std::string>::iterator nit;
				for(nit = it->second.begin(); nit!=it->second.end(); nit++){
					const std::string &name = *nit;
					std::map<std::string,float*>::iterator itf;
					std::map<std::string,int*>::iterator iti;
					std::map<std::string,bool*>::iterator itb;
					std::map<std::string,string*>::iterator its;
					if((itf=floatmap.find(name))!=floatmap.end()){
						os <<
							format("| %1% %|25t|%2% %3% %|78t|%4%\n")
							% name % "|" % *(itf->second) % "|";
					}else if((iti=intmap.find(name))!=intmap.end()){
						os <<
							format("| %1% %|25t|%2% %3% %|78t|%4%\n")
							% name % "|" %*(iti->second) % "|";
					}else if((its=stringmap.find(name))!=stringmap.end()){
						const std::string& pth = *(its->second);
						string rep;
						if(pth.length()>47){
							rep = pth.substr(0,46)+"...";
						}else
							rep = pth;
						os <<
							format("| %1% %|25t|%2% \"%3%\"%|78t|%4%\n")
							% name % "|" % rep % "|";
					}else if((itb=boolnmap.find(name))!=boolnmap.end()){
						os <<
							format("| %1% %|25t|%2% %3% %|78t|%4%\n")
							% name % "|" % (*(itb->second) ? "ON" : "OFF") % "|";
					}
				}
			}
			os<<
" -----------------------------------------------------------------------------\n";
		}
		
};


#define BEGIN_PARAMETERS CQParam __param; using namespace std;

#define DEF_PARAM(type,name,def,args,desc,sect) type name ; __param.AddParam(#type,#name,&name,#def,args,desc,sect,false);
#define REQ_PARAM(type,name,args,desc,sect) type name ; __param.AddParam(#type,#name,&name,NULL,args,desc,sect,true);
#define DEF_SWITCH(type,name,args,desc,sect) type name ; __param.AddParam(#type,#name,&name,NULL,args,desc,sect,false);
#define END_PARAMETERS __param.prepareParse();

#define PARSE_COMMANDLINE(c,v) __param.parseCmdline(c,v);
#define PARSE_CONFIGFILE(f) __param.parseCmdline(f);

#define PRINT_CONFIG __param.printConfigure(std::cerr);

#define PRINT_HELP __param.help(std::cerr);


#endif

















