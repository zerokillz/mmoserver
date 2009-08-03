/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "utils.h"
#include <string>

//==========================================================================

uint64 swap64(uint64 value)
{
	BYTE arr[8];

	memcpy(arr, &value, 8);

	for(int x = 0; x < 4; x++)
	{
		BYTE temp = arr[x];
		arr[x] = arr[7-x];
		arr[7-x] = temp;
	}

	memcpy(&value, arr, 8);
	return value;
} 

//==========================================================================

std::string strRep(std::string a,std::string b,std::string c)
{
	std::string d;
	std::string::size_type p;
	bool found;

	do 
	{
		if(found = (p = a.find(b)) != std::string::npos)
		{
			d += a.substr(0,p) + c;
			a = a.substr(p+b.size());
		}
	}
	while(found);

	d += a;

	return d;
} 

//==========================================================================
