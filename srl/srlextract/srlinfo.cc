#include <string>
#include <vector>

#include "srlinfo.h"

namespace srl {

using namespace std;

SRLInformation::SRLInformation() {
	error_ = false;
	Init();
}

SRLInformation::~SRLInformation() {

}

void SRLInformation::Init() {
	error_ = false;
	label_.clear();
	regions_.clear();
	words_.clear();
}

}
