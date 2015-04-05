#include <iostream>
#include <string.h>
#include "getopt.h"
using namespace std;
int main(int argc, char* argv[]) {
    if(argc <= 1) {
        cerr << "no optstring" << endl;
        return -1;
    }
    char* const optstring = argv[1];
    --argc;
    for(int i = 1; i < argc; i++) {
        argv[i] = argv[i + 1];
    }
    cout << "optstring=" << optstring << endl;
    for(int i = 0; i < argc; i++) {
        cout << "argv[" << i << "]=" << argv[i] << endl;
    }

    int count = 0;
    int opt = 0;
    do {
        cout << "getopt[" << count << "] : ";
        count++;
        opt = getopt(argc, argv, optstring); 
        if(opt < 0) {
            cout << "opt=" << "(END OF GETOPT)";
        } else {
            cout << "opt=" << "'" << (char)opt << "'";
            if(opt != '?' && opt != ':') {
                char* optptr = strchr(optstring, opt);
                if(optptr != 0 && *(optptr+1) == ':') {
                    cout << " optarg=\"" << optarg << "\"";
                }
            }
        }
        cout << endl;
    } while(opt >= 0);
    cout << "non option parameter ---- " << endl;
    for(int i = 0; optind + i < argc; i++) {
        cout << "argv[optind+" << i << "]=" << argv[optind + i];
    }
}
