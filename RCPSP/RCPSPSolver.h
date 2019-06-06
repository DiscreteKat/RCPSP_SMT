#pragma once
/* Description: This RCPSP solver implemented the reseach[1] with the task formulation of SMT
				[1] Carlos  Ansotegui,  Miquel  Bofill,  Miquel  Palahi,  Josep  Suy,  and  Mateu  Villaret.   Satisfiability  mod-ulo theories:  An efficient approach for the resource-constrained project scheduling problem.  InNinthSymposium of Abstraction, Reformulation, and Approximation, 2011.
*  Version:  1.0
*  Created: 2019
*  Revision: none
*  Compiler: VS2017
*  Author:  Shih-Hsuan Hung
*/
#include"RCPSPData.h"

namespace MyRCPSP
{
	class RCPSPSolver
	{
	public:
		 RCPSPSolver();
		~RCPSPSolver();
	public:
		bool Solve(Project* proj);
	private:
		void CalcExpendedEdge(Project * proj);
		void CalcBound(Project * proj);
		void CalcWindows(Project * proj);
		void CalcIncompatibity(Project * proj);

		bool SolveSMT(Project * proj);
	};
}

