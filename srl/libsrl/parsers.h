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

Definition of parsers
*/

#ifndef __PARSERS_H__
#define __PARSERS_H__

#include "srl.h"
#include "srlio.h"

#include <string>

class SPredEntryParser : public IParserBase{
private:
	
	std::string m_error;

public:
	/**
	Parse the text and input it to the argument table. 
	@param object The parameter should be an PredTable instance, and the new entry will be inserted into that object.
	*/
	virtual int parse(const char* text, void *object);

	/**
	The implementations can return an error message by taking the error code.
	It can also just return the latest error message.
	*/
	virtual std::string getErrorMessage(int errorcode);
};

class SArgEntryParser : public IParserBase{
private:
	
	std::string m_error;

public:

	/**
	Parse the text and input it to the argument table. 
	@param object The parameter should be an ArgumentTable instance, and the new entry will be inserted into that object.
	*/
	virtual int parse(const char* text, void *object);

	/**
	The implementations can return an error message by taking the error code.
	It can also just return the latest error message.
	*/
	virtual std::string getErrorMessage(int errorcode);
};

class SFeatEntryParser : public IParserBase{
private:
	
	std::string m_error;

public:

	/**
	Parse the text and input it to the argument table. 
	@param object The parameter should be an SRLFeatEntry instance
	*/
	virtual int parse(const char* text, void *object);

	/**
	The implementations can return an error message by taking the error code.
	It can also just return the latest error message.
	*/
	virtual std::string getErrorMessage(int errorcode);
};


#endif