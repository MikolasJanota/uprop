/*
 * File:  main.cpp
 * Author:  mikolas
 * Created on:  Sat Mar 28 13:58:57 WET 2020
 * Copyright (C) 2020, Mikolas Janota
 */

#include "unit.h"
#include "defs.h"
#include "minisat/utils/System.h"
#include "minisat/utils/ParseUtils.h"
#include "minisat/utils/Options.h"
#include "minisat/core/Dimacs.h"
#include "minisat/core/Solver.h"
#include <cstdlib>
#include <string>
#include <iostream>
#include <unordered_map>
#include "auxiliary.h"
#include "ReadCNF.h"
using namespace std;
int run_cnf(const string& flafile);

int main(int argc, char** argv) {
#ifndef NDEBUG
    cout << "c DEBUG version." << endl;
#endif
    cout<<"c uprop, v00.0, "<<GITHEAD<<endl;
    cout<<"c (C) 2020 Mikolas Janota, mikolas.janota@gmail.com"<<endl;
    const string flafile(argc>1 ? argv[1] : "-");
    if (flafile=="-") cout<<"c reading from standard input"<<endl;
    else cout<<"c reading from "<<flafile<<endl;
    return run_cnf(flafile);
}

int run_cnf(const string& flafile) {
    scoped_ptr<Reader> fr;
    gzFile ff=Z_NULL;
    if (flafile.size()==1 && flafile[0]=='-') {
        fr.attach(new Reader(cin));
    } else {
        ff = gzopen(flafile.c_str(), "rb");
        if (ff == Z_NULL) {
            cerr << "ERROR: " << "Unable to open file: " << flafile << endl;
            cerr << "ABORTING" << endl;
            exit(1);
        }
        fr.attach(new Reader(ff));
    }
    ReadCNF reader(*fr);
    try {
        reader.read();
    } catch (ReadException& rex) {
        cerr << "ERROR: " << rex.what() << endl;
        cerr << "ABORTING" << endl;
        exit(EXIT_FAILURE);
    }
    cout<<"c done reading: "<<read_cpu_time()<<std::endl;
    if (!reader.get_header_read()) {
        cerr << "ERROR: Missing header." << endl;
        cerr << "ABORTING" << endl;
        exit(EXIT_FAILURE);
    }

    Unit up(reader.get_clauses());
    const bool original_propagation = up.propagate();
    if (!original_propagation) {
        cout << "Original propagation already failed." << endl;
        return EXIT_SUCCESS;
    }
    bool fixpoint;
    int  cycle_count = 0;
    bool all_defined;
    int  failed_literal_counter = 0;
    do {
        cout << "== CYCLE " <<  ++cycle_count << endl;
        fixpoint = true;
        all_defined = true;
        for (Var v = 1; v <= reader.get_max_id(); v++) {
            if (up.value(v) != l_Undef) {
                cout << v << " already set to " << up.value(v) << endl;
                continue;
            }
            all_defined = false;

            bool unsatisfiable = false;
            if (up.is_failed_lit(mkLit(v))) {
                cout << "FAILED: " << mkLit(v) << endl;
                unsatisfiable = !up.assert_lit(~mkLit(v));
                fixpoint = false;
                ++failed_literal_counter;
            } else if (up.is_failed_lit(~mkLit(v))) {
                cout << "FAILED: " << ~mkLit(v) << endl;
                unsatisfiable = !up.assert_lit(mkLit(v));
                fixpoint = false;
                ++failed_literal_counter;
            }

            if (unsatisfiable) {
                cout << "UNSAT by failed literals " << endl;
                fixpoint = true;
                break;
            }
        }
    } while (!fixpoint);

    if (all_defined)
        cout << "== Unique model" << endl;

    cout << "== Failed literal counter:" << failed_literal_counter << endl;
    return EXIT_SUCCESS;
}
