#include"SMReader.h"
#include"RCPSPSolver.h"

int main(int argc, char *argv[])
{
	MyRCPSP::SMReader reader;
	MyRCPSP::Project project;
	MyRCPSP::RCPSPSolver solver;
	//
	bool flag = false;
	if (argc == 2)
	{
		flag = reader.ReadFile(argv[1], &project);
	}
	else
	{
		flag = reader.ReadFile("../Data/example.sm", &project);
	}
	
	if (flag) 
	{
	std::cout << "------------------------------------------------------------" << std::endl;
	project.print();
	std::cout << "------------------------------------------------------------" << std::endl;
	solver.Solve(&project);
	}
	system("pause");
	return 0;
}