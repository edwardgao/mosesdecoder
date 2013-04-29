#ifndef _T_TABLE_H__
#define _T_TABLE_H__
#include <string>
#include <list>
#include <iostream>
#include <stdio.h>
#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>

#ifdef USE_TR1_UNORDERED_MAP
#include <tr1/unordered_map>
#include <tr1/unordered_set>
typedef std::tr1::unordered_map<std::string,double , boost::hash<std::string> > TDictType;
typedef std::tr1::unordered_set<std::string , boost::hash<std::string> > DictSetType;
typedef std::tr1::unordered_map<std::string,TDictType, boost::hash<std::string> > TTableType;
#endif


#ifdef USE_UNORDERED_MAP
#include <unordered_map>
#include <unordered_set>
typedef std::unordered_map<std::string,double, boost::hash<std::string> > TDictType;
typedef std::unordered_set<std::string, boost::hash<std::string> > DictSetType;
typedef std::unordered_map<std::string,TDictType, boost::hash<std::string> > TTableType;
#endif

namespace cindex{
	class TTable{
	public:
		TTable(double /*floorvalue*/); // Initialize the TTable given the floor value of the probability
	public:
		bool LoadTable(const char* fname, bool verify = false); // If Verify is true, then will check if the probability sum up to one
		bool LoadTable(std::istream& /*ifs*/, bool verify = false); // If Verify is true, then will check if the probability sum up to one
		double GetProb(const std::string& src, const std::string& tgt); // Get the probability
		double GetProb(const std::vector<std::string>& src, const std::vector<std::string>& tgt);
	private:
		TTableType ttable;
		double floor;
		double uniform;
	};
}


#endif