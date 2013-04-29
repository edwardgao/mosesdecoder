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

All the writer definitions

*/


#ifndef __FORMATTERS_H__
#define __FORMATTERS_H__

#include "srl.h"
#include "srlio.h"

#include <string>

class SPredEntryFmter : public IFormatterBase{
private:
	
	std::string m_error;

public:
	/**
	Format the object, if error happens, error code is returned.
	@param ostr The return string
	@param object The object to be formatted
	@return Error code, zero (SRLIO_SUCCSSFUL) if successful.
	*/
	virtual int format(std::string& ostr, const void *object, int index);

	/**
	The implementations can return an error message by taking the error code.
	It can also just return the latest error message.
	*/
	virtual std::string getErrorMessage(int errorcode);
};

class SArgEntryFmter : public IFormatterBase{
private:
	
	std::string m_error;

public:

	/**
	Format the object, if error happens, error code is returned.
	@param ostr The return string
	@param object A pointer to argument info (int)
	@return Error code, zero (SRLIO_SUCCSSFUL) if successful.
	*/
	virtual int format(std::string& ostr, const void *object, int index);

	/**
	The implementations can return an error message by taking the error code.
	It can also just return the latest error message.
	*/
	virtual std::string getErrorMessage(int errorcode);
};


class SFeatEntryFmter : public IFormatterBase{
private:
	
	std::string m_error;

public:

	/**
	Format the object, if error happens, error code is returned.
	@param ostr The return string
	@param object A pointer to argument info (int)
	@return Error code, zero (SRLIO_SUCCSSFUL) if successful.
	*/
	virtual int format(std::string& ostr, const void *object, int index);

	/**
	The implementations can return an error message by taking the error code.
	It can also just return the latest error message.
	*/
	virtual std::string getErrorMessage(int errorcode);
};


#endif

