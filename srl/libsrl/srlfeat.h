/**
@file 
@author Qin Gao <pku.gaoqin@gmail.com>
@version 0.0.1

@section LICENSE

The source code can be used freely as long as the author is aware of the usage. Send an email
to the author before using it.

The source code is provide "as is" and the author is not responsible for any effect/damage of the code.

The banner of the source code should not be removed.

@section DESCRIPTION

SRL Feature line, this header only contains the feature data structure, not the reader/writer
*/

#ifndef __SRL_FEAT_H__
#define __SRL_FEAT_H__
#include <map>
#include "defs.h"
#include <vector>
#include <string>


#define FEATUREID_NOCHAR ('A'-1)

inline FeatureId getFeatureIDInt(char c1, char c2){return (c1-FEATUREID_NOCHAR)*27 + (c2 - FEATUREID_NOCHAR);};
inline std::string getFeatureIDStr(FeatureId fid){
	std::string ret; 
	ret += (char)(fid/27+FEATUREID_NOCHAR);
	if(fid%27)
		ret += (char)(fid%27+FEATUREID_NOCHAR);
	return ret;
}

class ProvideFeature{
private:
	uint m_idtype; // The ID and type, the m_idtype % 4 == type; m_idtype / 4 == id

	uint m_range; // The range (terminal position), it will also be a combination of two really small integers, the start, and the len.
	              // start = m_range / 16, len = m_range % 16

	std::map<FeatureId, double> m_features; // All the features
public:
	/// Get the type (Predicate/Argument/Nonterminal)
	inline PfeatureType GetType()const {return (PfeatureType)(m_idtype % 4);};
	/// Get the id 
	inline uint GetID() const {return m_idtype / 4;};
	/// Get Start
	inline uint GetStart() const {return m_range / 16;};
	/// Get Length
	inline uint GetLength() const {return m_range % 16;};

public:

	ProvideFeature(PfeatureType type, uint id, uint start, uint len);
	ProvideFeature(const ProvideFeature &other);

public:
	/// Query the feature value with feature id, if not found, return false in the result->second
	std::pair<double, bool> QueryFeature(FeatureId id);
	/// Set the feature value. If the feature does not exist, the second element of the return value will be true and the first 
	/// element be the value. If the feature already exist, the second element of the return value will be false and the first be 
	/// the old value
	std::pair<double, bool> SetFeature(FeatureId id, double value);
	
public: // Navigate
	typedef std::map<FeatureId, double>::const_iterator iterator;

	/// Get the iterator for navigating all features, note that the iterator is CONSTANT
	inline iterator begin() const {return m_features.begin();}
	/// The end of the iterator
	inline iterator end() const{return m_features.end();}

public:
	
};

/**
SRL Feature entry, this will be appended after each Moses/cdec rules
*/
class SRLFeatEntry{
private:
	std::vector<ProvideFeature> m_features;

public:
	/// Get the provided feature by index
	inline const ProvideFeature& GetPf(size_t idx) const {return m_features[idx];};
	inline ProvideFeature& GetPf(size_t idx) {return m_features[idx];};

	/// Number of features
	inline size_t GetNumPf() const {return m_features.size();};

	/// Add new Pf to the list, return the index of newly added entry
	inline size_t AppendPf(const ProvideFeature& pf){m_features.push_back(pf); return m_features.size()-1;};

	/// Re-initialize
	inline void clear(){m_features.clear();};
};

#endif 
