// ConfigFile.cpp

#include "configfile.h"

#include <string>

#include <strutils.h>
#include <common.h>

ConfigFile::ConfigFile( tstring filename, tstring delimiter,
                        tstring comment, tstring sentry )
	: myDelimiter(delimiter), myComment(comment), mySentry(sentry)
{
	// Construct a ConfigFile, getting keys and values from given file
	
	std::basic_ifstream<tchar> in( filename.c_str() );

	fileValid = !!in;

	if (in)
		in >> (*this);
}


ConfigFile::ConfigFile()
	: myDelimiter( tstring(1,'=') ), myComment( tstring(1,'#') )
{
	// Construct a ConfigFile without a file; empty

	fileValid = false;
}


void ConfigFile::remove( const tstring& key )
{
	// Remove key and its value
	myContents.erase( myContents.find( key ) );
	return;
}


bool ConfigFile::keyExists( const tstring& key ) const
{
	// Indicate whether key is found
	mapci p = myContents.find( key );
	return ( p != myContents.end() );
}


/* static */
void ConfigFile::trim( tstring& s )
{
	// Remove leading and trailing whitespace
	static const tchar whitespace[] = " \n\t\v\r\f";
	s.erase( 0, s.find_first_not_of(whitespace) );
	s.erase( s.find_last_not_of(whitespace) + 1U );
}


std::basic_ostream<tchar>& operator<<( std::basic_ostream<tchar>& os, const ConfigFile& cf )
{
	// Save a ConfigFile to os
	for( ConfigFile::mapci p = cf.myContents.begin();
	     p != cf.myContents.end();
		 ++p )
	{
		os << p->first.c_str() << " " << cf.myDelimiter.c_str() << " ";
		os << p->second.c_str() << std::endl;
	}
	return os;
}


std::basic_istream<tchar>& operator>>( std::basic_istream<tchar>& is, ConfigFile& cf )
{
	// Load a ConfigFile from is
	// Read in keys and values, keeping internal whitespace
	typedef tstring::size_type pos;
	const tstring& delim  = cf.myDelimiter;  // separator
	const tstring& comm   = cf.myComment;    // comment
	const tstring& sentry = cf.mySentry;     // end of file sentry
	const pos skip = delim.length();        // length of separator
	
	tstring nextline = "";  // might need to read ahead to see where value ends

	while( is || nextline.length() > 0 )
	{
		// Read an entire line at a time
		std::basic_string<tchar> sline;
		tstring line;
		if( nextline.length() > 0 )
		{
			line = nextline;  // we read ahead; use it now
			nextline = "";
		}
		else
		{
			std::getline( is, sline );
			line = sline.c_str();
		}
		
		// Ignore comments
		line = line.substr( 0, line.find(comm) );
		
		// Check for end of file sentry
		if( sentry != "" && line.find(sentry) != tstring::npos ) return is;
		
		// Parse the line if it contains a delimiter
		pos delimPos = line.find( delim );
		if( delimPos < tstring::npos )
		{
			// Extract the key
			tstring key = line.substr( 0, delimPos );
			line.replace( 0, delimPos+skip, "" );
			
			// See if value continues on the next line
			// Stop at blank line, next line with a key, end of stream,
			// or end of file sentry
			bool terminate = false;
			while( !terminate && is )
			{
				std::basic_string<tchar> snextline;
				std::getline( is, snextline );
				nextline = snextline.c_str();
				terminate = true;
				
				tstring nlcopy = nextline;
				ConfigFile::trim(nlcopy);
				if( nlcopy == "" ) continue;
				
				nextline = nextline.substr( 0, nextline.find(comm) );
				if( nextline.find(delim) != tstring::npos )
					continue;
				if( sentry != "" && nextline.find(sentry) != tstring::npos )
					continue;
				
				nlcopy = nextline;
				ConfigFile::trim(nlcopy);
				if( nlcopy != "" ) line += "\n";
				line += nextline;
				terminate = false;
			}
			
			// Store key and value
			ConfigFile::trim(key);
			ConfigFile::trim(line);
			cf.myContents[key] = line;  // overwrites if key is repeated
		}
	}
	
	return is;
}
