/*
 * File:  main.cpp
 * Author:  mikolas
 * Created on:  Sat Mar 28 13:58:57 WET 2020
 * Copyright (C) 2020, Mikolas Janota
 */

#include "unit.h"
#include "minisat/utils/System.h"
#include "minisat/utils/ParseUtils.h"
#include "minisat/utils/Options.h"
#include "minisat/core/Dimacs.h"
#include "minisat/core/Solver.h"
#include <stdlib.h>
#include <string>
#include <iostream>
#include <unordered_map>
#include "auxiliary.h"
#include "ReadCNF.h"
using  namespace  std;
int run_cnf(const string& flafile);

int main(int argc, char** argv) {
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
        exit(1);
    }
    cout<<"c done reading: "<<read_cpu_time()<<std::endl;
    if (!reader.get_header_read()) {
        cerr << "ERROR: Missing header." << endl;
        cerr << "ABORTING" << endl;
        exit(1);
    }



    return 0;
}

