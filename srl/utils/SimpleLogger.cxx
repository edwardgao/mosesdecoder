//           SimpleLogger.cxx
//  Wed Jul 22 13:17:02 2009
//  Copyright  2009  qing
//  <qing@<host>>

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA

#define LOGGER_IMPL
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "SimpleLogger.h"


FILE * _fFatal = NULL;
FILE * _fError = NULL;
FILE * _fWarn  = NULL;
FILE * _fInfo  = NULL;
FILE * _fDebug = NULL;
unsigned int logger_verbose = 0;


extern "C" void init_logging (int count,...){
	_fFatal = stderr;
	_fError = stderr;
	_fWarn  = stderr;
	_fInfo  = stderr;
	_fDebug = stderr;

	va_list ap;
	int i/*,sum*/;

	va_start (ap, count);         /* Initialize the argument list. */

	
	for (i = 0; i < count; i++){
		switch(i){
		case 0:
			_fFatal = va_arg (ap, FILE*);    /* Get the next argument value. */
			break;
		case 1:
			_fError = va_arg (ap, FILE*);    /* Get the next argument value. */
			break;
		case 2:
			_fWarn  = va_arg (ap, FILE*);    /* Get the next argument value. */
			break;
		case 3:
			_fInfo  = va_arg (ap, FILE*);    /* Get the next argument value. */
			break;
		case 4:
			_fDebug = va_arg (ap, FILE*);    /* Get the next argument value. */
			break;
		default:
			break;
		}
	}

	va_end (ap);                  /* Clean up. */
}

extern "C" void m_exit(int code){
#ifdef _DEBUG 
	getchar();
#endif
	exit(code);
}

extern "C" void printtime(const char* format, FILE* file){
	char buffer [80];
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

//#ifdef _WIN32
	strftime(buffer,80,format,timeinfo);
//#else
	//strftime(buffer[0],80,format,timeinfo);
//	strptime(&(buffer[0]),format,timeinfo);
//#endif
	fprintf(file,"%s",buffer);
}


