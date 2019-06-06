#pragma once
/* Description: Structures of the resource constrained project scheduling problems.
*  Version:  1.0
*  Created: 2019
*  Revision: none
*  Compiler: VS2017
*  Author:  Shih-Hsuan Hung
*/
#include<iostream>
#include<vector>

namespace MyRCPSP
{
	struct Activity
	{
		int index;
		int duration;
		std::vector<int> successors;
		//Mid-product
		int lowerB;
		int upperB;
		Activity() :index(0), duration(0), successors(std::vector<int>()), lowerB(0), upperB(0){}
	};
	typedef std::vector<Activity> Activities;

	struct Resource
	{
		int capacity;
		Resource() :capacity(0) {}
	};
	typedef std::vector<Resource> Resources;

	typedef std::vector<int> Schedule;

	typedef std::vector<std::vector<int>> Consumption; //Consumption[A][R] of the activities
	typedef std::vector<std::vector<int>> ExpendedEdge;
	typedef std::vector<std::vector<bool>> Incompatibity;

	struct Project
	{
		//Input
		int		num_activities;
		int		num_resources;
		Consumption consumption;
		Activities activity;
		Resources  resource;
		//Output
		Schedule schedule;
		double   cost;
		float    performance;
		//Mid-product
		ExpendedEdge edge;
		int lowerB;
		int upperB;
		Incompatibity incompatibity;

		Project() :num_activities(0), num_resources(0), lowerB(0), upperB(0), cost(0), performance(0){}
		Project(int n,int m) :num_activities(n), num_resources(m), lowerB(0), upperB(0), cost(0), performance(0),
			activity(Activities(n)), resource(Resources(m)), consumption(Consumption(n, std::vector<int>(m, 0))),
			schedule(Schedule(n)), edge(ExpendedEdge(n, std::vector<int>(n, 0))), incompatibity(Incompatibity(n, std::vector<bool>(n, false)))
		{}

		void print() 
		{
			std::cout << "# of Activities:\t" << num_activities-2 << "+2" << "\n";
			std::cout << "# of Resources:\t\t" << num_resources << "\n";
			std::cout << "Resources:"<< "\n";
			for (size_t i = 0; i < resource.size(); ++i){std::cout << "R" << i+1 << "\t";}
			std::cout << "\n";
			for (size_t i = 0; i < resource.size(); ++i)
			{
				std::cout << resource[i].capacity << "\t";
			}
			std::cout << "\n";
			std::cout << "Activities:\n";
			std::cout << "Index\tDuration\tConsumption\t\tSuccessors\n";
			for (size_t i = 0; i < activity.size(); ++i)
			{
				std::cout << activity[i].index << "\t" << activity[i].duration << "\t\t(";
				for (size_t k = 0; k < resource.size(); ++k)
				{
					std::cout << consumption[i][k];
					if (k != resource.size() - 1) { std::cout << ", "; }
				}
				std::cout<< ")\t\t" << "{";
				for (size_t j = 0; j < activity[i].successors.size(); ++j)
				{
					std::cout << activity[i].successors[j];
					if (j != activity[i].successors.size() - 1) { std::cout << ", "; }
				}
				std::cout << "}";
				std::cout << "\n";
			}
		}
	};
}