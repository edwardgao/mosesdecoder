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

All the reader definitions

*/

#ifndef __READERS__H__
#define __READERS__H__

#include <fstream>

class SFileReader : public IReaderBase{
protected:
	std::ifstream m_file;
public:
	/// Return true if the reader have more records to be processed
	virtual bool hasNext();  // Return false if no more record CAN be readed.
	/// Read next element, the caller need to initialize the object
	virtual int readNext(void *object); // The caller is responsible to allocate 
	// the object, and the error code is the error 
	// code returned by the parser.
	/// Close the reader
	virtual void close();

public:
	/**
	Open the file for input.
	@param fname File name to open
	*/
	SFileReader(const char* fname);

	virtual ~SFileReader(){};
};

#endif