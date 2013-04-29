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

Implementation of readers
*/


#include "srl.h"
#include "srlio.h"

#include <string>

using namespace std;

SFileReader::SFileReader(const char* fname): m_file(fname) {}

void SFileReader::close(){
	m_file.close();
}

bool SFileReader::hasNext(){
	return (bool)(m_file);
}
int SFileReader::readNext(void *object){
	string line;
	while(m_file && line.length()==0){
		getline(m_file,line);
	}
	return m_parser->parse(line.c_str(),object);
}