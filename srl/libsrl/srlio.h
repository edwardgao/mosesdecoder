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

The header defines the IO interfaces for reading/writing SRL. 

Parser/Reader
We stick to Moses's Rule table reader/writer for rule table IO, however, the last additional data filed for the features should be re-written.

Also, the predicate/argument tables should be read/written first.

The names for parsers/readers should be:

SRLFieldParser
SRLPredicateTableParser
SRLArgumentTableParser

And the names for writers, Formatters are:

SRLFieldFormatter
SRLPredicateTableFormatter
SRLArgumentTableFormatter

Parser's design

IMO, the parser and formatters deals with parsing string into objects, in the mean time, reader and writter handles the file input. It reads lines and call designed parser to parse.

The command function for a parser is 

int ParserBase::parse(const char* text, void *object);

It returns zero when successfully parse the object, and otherwise when failed.

It should also be able to return an error message upon the error, according to the error code.

string ParserBase::getErrorMessage(int errorcode);

Reader's Design

The Reader are simpler, the handle the task of fetching data, but not parsing them. Also, it handles how a "line" is defined, and therefore how the text is passed to the parser. The interface for the parser should contain a method to get and set parser.

void setParser(ParserBase * parser);
const ParserBase* getParser(); const;

And the reader acts as an iterator, with two functions hasNext(); and readNext();

bool hasNext();  // Return false if no more record CAN be readed.
int readNext(void *object); // The caller is responsible to allocate 
// the object, and the error code is the error 
// code returned by the parser.
string getErrorMessage(int errorcode);

Formatter and Writer are reversed operators of Readers/Writers.

*/

#ifndef __SRLIO_H__
#define __SRLIO_H__

#include <string>
#include "srl.h"

#define SRLIO_SUCCESSFUL (0)

/**
Interface for the formatter, which takes an object and return the string
*/
class IFormatterBase{
public:
	/**
	Format the object, if error happens, error code is returned.
	@param ostr The return string
	@param object The object to be formatted
	@return Error code, zero (SRLIO_SUCCSSFUL) if successful.
	*/
	virtual int format(std::string& ostr, const void *object,int index) = 0;

	/**
	The implementations can return an error message by taking the error code.
	It can also just return the latest error message.
	*/
	virtual std::string getErrorMessage(int errorcode) = 0;
};

class IParserBase{
public:
	/**
	Parse the string into an object.
	@param text The string to be parsed
	@param object The object that the result will be stored, the caller need to initialize the object
	@return Error code, zero (SRLIO_SUCCSSFUL) if successful.
	*/
	virtual int parse(const char* text, void *object) = 0;

	/**
	The implementations can return an error message by taking the error code.
	It can also just return the latest error message.
	*/
	virtual std::string getErrorMessage(int errorcode) = 0;
};

class IReaderBase{
protected:
	IParserBase *m_parser;
public:
	/**
	The implementations can return an error message by taking the error code.
	It can also just return the latest error message.
	*/
	virtual std::string getErrorMessage(int errorcode) {return m_parser->getErrorMessage(errorcode);}

	/**
	Set the parser to be used.
	*/
	virtual void setParser(IParserBase * parser) {m_parser = parser;};
	/// Return current parser
	virtual const IParserBase* getParser() const {return m_parser;}

	/// Return true if the reader have more records to be processed
	virtual bool hasNext() = 0;  // Return false if no more record CAN be readed.
	/// Read next element, the caller need to initialize the object
	virtual int readNext(void *object) = 0; // The caller is responsible to allocate 
	// the object, and the error code is the error 
	// code returned by the parser.
	/// Close the reader
	virtual void close() = 0;
};

class IWriterBase{
protected:
	IFormatterBase *m_formatter;
public:
	/**
	The implementations can return an error message by taking the error code.
	It can also just return the latest error message.
	*/
	virtual std::string getErrorMessage(int errorcode){return m_formatter->getErrorMessage(errorcode);};

	/// Set the formatter
	virtual void setFormatter(IFormatterBase * formatter) {m_formatter = formatter;};
	/// Get the formatter
	virtual const IFormatterBase* getFormatter() const{return m_formatter;};

	/// Write the content of the object
	virtual int write(const IIndexDirectAccessable *object); // The caller is responsible to allocate 
	// the object, and the error code is the error 
	// code returned by the parser.

	/// Close the writer
	virtual void close();
};

#include "parsers.h"
#include "formatters.h"
#include "readers.h"
#include "writers.h"


#endif

