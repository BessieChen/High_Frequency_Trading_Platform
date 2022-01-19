#include "mystruct.h"

void ReadKbarSeries(std::string fileName, std::vector<Kbar>& kbar_vec)
{
}

void Save_FileName(std::string path, std::vector<std::string>& fileName_vec)
{
}


void StringToCharPP(std::string & str, char  * pp)
{
	int len = str.length();
	pp = new char[len + 1];

	memcpy(pp, str.c_str(), len);
	pp[len] = '\0';
}
