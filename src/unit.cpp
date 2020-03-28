/*
 * File:  Unit.cc
 * Author:  mikolas
 * Created on:  Thu Dec 29 22:29:15 CEST 2011
 * Copyright (C) 2011, Mikolas Janota
 */
#include "unit.h"

Unit::Unit(const CNF& cnf) : conflict(false)
{
    add_clauses(cnf);
}
Unit::~Unit() {
    for (auto w : watches)
        delete w;
}

bool Unit::add_clauses(const CNF& cnf) {
    for (const LitSet& c : cnf)
        if (!add_clause(c))
            return false;
    return true;
}


void Unit::eval(CNF& cnf) {
    if (conflict) {
        cnf.push_back(LitSet());
        return;
    }

    LiteralVector ls;
    for (size_t i=0; i < clauses.size(); ++i) {
        const LiteralVector& clause = clauses[i];
        ls.clear();
        bool taut=false;
        for (Lit literal : clause) {
            const lbool v = value(literal);
            if (v==l_Undef) {
                ls.push_back(literal);
            } else if(v==l_True) {
                taut=true;
                break;
            } else {
                assert (v==l_False);
                dirty_clauses[i]=true;
            }
        }
        if (!taut) {
            if (dirty_clauses[i]) {
                cnf.push_back(LitSet::mk(ls));
            } else {
                const LitSet& orig_cl = original_clauses[i];
                assert (LitSet(ls).equal(orig_cl));
                cnf.push_back(orig_cl);
            }
        }
    }
}

bool Unit::add_clause(const LitSet& clause) {
    clauses.resize(clauses.size()+1);
    const size_t clause_index = clauses.size() - 1;
    LiteralVector& ls = clauses[clause_index];
    ls.clear();
    bool taut=false;
    bool change=false;
    for (Lit literal : clause) {
        const lbool     v = value(literal);
        if (v==l_Undef) ls.push_back(literal);
        else {
            change=true;
            if (v==l_True) {
                taut=true;
                break;
            }
        }
    }
    if (taut) {
        clauses.pop_back();  // discard
        return true;
    }

    if (ls.size()==1) { // unit
        schedule(ls[0]);
        clauses.pop_back();   // discard
        return true;
    }

    if (ls.size()==0) { //confl
        clauses.pop_back();
        conflict=true;
        return false;
    }

    assert(clause.size() >= 2);
    original_clauses.push_back(clause);
    assert(dirty_clauses.size()==clause_index);
    dirty_clauses.resize(clause_index+1,false);
    if (change) dirty_clauses[clause_index]=true;
    // setup watches
    watch(~ls[0], clause_index);
    watch(~ls[1], clause_index);
    return true;
}


bool Unit::propagate() {
    while (!trail.empty() && !conflict) {
        const Lit literal = trail.back();
        trail.pop_back();
        if (!propagate(literal))
            conflict=true;
    }
    return !conflict;
}

void Unit::schedule(Lit literal) {
    trail.push_back(literal);
}

void Unit::watch(Lit literal, size_t clause_index) {
    const size_t index = literal_index(literal);
    if (index >= watches.size()) {
        watches.resize(index+1);
        watches[index] = new vector<size_t>();
    }
    watches[index]->push_back(clause_index);
}

void Unit::set_value(Var variable, lbool value) {
    const size_t index = (size_t) variable;
    if (index >= values.size()) values.resize(index+1, l_Undef);
    values[index]=value;
}


lbool Unit::value(Var variable) const {
    const size_t index = (size_t) variable;
    if (index >= values.size()) return l_Undef;
    return values[index];
}

lbool Unit::value(Lit literal) const {
    const lbool v = value(var(literal));
    if (v==l_Undef) return l_Undef;
    return (v==l_False) == sign(literal) ? l_True : l_False;
}

bool Unit::propagate(Lit literal) {
    const Var variable = var(literal);
    const lbool literal_value = sign(literal) ? l_False : l_True;
    if (value(variable) != l_Undef) {
        return (literal_value == value(variable));
    }
    set_value(variable, literal_value);
    const Lit false_literal = ~literal;
    const size_t li = literal_index(literal);
    if (li >= watches.size())
        return true; // the literal does not watch anything
    const vector<size_t>* const w = watches[li];
    bool return_value = true;
    for (size_t clause_index: *w) {
        assert(clause_index<clauses.size());
        LiteralVector& clause = clauses[clause_index];
        if (clause[0] == false_literal) {
            clause[0] = clause[1];
            clause[1] = false_literal;
        }
        assert(clause[1] == false_literal);
        if (value(clause[0]) == l_True)
            continue; // clause already true
        size_t new_watch = 0;
        for (size_t index = 2; index < clause.size(); ++index) { // find a new watch
            if (value(clause[index]) != l_False) {
                new_watch = index;
                break;
            }
        }
        if (new_watch > 0) { // new watch found
            assert(new_watch > 1);
            watch(~clause[new_watch], clause_index);
            clause[1] = clause[0];
            clause[0] = clause[new_watch];
            clause[new_watch] = false_literal;
        } else { // no new watch found
            if (value(clause[0]) == l_False) {
                return_value = false; //  conflict
                break;
            }
            //  unit clause
            assert(value(clause[0]) == l_Undef);
            schedule(clause[0]);
        }
    }
    return return_value;
}
