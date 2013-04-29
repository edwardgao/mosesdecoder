/***************************************************************************
 *            SimpleLogger.h
 *
 *  Wed Jul 22 13:10:19 2009
 *  Copyright  2009  qing
 *  <qing@<host>>
 ****************************************************************************/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */


/**********************

 The package provide basic logging utility functions, use E_FATAL/ERROR/WARN/
 INFO/DEBUG to display logging information, D_FATAL/D_ERROR/.../ to dump 
 variable information.
 
 **********************/
#ifndef _SIMPLE_LOGGER_H__
#define _SIMPLE_LOGGER_H__
#include <stdio.h>
#include <time.h>
/**
Defining several levels of logging
*/
#ifdef LOG_ALL
#define LOG_FATAL
#define LOG_ERROR
#define LOG_WARN
#define LOG_INFO
#define LOG_DEBUG
#endif


#ifdef LOG_NONE
#undef  LOG_FATAL
#undef  LOG_ERROR
#undef  LOG_WARN
#undef  LOG_INFO
#undef  LOG_DEBUG
#endif

#ifdef LOG_CRITICAL
#define LOG_FATAL
#define LOG_ERROR
#undef  LOG_WARN
#undef  LOG_INFO
#undef  LOG_DEBUG
#endif

#ifdef LOG_ATTENTION
#define LOG_FATAL
#define LOG_ERROR
#define LOG_WARN
#undef  LOG_INFO
#undef  LOG_DEBUG
#endif

#ifdef LOG_INFORMATIVE
#define LOG_FATAL
#define LOG_ERROR
#define LOG_WARN
#define LOG_INFO
#undef  LOG_DEBUG
#endif

#ifndef LOGGER_IMPL
extern FILE * _fFatal;
extern FILE * _fError;
extern FILE * _fWarn;
extern FILE * _fInfo;
extern FILE * _fDebug;
extern unsigned int logger_verbose;
#endif

#define SET_LOGGER_FATAL 1
#define SET_LOGGER_ERROR 2
#define SET_LOGGER_WARN  3
#define SET_LOGGER_INFO  4
#define SET_LOGGER_DEBUG 5

#define TIME_FORMAT ("[%y-%m-%d %H:%M:%S]")

#define eprintf(format, ...) fprintf (stderr, format, ##__VA_ARGS__)
#define _F(x)  (fopen(x,"w"))
#define _FA(x) (fopen(x,"a+"))

#define P_GENERAL(format,_file,_info, ...) (_file && (fprintf(_file, "%s",_info ),printtime(TIME_FORMAT,_file),fprintf(_file, "%s:%u(%s): "format"\n" , __FILE__, __LINE__,__FUNCTION__,##__VA_ARGS__)))
#define P_GENERAL_F(format,_file,_info, ...) (_file && (fprintf(_file, "%s",_info ),printtime(TIME_FORMAT,_file),fprintf(_file, "%s:%u(%s): "format"\n" , __FILE__, __LINE__,__FUNCTION__,##__VA_ARGS__),m_exit(1),1))
#define D_GENERAL(x,format,_file,_info) (_file ?  (fprintf(_file, "%s", _info ),printtime(TIME_FORMAT,_file),fprintf(_file, "%s:%u(%s):\"%s\" ==> "format"\n" , __FILE__, __LINE__,__FUNCTION__, #x, (x))):(x,1))
#define D_GENERAL_F(x,format,_file,_info) ((_file ?  (fprintf(_file, "%s", _info ),printtime(TIME_FORMAT,_file),fprintf(_file, "%s:%u(%s):\"%s\" ==> "format"\n" , __FILE__, __LINE__,__FUNCTION__, #x, (x))):(x,1)),m_exit(1))
#define A_GENERAL(x,_file,_info) ( (!(x)) && _file  &&  (fprintf(_file, "%s", _info ),printtime(TIME_FORMAT,_file),fprintf(_file, "%s:%u(%s):\"%s\" Assretion Failed, statment is false\n" , __FILE__, __LINE__,__FUNCTION__, #x), m_exit(-1),1))
#define C_GENERAL(x,_file,_info) ( (!(x)) && _file  &&  (fprintf(_file, "%s", _info ),printtime(TIME_FORMAT,_file),fprintf(_file, "%s:%u(%s):\"%s\" Assretion Failed, statment is false\n" , __FILE__, __LINE__,__FUNCTION__, #x)))
#define C_GENERAL_F(x,_file,_info) ( (!(x)) && _file  &&  (fprintf(_file, "%s", _info ),printtime(TIME_FORMAT,_file),fprintf(_file, "%s:%u(%s):\"%s\" Assretion Failed, statment is false\n" , __FILE__, __LINE__,__FUNCTION__, #x), m_exit(-1),1))

/*!Print*/
#define P_FATAL(format, ...) P_GENERAL_F(format,_fFatal,"[Fatal]",##__VA_ARGS__)
#define P_ERROR(format, ...) P_GENERAL(format,_fError,"[Error]",##__VA_ARGS__)
#define P_WARN(format, ...) P_GENERAL(format,_fWarn,"[Warn ]",##__VA_ARGS__)
#define P_INFO(format, ...) P_GENERAL(format,_fInfo,"[Info ]",##__VA_ARGS__)
#define P_INFO_VERBOSE(L,format, ...) ((L<logger_verbose) && P_GENERAL(format,_fInfo,"[Info ]",##__VA_ARGS__))
#ifdef DEBUG
#define P_DEBUG(format, ...) P_GENERAL(format,_fDebug,"[Debug]",##__VA_ARGS__)
#define P_DEBUG_VERBOSE(L,format, ...) ((L<logger_verbose) && P_GENERAL(format,_fDebug,"[Debug]",##__VA_ARGS__))
#else
#define P_DEBUG(format, ...)
#define P_DEBUG_VERBOSE(L,format, ...)
#endif

/*!Dump */
#define D_FATAL(format,x) D_GENERAL_F(x,format,_fFatal,"[Dump(Fatal)]")
#define D_ERROR(format,x) D_GENERAL(x,format,_fError,"[Dump(Error)]")
#define D_WARN(format,x) D_GENERAL(x,format,_fWarn,"[Dump(Warn )]")
#define D_INFO(format,x) D_GENERAL(x,format,_fInfo,"[Dump(Info )]")
#define D_INFO_VERBOSE(L,format,x) ((L<logger_verbose) && D_GENERAL(x,format,_fInfo,"[Dump(Info )]"))
#ifdef DEBUG
#define D_DEBUG(format,x) D_GENERAL(x,format,_fDebug,"[Dump(Debug)]")
#define D_DEBUG_VERBOSE(L,format,x) ((L<logger_verbose) && D_GENERAL(x,format,_fDebug,"[Dump(Debug)]"))
#else
#define D_DEBUG(format,x)
#define D_DEBUG_VERBOSE(L,format,x)
#endif

/*!Assertion */
#ifdef DEBUG
#define A_FATAL(x) A_GENERAL((x),_fFatal,"[Assert(Fatal)]")
#define A_ERROR(x) A_GENERAL((x),_fError,"[Assert(Error)]")
#define A_WARN(x) A_GENERAL((x),_fWarn,"[Assert(Warn )]")
#define A_INFO(x) A_GENERAL((x),_fInfo,"[Assert(Info )]")
#define A_INFO_VERBOSE(L,x) ((L<logger_verbose) &&A_GENERAL((x),_fInfo,"[Assert(Info )]"))
#define A_DEBUG(x) A_GENERAL(x,_fDebug,"[Assert(Debug)]")
#define A_DEBUG_VERBOSE(L,x) ((L<logger_verbose) && A_GENERAL((x),_fDebug,"[Assert(Debug)]"))
#else
#define A_FATAL(x) 
#define A_ERROR(x) 
#define A_WARN(x) 
#define A_INFO(x) 
#define A_INFO_VERBOSE(L,x) 
#define A_DEBUG(x) 
#define A_DEBUG_VERBOSE(L,x) 
#endif

#define C_FATAL(x) C_GENERAL_F(x,_fFatal,"[Confirm(Fatal)]")
#define C_ERROR(x) C_GENERAL(x,_fError,"[Confirm(Error)]")
#define C_WARN(x) C_GENERAL(x,_fWarn,"[Confirm(Warn )]")
#define C_INFO(x) C_GENERAL(x,_fInfo,"[Confirm(Info )]")
#define C_INFO_VERBOSE(L,x) ( (L<logger_verbose) &&C_GENERAL(x,_fInfo,"[Confirm(Info )]"))
#ifdef DEBUG
#define C_DEBUG(x) C_GENERAL(x,_fDebug,"[Assert(Debug)]")
#define C_DEBUG_VERBOSE(L,x) ((L<logger_verbose) && C_GENERAL(x,_fDebug,"[Confirm(Debug)]"))
#else
#define C_DEBUG(x)
#define C_DEBUG_VERBOSE(L,x)
#endif


#define DINT    "%d"
#define DFLOAT  "%g"
#define DSTRING "%s"
#define DDOUBLE "%lg"
//#define P_FATAL(format, ...) (fprintf(_fFatal, "[Fatal]" ),printtime(TIME_FORMAT,_fFatal),fprintf(_fFatal, "%s:%u: "format"\n" , __FILE__, __LINE__,##__VA_ARGS__))
#ifdef __cplusplus
extern "C" 
#endif
void printtime(const char* format, FILE* file);

/*!
Initialize logging, and set the logger strings

E.g.

init_logging(0) ;  // initialize all logging, to stderr
init_logging(SET_LOGGER_FATAL, _F("c://log")); // initialize only fatal, create new file "C://log"
init_logging(SET_LOGGER_ERROR, _F("C://log"),_FA("C://log2")); // initialize fatal and error, create new file "C://log" and append to "C://log1"
*/
#ifdef __cplusplus
extern "C" 
#endif
void init_logging (int count,...);

#ifdef __cplusplus
extern "C" 
#endif
void m_exit(int code);

//Enable/Disable loggers
#define set_logger_fatal(x) (_fFatal=x)
#define set_logger_error(x) (_fError=x)
#define set_logger_warn(x)  (_fWarn=x)
#define set_logger_info(x)  (_fInfo=x)
#define set_logger_debug(x) (_fDebug=x)

#define disable_logger_fatal (set_logger_fatal(NULL))
#define disable_logger_error (set_logger_error(NULL))
#define disable_logger_warn (set_logger_warn(NULL))
#define disable_logger_info (set_logger_info(NULL))
#define disable_logger_debug (set_logger_debug(NULL))
#define set_verbose_level(x) (logger_verbose = x)

#define VERBOSE_LEVEL (logger_verbose)
#endif
