/***************************************************************************
 *            IOUtils.h
 *
 *  Sat Jul 25 10:54:02 2009
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

#ifndef __IOUTILS__H__
#define __IOUTILS__H__

#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <stdio.h>
#include <string.h>
#include "SimpleLogger.h"
#include <boost/algorithm/string.hpp>


// Macros

#define OPEN_STREAM_OR_DIE_MSG(type, name, fname, format, ...) type name(fname); if(!name) P_FATAL(format, ##__VA_ARGS__)
#define OPEN_FILE_OR_DIE_MSG(name, fname,mode, format, ...) FILE* name = fopen(fname,mode); if(!name) P_FATAL(format, ##__VA_ARGS__)
#define OPEN_STREAM_OR_DIE(type, name, fname) OPEN_STREAM_OR_DIE_MSG(type,name,fname,"Cannot open file %s", fname)
#define OPEN_FILE_OR_DIE(name, fname,mode) OPEN_FILE_OR_DIE_MSG(type,name,fname,"Cannot open file %s", fname)



namespace qutil{
	namespace io{
		/* Read a bunch of lines int buffer, ended with empty line
		 \param is the input stream
		 \param conntainer a sequential container, such as list, 
		 		vector etc, we use container.push_back and container.clear()
		 \param limit the number of lines to be read, option, 0 means no limt
		 */
		template <typename C>
			void ReadLinesEndsEmpty(std::istream& is, 
			                        C& container, 
			                        size_t limit = 0)
		{
			container.clear();
			char buffer[16384];
			size_t i = 0;
			while(!is.getline(buffer,16384).eof()){
				i++;
				if((!strlen(buffer))
				   || ( limit && i > limit ))
					return;
				else
					container.push_back(std::string(buffer));
			}
		}
	}
}


#endif

