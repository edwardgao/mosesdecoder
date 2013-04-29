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

Implementation of writers
*/


#include "srl.h"
#include "srlio.h"

#include <string>

using namespace std;

SFileWriter::SFileWriter(const char* fname): m_file(fname) {}

void SFileWriter::close(){
	m_file.close();
}


int SFileWriter::write(const IIndexDirectAccessable *object){
	string pt;
	for(size_t idx = 0; idx < object->size() ; idx++){
		int result = m_formatter->format(pt,(*object)[idx],idx);
		if(result != SRLIO_SUCCESSFUL) return result;
	}
	
	return SRLIO_SUCCESSFUL;
}
