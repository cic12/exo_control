/* This file is part of GRAMPC - (https://sourceforge.net/projects/grampc/)
 *
 * GRAMPC -- A software framework for embedded nonlinear model predictive
 * control using a gradient-based augmented Lagrangian approach
 *
 * Copyright (C) 2014-2018 by Tobias Englert, Knut Graichen, Felix Mesmer,
 * Soenke Rhein, Andreas Voelz, Bartosz Kaepernick (<v2.0), Tilman Utz (<v2.0).
 * Developed at the Institute of Measurement, Control, and Microtechnology,
 * Ulm University. All rights reserved.
 *
 * GRAMPC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * GRAMPC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GRAMPC. If not, see <http://www.gnu.org/licenses/>
 *
 */


#include "problem_description.hpp"
#include "math.h"

 /* square macro */
#define POW2(a) ((a)*(a))

extern "C"
{
	void ocp_dim(typeInt *Nx, typeInt *Nu, typeInt *Np, typeInt *Ng, typeInt *Nh, typeInt *NgT, typeInt *NhT, typeUSERPARAM *userparam)
	{
		*Nx = 6; // 2 system states and 4 dummy states
		*Nu = 1;
		*Np = 0;
		*Nh = 0;
		*Ng = 0;
		*NgT = 0;
		*NhT = 0;

		//((grampc::ProblemDescription*)userparam)->ocp_dim(Nx, Nu, Np, Ng, Nh, NgT, NhT);
	}

	void ffct(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, typeUSERPARAM *userparam)
	{
		ctypeRNum* pSys = (ctypeRNum*)userparam;
		ctypeRNum A = pSys[0];
		ctypeRNum B = pSys[1];
		ctypeRNum J = pSys[2];
		ctypeRNum tau_g = pSys[3];

		out[0] = x[1];

		out[1] = (u[0] + x[2] - A * (-0.1e1 + 0.2e1 / (exp(-(15 * x[1])) + 0.1e1)) - (B * x[1]) - tau_g * sin(x[0])) / J;
		// x[2] is modelled human torque (estimate) 

		// input states

		out[2] = 0;	// human torque estimate
		out[3] = 0; // control torque
		out[4] = 0; // m1
		out[5] = 0; // m2

		//((grampc::ProblemDescription*)userparam)->ffct(out, t, x, u, p);
	}
	void dfdx_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *adj, ctypeRNum *u, ctypeRNum *p, typeUSERPARAM* userparam)
	{
		ctypeRNum* pSys = (ctypeRNum*)userparam;
		ctypeRNum A = pSys[0];
		ctypeRNum B = pSys[1];
		ctypeRNum J = pSys[2];
		ctypeRNum tau_g = pSys[3];

		out[0] = adj[1] * (-tau_g * cos(x[0]) / J);

		out[1] = adj[0] + adj[1] * ((-0.30e2 * A * pow(exp(-(15 * x[1])) + 0.1e1, -0.2e1) * exp(-(15 * x[1])) - B) / J);

		// input states

		out[2] = adj[1] * (0.1e1 / J);
		out[3] = 0;
		out[4] = 0;
		out[5] = 0;

		//((grampc::ProblemDescription*)userparam)->dfdx_vec(out, t, x, adj, u, p);
	}
	void dfdu_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *adj, ctypeRNum *u, ctypeRNum *p, typeUSERPARAM* userparam)
	{
		ctypeRNum* pSys = (ctypeRNum*)userparam;
		ctypeRNum J = pSys[2];

		out[0] = adj[1] * (0.1e1 / J);

		//((grampc::ProblemDescription*)userparam)->dfdu_vec(out, t, x, adj, u, p);
	}
	void dfdp_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *adj, ctypeRNum *u, ctypeRNum *p, typeUSERPARAM* userparam)
	{
		//((grampc::ProblemDescription*)userparam)->dfdp_vec(out, t, x, adj, u, p);
	}

	void lfct(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, ctypeRNum *xdes, ctypeRNum *udes, typeUSERPARAM* userparam)
	{
		ctypeRNum *pCost = (ctypeRNum*)userparam;
		ctypeRNum w_theta = pCost[4];
		ctypeRNum w_tau = pCost[5];
		ctypeRNum p_ass = pCost[6];
		ctypeRNum p_low = pCost[7];
		ctypeRNum p_stop = pCost[8];

		ctypeRNum sigm_a = 200;
		ctypeRNum sigm_c = 0.05;

		typeRNum M = 1;//-p_ass / (1 + exp(-sigm_a * (2 / (1 + exp(-sigm_a * x[3])) - 1) * (2 / (1 + exp(-sigm_a * x[4] + sigm_a * x[5])) - 1) + sigm_c)) - p_low * x[4] * x[5] - p_stop / (1 + exp(-sigm_a * (2 / (1 + exp(sigm_a * x[3])) - 1) * (2 / (1 + exp(-sigm_a * x[4] + sigm_a * x[5])) - 1) + sigm_c)) + 1;

		/*if ((x[4] > 0.05 && u[0] < -0.1) || (x[5] > 0.05 && u[0] > 0.1)) {
			M = 1 - p_stop;
		}
		else if ((x[4] > 0.01 && u[0] > -0.1) || (x[5] > 0.01 && u[0] < 0.1)) {
			M = 1 - p_ass;
		}*/

		out[0] = M * w_theta * POW2((x[0] - xdes[0])) +
			w_tau * POW2((u[0] - udes[0]));

		//((grampc::ProblemDescription*)userparam)->lfct(out, t, x, u, p, xdes, udes);
	}
	void dldx(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, ctypeRNum *xdes, ctypeRNum *udes, typeUSERPARAM* userparam)
	{
		ctypeRNum *pCost = (ctypeRNum*)userparam;
		ctypeRNum w_theta = pCost[4];
		ctypeRNum p_ass = pCost[6];
		ctypeRNum p_low = pCost[7];
		ctypeRNum p_stop = pCost[8];

		ctypeRNum sigm_a = 200;
		ctypeRNum sigm_c = 0.05;

		typeRNum M = 1;// -p_ass / (1 + exp(-sigm_a * (2 / (1 + exp(-sigm_a * x[3])) - 1) * (2 / (1 + exp(-sigm_a * x[4] + sigm_a * x[5])) - 1) + sigm_c)) - p_low * x[4] * x[5] - p_stop / (1 + exp(-sigm_a * (2 / (1 + exp(sigm_a * x[3])) - 1) * (2 / (1 + exp(-sigm_a * x[4] + sigm_a * x[5])) - 1) + sigm_c)) + 1;

		//if ((x[4] > 0.01 && u[0] < -0.1) || (x[5] > 0.05 && u[0] > 0.1)) {
		//	//M = 1 - p_stop;
		//}
		//else if ((x[4] > 0.01 && u[0] > -0.1) || (x[5] > 0.05 && u[0] < 0.1)) {
		//	M = 1 - p_ass;
		//}

		out[0] = 2 * M * w_theta * (x[0] - xdes[0]);

		//((grampc::ProblemDescription*)userparam)->dldx(out, t, x, u, p, xdes, udes);
	}
	void dldu(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, ctypeRNum *xdes, ctypeRNum *udes, typeUSERPARAM* userparam)
	{
		ctypeRNum *pCost = (ctypeRNum*)userparam;
		ctypeRNum w_tau = pCost[5];

		out[0] = 2 * w_tau * (u[0] - udes[0]);

		//((grampc::ProblemDescription*)userparam)->dldu(out, t, x, u, p, xdes, udes);
	}
	void dldp(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, ctypeRNum *xdes, ctypeRNum *udes, typeUSERPARAM* userparam)
	{
		//((grampc::ProblemDescription*)userparam)->dldp(out, t, x, u, p, xdes, udes);
	}


	void Vfct(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, ctypeRNum *xdes, typeUSERPARAM* userparam)
	{
		//((grampc::ProblemDescription*)userparam)->Vfct(out, t, x, p, xdes);
	}
	void dVdx(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, ctypeRNum *xdes, typeUSERPARAM* userparam)
	{
		//((grampc::ProblemDescription*)userparam)->dVdx(out, t, x, p, xdes);
	}
	void dVdp(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, ctypeRNum *xdes, typeUSERPARAM* userparam)
	{
		//((grampc::ProblemDescription*)userparam)->dVdp(out, t, x, p, xdes);
	}
	void dVdT(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, ctypeRNum *xdes, typeUSERPARAM* userparam)
	{
		//((grampc::ProblemDescription*)userparam)->dVdT(out, t, x, p, xdes);
	}


	void gfct(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, typeUSERPARAM* userparam)
	{
		//((grampc::ProblemDescription*)userparam)->gfct(out, t, x, u, p);
	}
	void dgdx_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM *userparam)
	{
		//((grampc::ProblemDescription*)userparam)->dgdx_vec(out, t, x, u, p, vec);
	}
	void dgdu_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		//((grampc::ProblemDescription*)userparam)->dgdu_vec(out, t, x, u, p, vec);
	}
	void dgdp_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		//((grampc::ProblemDescription*)userparam)->dgdp_vec(out, t, x, u, p, vec);
	}


	void hfct(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, typeUSERPARAM* userparam)
	{
		//((grampc::ProblemDescription*)userparam)->hfct(out, t, x, u, p);
	}
	void dhdx_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		//((grampc::ProblemDescription*)userparam)->dhdx_vec(out, t, x, u, p, vec);
	}
	void dhdu_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		//((grampc::ProblemDescription*)userparam)->dhdu_vec(out, t, x, u, p, vec);
	}
	void dhdp_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		//((grampc::ProblemDescription*)userparam)->dhdp_vec(out, t, x, u, p, vec);
	}


	void gTfct(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, typeUSERPARAM* userparam)
	{
		//((grampc::ProblemDescription*)userparam)->gTfct(out, t, x, p);
	}
	void dgTdx_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		//((grampc::ProblemDescription*)userparam)->dgTdx_vec(out, t, x, p, vec);
	}
	void dgTdp_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		//((grampc::ProblemDescription*)userparam)->dgTdp_vec(out, t, x, p, vec);
	}
	void dgTdT_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		//((grampc::ProblemDescription*)userparam)->dgTdT_vec(out, t, x, p, vec);
	}


	void hTfct(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, typeUSERPARAM* userparam)
	{
		//((grampc::ProblemDescription*)userparam)->hTfct(out, t, x, p);
	}
	void dhTdx_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		//((grampc::ProblemDescription*)userparam)->dhTdx_vec(out, t, x, p, vec);
	}
	void dhTdp_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		//((grampc::ProblemDescription*)userparam)->dhTdp_vec(out, t, x, p, vec);
	}
	void dhTdT_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		//((grampc::ProblemDescription*)userparam)->dhTdT_vec(out, t, x, p, vec);
	}


	void dfdx(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, typeUSERPARAM *userparam)
	{
		//((grampc::ProblemDescription*)userparam)->dhTdT_vec(out, t, x, u, p);
	}
	void dfdxtrans(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, typeUSERPARAM *userparam)
	{
		//((grampc::ProblemDescription*)userparam)->dhTdT_vec(out, t, x, u, p);
	}
	void dfdt(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, typeUSERPARAM *userparam)
	{
		//((grampc::ProblemDescription*)userparam)->dhTdT_vec(out, t, x, u, p);
	}
	void dHdxdt(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *adj, ctypeRNum *p, typeUSERPARAM *userparam)
	{
		//((grampc::ProblemDescription*)userparam)->dHdxdt(out, t, x, u, adj, p);
	}
	void Mfct(typeRNum *out, typeUSERPARAM *userparam)
	{
		//((grampc::ProblemDescription*)userparam)->Mfct(out);
	}
	void Mtrans(typeRNum *out, typeUSERPARAM *userparam)
	{
		//((grampc::ProblemDescription*)userparam)->Mtrans(out);
	}

}
