/*
 * File:  Unit.hh
 * Author:  mikolas
 * Created on:  Thu Dec 29 22:29:07 CEST 2011
 * Copyright (C) 2011, Mikolas Janota
 */
#ifndef UNIT_HH_24023
#define UNIT_HH_24023
#include "minisat_auxiliary.h"
#include "collections.h"

using Minisat::lbool;
class Unit {
    public:
        explicit Unit(const CNF& cnf);
        ~Unit();
        bool                           propagate();
        lbool                          value(Var variable) const;
        void                           eval(CNF& cnf);
        Var                            size() { return values.size(); }
    private:
        bool                           conflict;           // a conflict was reached
        CNF                            original_clauses;   // original_clauses[i] corresponds to clauses[i]
        vector<bool>                   dirty_clauses;      // dirty_clauses[i] iff clause[i] modified
        vector< vector<size_t>* >      watches;
        vector<LiteralVector>          clauses;
        vector<Lit>                    trail;
        vector<lbool>                  values;
        lbool                          value(Lit literal) const;
        void                           mark_dirty(size_t clause_index);
        bool                           is_dirty(size_t clause_index) const;
        void                           set_value(Var variable, lbool value);
        bool                           propagate(Lit literal);
        bool                           add_clause(const LitSet& clause);
        void                           schedule(Lit literal);
        bool                           add_clauses(const CNF& cnf);
        void                           watch(Lit literal, size_t clause_index);
        inline size_t literal_index(Lit l) {
            assert(var(l)>=0);
            const size_t vi = (size_t) var(l);
            return sign(l) ? 2*vi : 2*vi+1;
        }
};
#endif /* UNIT_HH_24023 */
