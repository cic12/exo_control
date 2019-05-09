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

		// input states

		out[2] = 0;	// human torque estimate
		out[3] = 0; // M
		out[4] = 0;
		out[5] = 0;
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
	}
	void dfdu_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *adj, ctypeRNum *u, ctypeRNum *p, typeUSERPARAM* userparam)
	{
		ctypeRNum* pSys = (ctypeRNum*)userparam;
		ctypeRNum J = pSys[2];

		out[0] = adj[1] * (0.1e1 / J);
	}
	void dfdp_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *adj, ctypeRNum *u, ctypeRNum *p, typeUSERPARAM* userparam)
	{
		
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

		typeRNum M = 1;

		out[0] = M * w_theta * POW2((x[0] - xdes[0])) +
			w_tau * POW2((u[0] - udes[0]));
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

		typeRNum M = 1;

		out[0] = 2 * M * w_theta * (x[0] - xdes[0]);
	}
	void dldu(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, ctypeRNum *xdes, ctypeRNum *udes, typeUSERPARAM* userparam)
	{
		ctypeRNum *pCost = (ctypeRNum*)userparam;
		ctypeRNum w_tau = pCost[5];

		out[0] = 2 * w_tau * (u[0] - udes[0]);
	}
	void dldp(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, ctypeRNum *xdes, ctypeRNum *udes, typeUSERPARAM* userparam)
	{

	}


	void Vfct(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, ctypeRNum *xdes, typeUSERPARAM* userparam)
	{
		
	}
	void dVdx(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, ctypeRNum *xdes, typeUSERPARAM* userparam)
	{
		
	}
	void dVdp(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, ctypeRNum *xdes, typeUSERPARAM* userparam)
	{
		
	}
	void dVdT(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, ctypeRNum *xdes, typeUSERPARAM* userparam)
	{
		
	}


	void gfct(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, typeUSERPARAM* userparam)
	{
		
	}
	void dgdx_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM *userparam)
	{
		
	}
	void dgdu_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		
	}
	void dgdp_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		
	}


	void hfct(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, typeUSERPARAM* userparam)
	{
		
	}
	void dhdx_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		
	}
	void dhdu_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		
	}
	void dhdp_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		
	}


	void gTfct(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, typeUSERPARAM* userparam)
	{
		
	}
	void dgTdx_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		
	}
	void dgTdp_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		
	}
	void dgTdT_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		
	}


	void hTfct(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, typeUSERPARAM* userparam)
	{
		
	}
	void dhTdx_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		
	}
	void dhTdp_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		
	}
	void dhTdT_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM* userparam)
	{
		
	}


	void dfdx(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, typeUSERPARAM *userparam)
	{
		
	}
	void dfdxtrans(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, typeUSERPARAM *userparam)
	{
		
	}
	void dfdt(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *p, typeUSERPARAM *userparam)
	{
		
	}
	void dHdxdt(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u, ctypeRNum *adj, ctypeRNum *p, typeUSERPARAM *userparam)
	{
		
	}
	void Mfct(typeRNum *out, typeUSERPARAM *userparam)
	{
		
	}
	void Mtrans(typeRNum *out, typeUSERPARAM *userparam)
	{
		
	}
}
