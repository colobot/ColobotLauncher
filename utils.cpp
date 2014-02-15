#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>

#include "md5.h"

bool fexists(const std::string& name) {
	if (FILE *file = fopen(name.c_str(), "r")) {
		fclose(file);
		return true;
	} else {
		return false;
	}
}

//Source: http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
std::string get_file_contents(const char *filename)
{
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	if (in)
	{
		return(std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
	}
	throw(errno);
}

std::string GetFileMD5(std::string filename)
{
	return md5(get_file_contents(filename.c_str()));
}
