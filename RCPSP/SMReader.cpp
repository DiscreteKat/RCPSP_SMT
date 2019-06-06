#include "SMReader.h"

namespace MyRCPSP
{
	SMReader::SMReader()
	{
	}


	SMReader::~SMReader()
	{
	}

	bool SMReader::ReadFile(std::string filename, Project * proj)
	{
		std::ifstream input(filename);
		
		std::cout <<"Input: "<< filename.substr(filename.rfind("/")+1) << std::endl;
		if (input.is_open())
		{
			std::string line;
			int n, m;
			bool flag = GetInfo(input, &n, &m);
			*proj = Project(n, m);
			if (!flag)
			{
				std::cout << "Error format!" << std::endl;
				return false;
			}
			//
			flag = GetRelations(input, proj);
			if (!flag)
			{
				std::cout << "Error format!" << std::endl;
				return false;
			}
			//
			flag = GetActivityInfo(input, proj);
			if (!flag)
			{
				std::cout << "Error format!" << std::endl;
				return false;
			}
			//
			flag = GetResourceInfo(input, proj);
			if (!flag)
			{
				std::cout << "Error format!" << std::endl;
				return false;
			}
			input.close();
			return true;
		}
		else
		{
			std::cout << "Unable to open file \n";
			return false;
		}
	}

	bool SMReader::Skip(std::ifstream & input, int num)
	{
		std::string line;
		for (int i = 0; i < num; ++i)
		{
			if (!std::getline(input, line)){return false;}
		}
		return false;
	}

	bool SMReader::GetInt(std::stringstream & stream, int * value)
	{
		while (!stream.eof()) {
			std::string temp;
			/* extracting word by word from stream */
			stream >> temp;
			/* Checking the given word is integer or not */
			if (std::stringstream(temp) >> *value) { return true; }
			/* To save from space at the end of string */
			temp = "";
		}
		return false;
	}

	bool SMReader::GetInfo(std::ifstream& input, int * num_A, int * num_R)
	{
		std::string line;
		bool flag_A = false;
		bool flag_R = false;
		while (std::getline(input, line))
		{
			if (!flag_A)
			{
				std::size_t found = line.find("jobs");
				if (found != std::string::npos)
				{
					std::stringstream ss;
					/* Storing the whole string into string stream */
					ss << line;
					if (GetInt(ss, num_A)){flag_A = true;}
					else { return false; }
				}
			}
			else if (!flag_R)
			{
				std::size_t found = line.find("renewable");
				
				if (found != std::string::npos)
				{
					std::stringstream ss;
					/* Storing the whole string into string stream */
					ss << line;
					if (GetInt(ss, num_R)){flag_R = true;}
					else{return false;}
				}
			}

			if (flag_A && flag_R) { return true; }
		}
		return false;
	}

	bool SMReader::GetRelations(std::ifstream & input, Project * proj)
	{
		std::string line;
		while (std::getline(input, line))
		{
			std::size_t found = line.find("PRECEDENCE RELATIONS:");
			if (found != std::string::npos)
			{
				Skip(input, 1);
				int count=0;
				while (std::getline(input, line) && count< proj->num_activities)
				{
					std::stringstream ss;
					/* Storing the whole string into string stream */
					ss << line;
					int index, modes, num_succ;
					if (GetInt(ss, &index) && GetInt(ss, &modes) && GetInt(ss, &num_succ))
					{
						for (int j = 0; j < num_succ; j++)
						{
							int succ_index;
							if (GetInt(ss, &succ_index))
							{
								proj->activity[index-1].successors.push_back(succ_index-1);
							}
							else{return false;}
						}
					}
					else{return false;}
					count++;
				}
				if (count == proj->num_activities) { return true; }
			}
		}
		return false;
	}

	bool SMReader::GetActivityInfo(std::ifstream & input, Project * proj)
	{
		std::string line;
		while (std::getline(input, line))
		{
			std::size_t found = line.find("REQUESTS/DURATIONS:");
			if (found != std::string::npos)
			{
				Skip(input, 2);
				int count = 0;
				while (std::getline(input, line) && count < proj->num_activities)
				{
					std::stringstream ss;
					/* Storing the whole string into string stream */
					ss << line;
					int index, modes, duration;
					if (GetInt(ss, &index) && GetInt(ss, &modes) && GetInt(ss, &duration))
					{
						proj->activity[index-1].index    = index-1;
						proj->activity[index-1].duration = duration;

						for (int k = 0; k < proj->num_resources; k++)
						{
							int value;
							if (GetInt(ss, &value))
							{
								proj->consumption[index-1][k] = value;
							}
							else { return false; }
						}
					}
					else { return false; }
					count++;
				}
				if (count == proj->num_activities) { return true; }
			}
		}
		return false;
	}

	bool SMReader::GetResourceInfo(std::ifstream & input, Project * proj)
	{
		std::string line;
		while (std::getline(input, line))
		{
			std::size_t found = line.find("RESOURCEAVAILABILITIES:");
			if (found != std::string::npos)
			{
				Skip(input, 1);
				std::getline(input, line);
				std::stringstream ss;
				/* Storing the whole string into string stream */
				ss << line;
				for (int k = 0; k < proj->num_resources; k++)
				{
					int cap;
					if (GetInt(ss, &cap))
					{
						proj->resource[k].capacity = cap;
					}
					else { return false; }
				}
				return true;
			}
		}
		return false;
	}
}