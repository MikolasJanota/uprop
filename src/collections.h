/* 
 * File:   qtypes.hh
 * Author: mikolas
 *
 * Created on January 12, 2011, 5:32 PM
 */

#ifndef Q2TYPES_HH
#define	Q2TYPES_HH
#include <utility>
#include <vector>
#include "minisat/core/SolverTypes.h"
#include "LitSet.h"
#include "minisat_auxiliary.h"
#include <iostream>
#include <unordered_map>
#include <unistd.h>

typedef std::vector<Minisat::Var>                           VariableVector;

typedef std::unordered_map<int,int> Int2Int;
typedef std::unordered_map<int,Lit> Int2Lit;
typedef std::vector<LitSet> CNF;

ostream & operator << (std::ostream& outs, const CNF& f);
ostream & operator << (std::ostream& outs, const VariableVector& ls);
#endif	/* Q2TYPES_HH */

