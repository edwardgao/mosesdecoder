
#ifndef __SRL_RULESET_H_
#define __SRL_RULESET_H_

#include <map>
#include <string>
#include <vector>


namespace srl{

struct SRLGenRuleWithFeature{
	std::string source;
	std::string target;
	std::vector<float> features;
	float sum_feature;
	inline bool operator<(const SRLGenRuleWithFeature& rt) const{
		return sum_feature > rt.sum_feature;
	};
};



class SRLGenRuleSet{

public:
	SRLGenRuleSet(const char* /*file*/);
	const std::vector<SRLGenRuleWithFeature>&
			GetRules(const std::string& frame, const std::string& argument) const;

private:
	typedef SRLGenRuleWithFeature TPair;
	typedef std::vector<TPair> TPairList;
	typedef std::map<std::string, TPairList  > TArgMap;
	typedef std::map<std::string, TArgMap > TFrameMap;


	TFrameMap rules_;
	TPairList null_list_;
};

// Monolingual Rule Set

class SRLGenMonoRuleSet{
public:
	SRLGenMonoRuleSet(const char* /*file*/);

	virtual const std::vector<std::string>& 
		GetRules(const std::string& frame, const std::string& argument) const;

protected:
	typedef std::vector<std::string> TList;
	typedef std::map<std::string, TList> TArgMap;
	typedef std::map<std::string, TArgMap> TFrameMap;

	TFrameMap rules_;
	TList null_list_;
};

}


#endif
