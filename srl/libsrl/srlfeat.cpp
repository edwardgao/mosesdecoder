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

Implementation of SRL features
*/

#include "srlfeat.h"

using namespace std;

ProvideFeature::ProvideFeature(PfeatureType type, uint id, uint start, uint len){
	m_idtype = id*4 + type;
	m_range = start * 16 + len;
}
ProvideFeature::ProvideFeature(const ProvideFeature &other): m_range(other.m_range), m_idtype(other.m_idtype){
	m_features = other.m_features;
}

pair<double, bool> ProvideFeature::QueryFeature(FeatureId id){
	pair<double, bool> result;
	map<FeatureId,double>::iterator it = m_features.find(id);
	if(it == m_features.end()){
		result.first = 0.0;
		result.second = false;
	}else{
		result.first = it->second;
		result.second = true;
	}
	return result;
}

pair<double, bool> ProvideFeature::SetFeature(FeatureId id, double value){
	pair<double, bool> result;
	map<FeatureId,double>::iterator it = m_features.find(id);
	if(it == m_features.end()){
		m_features[id] = value;
		result.first = value;
		result.second = true;
	}else{
		it->second = value;
		result.first = it->second;
		result.second = false;
	}
	return result;
}


