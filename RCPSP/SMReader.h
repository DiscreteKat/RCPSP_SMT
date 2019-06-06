#pragma once
/* Description: This reader parses sm format from PSPlib to RSPSP data.
				PSPlib(http://www.om-db.wi.tum.de/psplib/)
*  Version:  1.0
*  Created: 2019
*  Revision: none
*  Compiler: VS2017
*  Author:  Shih-Hsuan Hung
*/
#include<string>
#include<iostream>
#include<fstream>
#include<sstream> 

#include"RCPSPData.h"

namespace MyRCPSP
{
	class SMReader
	{
	public:
		 SMReader();
		~SMReader();
	public:
		bool ReadFile(std::string filename, Project* proj);
	private:
		bool Skip(std::ifstream& input,int num);
		bool GetInt(std::stringstream& stream, int* value);
		//Start from section 2
		bool GetInfo(std::ifstream& input, int* num_A, int* num_R);
		bool GetRelations(std::ifstream& input, Project* proj);
		bool GetActivityInfo(std::ifstream& input, Project* proj);
		bool GetResourceInfo(std::ifstream& input, Project* proj);
	};
}