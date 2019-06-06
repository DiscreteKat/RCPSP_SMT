#include "RCPSPSolver.h"
#include <algorithm>
#include <string>

#include <yices_c.h>
#include <yicesl_c.h>

#include "Util.h"

#define INF 999999999
//#define DEBUG_PRECOMPUTE
namespace MyRCPSP
{
	RCPSPSolver::RCPSPSolver()
	{
	}

	RCPSPSolver::~RCPSPSolver()
	{
	}

	bool RCPSPSolver::Solve(Project * proj)
	{
		float ms = 0.0f;
		/*Precompute*/
		std::cout << "Precomputation:" << std::endl;
		Timer::Tic();
		CalcExpendedEdge(proj);
		CalcBound(proj);
		CalcWindows(proj);
		CalcIncompatibity(proj);
		ms = Timer::Toc();
		//
		int numN = proj->num_activities;
		int numM = proj->num_resources;
		std::cout << " LB: " << proj->lowerB << std::endl;
		std::cout << " UB: " << proj->upperB << std::endl;
		std::cout << " rumtime: " << ms << " ms \n";
		std::cout << std::endl;

		/*SMT-Slover*/
		std::cout << "Yices " << yices_version() << ":" << std::endl;
		Timer::Tic();
		bool flag = SolveSMT(proj);
		ms = Timer::Toc();
		proj->performance = ms;
		//
		if (flag) 
		{
			std::cout << " Satisfiable (cost: " << proj->cost << ")" << std::endl;
			std::cout << " S = (";
			for (int i = 0; i < numN; ++i)
			{
				std::cout << proj->schedule[i];
				if (i < numN - 1) { std::cout << ", "; }
			}
			std::cout << ") \n";
			std::cout << " Workspan: " << proj->schedule[numN-1] << "\n";
			std::cout << " rumtime: " << ms << " ms \n";
		}
		std::cout << std::endl;
		return false;
	}

	void RCPSPSolver::CalcExpendedEdge(Project * proj)
	{
		int num = proj->num_activities;
		const Activities& act = proj->activity;
		ExpendedEdge&    edge = proj->edge;

		for (size_t i = 0; i < act.size(); ++i) 
		{
			for (size_t j = 0; j < act.size(); ++j){ edge[i][j] = INF; }
			edge[i][i] = 0;

			for (size_t j = 0; j < act[i].successors.size(); ++j)
			{
				int idx = act[i].successors[j];
				edge[i][idx] = -act[i].duration;
			}
		}

		for (int k = 0; k < num; ++k)
		{
			for (int i = 0; i < num; ++i)
			{
				for (int j = 0; j < num; ++j)
				{
					if (edge[i][j] > edge[i][k] + edge[k][j])
					{
						edge[i][j] = edge[i][k] + edge[k][j];
					}
				}
			}
		}

		for (int i = 0; i < num; ++i)
		{
			for (int j = 0; j < num; ++j)
			{
				if (edge[i][j] < 0)
				{
					edge[i][j] = -edge[i][j];
				}
				else if (edge[i][j] > 0)
				{
					edge[i][j] = INF;
				}
			}
		}

#ifdef DEBUG_PRECOMPUTE
		std::cout<< "Extended Precedence:" <<std::endl;
		for (int i = 0; i < num; ++i)
		{
			for (int j = 0; j < num; ++j)
			{
				if (edge[i][j] == INF)
				{
					std::cout << "INF\t";
				}
				else
				{
					std::cout << edge[i][j] << "\t";
				}

			}
			std::cout << "\n";
		}
		std::cout << "\n";
#endif // DEBUG
	}

	void RCPSPSolver::CalcBound(Project * proj)
	{
		int numN = proj->num_activities;
		int numM = proj->num_resources;
		const Activities& act = proj->activity;
		const Consumption& con = proj->consumption;
		const Resources& res = proj->resource;
		//LB
		//Capacity Bound
		int lb2 = 0;
		for (int k = 0; k < numM; ++k)
		{
			int time = 0;
			for (int i = 0; i < numN; ++i)
			{
				time += act[i].duration * con[i][k];
			}
			time = (int)std::ceil((float)time / (float)res[k].capacity);
			if (time > lb2){lb2 = time;}
		}
		proj->lowerB = std::max(proj->edge[0][numN - 1], lb2);
		//UB
		int ub = 0;
		for (int i = 0; i < numN; ++i){ub += act[i].duration;}
		proj->upperB = ub;
	}

	void RCPSPSolver::CalcWindows(Project * proj)
	{
		int lb = proj->lowerB;
		int ub = proj->upperB;
		int numN = proj->num_activities;
		const ExpendedEdge& edge = proj->edge;
		Activities&   act = proj->activity;
		//
		for (int i = 0; i < numN-1; ++i)
		{
			act[i].lowerB = edge[0][i];
			act[i].upperB = ub - edge[i][numN-1];
		}

		act[numN - 1].lowerB = lb;
		act[numN - 1].upperB = ub;

#ifdef DEBUG_PRECOMPUTE
		std::cout << "Windows:" << std::endl;
		for (int i = 0; i < numN; ++i)
		{
			std::cout << "[" << act[i].lowerB << ", " << act[i].upperB << "]" << std::endl;
		}
		std::cout << "\n";
#endif // DEBUG
	}

	void RCPSPSolver::CalcIncompatibity(Project * proj)
	{
		int numN = proj->num_activities;
		int numM = proj->num_resources;
		const Activities& act = proj->activity;
		const Consumption& con = proj->consumption;
		const Resources& res = proj->resource;
		const ExpendedEdge& edge = proj->edge;
		Incompatibity& inc = proj->incompatibity;

		for (int i = 0; i < numN; ++i)
		{
			for (int j = i+1; j < numN; ++j)
			{
				if (edge[i][j] < INF) { continue; }
				for (int k = 0; k < numM; ++k)
				{
					if (con[i][k] + con[j][k] > res[k].capacity)
					{
						inc[i][j] = true;
						inc[j][i] = true;
					}
				}
			}
		}

#ifdef DEBUG_PRECOMPUTE
		std::cout << "Incompatibity:" << std::endl;
		for (int i = 0; i < numN; ++i)
		{
			for (int j = 0; j < numN; ++j)
			{
					std::cout << inc[i][j] << " ";
			}
			std::cout << "\n";
		}
		std::cout << "\n";
#endif // DEBUG
	}

	bool RCPSPSolver::SolveSMT(Project * proj)
	{
		const int numN = proj->num_activities;
		const int numM = proj->num_resources;
		const Activities& act = proj->activity;
		const Consumption& con = proj->consumption;
		const Resources& res = proj->resource;
		const Incompatibity& inc = proj->incompatibity;
		const ExpendedEdge& edge = proj->edge;
		////
		yices_context ctx = yices_mk_context();
		yices_set_maxsat_initial_cost(INF);
		//yices_set_max_num_conflicts_in_maxsat_iteration(1000);
		yices_set_max_num_iterations_in_maxsat(100);
		////
		yices_type ty_int = yices_mk_type(ctx, "int");
		yices_expr  zero = yices_mk_num(ctx, 0);
		yices_expr  one  = yices_mk_num(ctx, 1);
		//Declaim schedule and duration
		std::vector<yices_var_decl> sdecl(numN);
		std::vector<yices_expr> s(numN);
		std::vector<yices_expr> d(numN);
		for (int i = 0; i < numN; ++i)
		{
			std::string name("s" + std::to_string(i));
			sdecl[i] = yices_mk_var_decl(ctx, name.c_str(), ty_int);
			s[i] = yices_mk_var_from_decl(ctx, sdecl[i]);
			//
			d[i] = yices_mk_num(ctx, act[i].duration);
		}

		//Set activities constraints
		{//s0
			yices_expr  c = yices_mk_eq(ctx, s[0], zero);
			yices_assert(ctx, c);
		}
		{//s_{n+1}
			//Set weighted constraints (Optimization)
			for (int i = proj->lowerB; i <= proj->upperB; ++i)
			{
				yices_expr  t = yices_mk_num(ctx, i);
				yices_expr  c = yices_mk_ge(ctx, t, s[numN - 1]);
				yices_assert_weighted(ctx, c, 1);
			}
		}
		for (int i = 1; i < numN; ++i)//s1 ~ s_n
		{
			yices_expr  lb = yices_mk_num(ctx, act[i].lowerB);
			yices_expr  ub = yices_mk_num(ctx, act[i].upperB);
			yices_expr  c1 = yices_mk_ge(ctx, s[i], lb);
			yices_expr  c2 = yices_mk_le(ctx, s[i], ub);
			yices_assert(ctx, c1);
			yices_assert(ctx, c2);
			for (size_t jidx = 0; jidx < act[i].successors.size(); jidx++)
			{
				int j = act[i].successors[jidx];
				yices_expr args[2];
				args[0] = s[j];
				args[1] = s[i];
				yices_expr  sub = yices_mk_sub(ctx, args, 2);
				yices_expr  d = yices_mk_num(ctx, act[i].duration);
				yices_expr  c3  = yices_mk_ge(ctx, sub, d);
				yices_assert(ctx, c3);
			}
		}

		//Set incompatibity constraints
		for (int i = 0; i < numN; ++i)
		{
			for (int j = 0; j < numN; ++j)
			{
				if (inc[i][j])
				{
					yices_expr args[2];
					args[0] = s[i];
					args[1] = d[i];
					yices_expr  add1 = yices_mk_sum(ctx, args, 2);
					yices_expr  e1 = yices_mk_le(ctx, add1, s[j]);

					args[0] = s[j];
					args[1] = d[j];
					yices_expr  add2 = yices_mk_sum(ctx, args, 2);
					yices_expr  e2 = yices_mk_le(ctx, add2, s[i]);
					
					args[0] = e1;
					args[1] = e2;
					yices_expr  c = yices_mk_or(ctx, args, 2);

					yices_assert(ctx, c);
				}
			}
		}

		//Set resourse constraints (Task formulation)
		std::vector<std::vector<yices_expr>> z(numN-2, std::vector<yices_expr>(numN-2));
		for (int i = 1; i < numN-1; ++i)
		{
			for (int j = 1; j < numN-1; ++j)
			{
				if (i == j) { continue; } //i~=j
				//
				yices_expr zvar = yices_mk_fresh_bool_var(ctx);
				//
				if (edge[i][j] < INF || edge[j][i] < INF || inc[i][j])
				{
					z[i-1][j-1] = yices_mk_eq(ctx, zvar, zero);
				}
				else
				{
					yices_expr z1var = yices_mk_fresh_bool_var(ctx);
					yices_expr z2var = yices_mk_fresh_bool_var(ctx);
					yices_expr e1 = yices_mk_le(ctx,s[i],s[j]);
					yices_expr z1 = yices_mk_eq(ctx, z1var, e1);

					yices_expr args[2];
					args[0] = s[i];
					args[1] = d[i];
					yices_expr add1 = yices_mk_sum(ctx, args, 2);
					yices_expr e2 = yices_mk_lt(ctx, s[j], add1);
					yices_expr z2 = yices_mk_eq(ctx, z2var, e2);

					args[0] = z1;
					args[1] = z2;
					yices_expr c = yices_mk_and(ctx, args, 2);
					yices_expr e3 = yices_mk_eq(ctx, zvar, one);
					yices_expr e4 = yices_mk_eq(ctx, zvar, zero);

					z[i-1][j-1] = yices_mk_ite(ctx, c, e3, e4);
				}
				//
			}
		}

		for (int k = 0; k < numM; ++k)
		{
			for (int j = 1; j < numN-1; ++j)
			{
				std::vector<yices_expr> args(numN-2);
				for (int i = 1; i < numN-1; ++i)
				{
					if (i == j) 
					{ 
						args[i-1] = yices_mk_num(ctx, con[j][k]);
					}
					else
					{
						yices_expr arg2[2];
						arg2[0] = yices_mk_num(ctx, con[i][k]);
						arg2[1] = z[i-1][j-1];
						args[i-1] = yices_mk_mul(ctx, arg2, 2);
					}
				}
				yices_expr sum = yices_mk_sum(ctx, &args[0], numN-2);
				yices_expr b = yices_mk_num(ctx, res[k].capacity);
				yices_expr c = yices_mk_le(ctx, sum, b);
				yices_assert(ctx, c);
			}
		}

		bool flag = true;
		switch (yices_max_sat(ctx))
		{
		case l_true:
		{
			yices_model m = yices_get_model(ctx);
			proj->cost = yices_get_cost_as_double(m);
			for (int i = 0; i < numN; ++i)
			{
				long value;
				yices_get_int_value(m, sdecl[i], &value);
				proj->schedule[i] = (int)value;
			}
			flag = true;
			break;
		}
		case l_false:
			printf(" Unsatisfiable\n");
			flag = false;
			break;
		case l_undef:
			printf(" Unknown\n");
			flag = false;
			break;
		}
		////
		yices_del_context(ctx);// Cleanup
		return flag;
	}
}