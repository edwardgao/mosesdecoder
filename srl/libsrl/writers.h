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
#ifndef __WRITERS_H__
#define __WRITERS_H__

#include <fstream>

class SFileWriter : public IWriterBase{
protected:
	std::ofstream m_file;
public:

	/// Write the object, note for the formatter writer: The writer will not append line breaks, add your own
	virtual int write(const IIndexDirectAccessable *object); // The caller is responsible to allocate 
	// the object, and the error code is the error 
	// code returned by the parser.

	/// Close the writer
	virtual void close();

public:
	/**
	Open the file for input.
	@param fname File name to open
	*/
	SFileWriter(const char* fname);

	virtual ~SFileWriter(){};
};


#endif
