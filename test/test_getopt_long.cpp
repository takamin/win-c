#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <string.h>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)

#include "getopt.h"
extern "C" int postpone_count;
extern "C" int nextchar;

#else

#include <getopt.h>

#endif


using namespace std;

class Arg {
public:
    Arg(char const* command_line) : argc(0), argv(0) {
        cout << "*** " << command_line << endl;
        vector<string> elements;
        this->argc = Arg::split(command_line, elements);
        if(this->argc == 0) {
            return;
        }
        this->argv = new char*[argc];
        int i = 0;
        for(int i = 0; i < this->argc; i++) {
            string s = elements[i];
            this->argv[i] = new char[s.size() + 1];
            strcpy(this->argv[i], s.c_str());
        }
    }
    virtual ~Arg() {
        for(int i = 0; i < this->argc; i++) {
            delete this->argv[i];
        }
        delete [] this->argv;
    }
    int count() const { return this->argc; }
    char** values() { return this->argv; }
private:
    int argc;
    char** argv;
private:
    static int split(char const* in, vector<string>& argv);
};
int Arg::split(char const* in, vector<string>& argv)
{
    int argc = 0;
    bool indq = false;
    bool insq = false;
    bool inword = false;
    stringstream ss;

    argv.clear();
    while(*in != '\0') {
        if(!inword) {
            if(*in == '"') {
                ++in;
                inword = indq = true;
            } else
            if(*in == '\'') {
                ++in;
                inword = insq = true;
            } else
            if(*in != ' ') {
                if(*in == '\\') {
                    ++in;
                }
                inword = true;
            } else {
                while(*in == ' ') {
                    ++in;
                }
            }
        } else {
            if(indq) {
                if(*in == '"') {
                    ++in;
                    indq = false;
                } else {
                    if(*in == '\\') {
                        ++in;
                    }
                    ss << (char)*in;
                    ++in;
                }
            } else
            if(insq) {
                if(*in == '\'') {
                    ++in;
                    insq = false;
                } else {
                    if(*in == '\\') {
                        ++in;
                    }
                    ss << (char)*in;
                    ++in;
                }
            } else
            if(*in != ' ') {
                if(*in == '\\') {
                    ++in;
                }
                ss << (char)*in;
                ++in;
            } else {
                inword = false;
                argv.push_back(ss.str());
                ss.str("");
                argc++;
            }
        }
    }
    if(ss.str() != "") {
        argv.push_back(ss.str());
        ss.str("");
        argc++;
    }
    return argc;
}
int pass_count = 0;
int fail_count = 0;
static void test_equal(char const* name, char const* A, char const* B) {
    if(A == B || A != 0 && B != 0 && string(A) == string(B)) {
        pass_count++;
        cout << "PASS ( " << name << " )" << endl;
    } else {
        fail_count++;
        cout << "FAIL ( " << name << " )" << endl;
    }
}
static void test_equal(char const* name, int A, int B) {
    if(A == B) {
        pass_count++;
        cout << "PASS ( " << name << " )" << A << "==" << B << endl;
    } else {
        fail_count++;
        cout << "FAIL ( " << name << " )" << A << "!=" << B << endl;
    }
}
#define TEST_EQUAL(A,B) test_equal("" #A " == " #B, A, B)
int main(int argc, char* argv[]) {
    struct option opt01[] = {
        {"aaa", no_argument, NULL, 'a', },
        {"ddd", required_argument, NULL, 'd', },
        {"bbb", no_argument, NULL, 'b', },
        {"fff", required_argument, NULL, 'f', },
        {"ccc", no_argument, NULL, 'c', },
        {"eee", required_argument, NULL, 'e', },
        { NULL, 0, NULL, 0, },
    };
    int longindex = 0;
    int opt = 0;
    Arg test01arg("a.out --aa --bbb --c");
    opt = getopt_long(test01arg.count(), test01arg.values(), "abcdef", opt01, &longindex);
    TEST_EQUAL(opt, 'a');
    TEST_EQUAL(longindex, 0);
    TEST_EQUAL(optarg, 0);
    opt = getopt_long(test01arg.count(), test01arg.values(), "abcdef", opt01, &longindex);
    TEST_EQUAL(opt, 'b');
    TEST_EQUAL(longindex, 2);
    TEST_EQUAL(optarg, 0);
    opt = getopt_long(test01arg.count(), test01arg.values(), "abcdef", opt01, &longindex);
    TEST_EQUAL(opt, 'c');
    TEST_EQUAL(longindex, 4);
    TEST_EQUAL(optarg, 0);

    optarg = 0;
    optind = 1;
    opterr = 1;
    optopt = 0;
    Arg test02arg("a.out --ee --aa --f=XXX --f= XYZ --bbb --ddd =YYY --ddd = ZZZ");
    opt = getopt_long(test02arg.count(), test02arg.values(), "abcdef", opt01, &longindex);
    TEST_EQUAL(opt, 'e');
    TEST_EQUAL(longindex, 5);
    TEST_EQUAL(optarg, "--aa");
    TEST_EQUAL(test02arg.values()[optind], "--f=XXX");
    opt = getopt_long(test02arg.count(), test02arg.values(), "abcdef", opt01, &longindex);
    TEST_EQUAL(opt, 'f');
    TEST_EQUAL(longindex, 3);
    TEST_EQUAL(optarg, "XXX");
    TEST_EQUAL(test02arg.values()[optind], "--f=");
    opt = getopt_long(test02arg.count(), test02arg.values(), "abcdef", opt01, &longindex);
    TEST_EQUAL(opt, 'f');
    TEST_EQUAL(longindex, 3);
    TEST_EQUAL(optarg, "");
    TEST_EQUAL(test02arg.values()[optind], "XYZ");
    opt = getopt_long(test02arg.count(), test02arg.values(), "abcdef", opt01, &longindex);
    TEST_EQUAL(opt, 'b');
    TEST_EQUAL(longindex, 2);
    TEST_EQUAL(optarg, 0);
    TEST_EQUAL(test02arg.values()[optind], "--ddd");
    opt = getopt_long(test02arg.count(), test02arg.values(), "abcdef", opt01, &longindex);
    TEST_EQUAL(opt, 'd');
    TEST_EQUAL(longindex, 1);
    TEST_EQUAL(optarg, "=YYY");
    TEST_EQUAL(test02arg.values()[optind], "--ddd");
    opt = getopt_long(test02arg.count(), test02arg.values(), "abcdef", opt01, &longindex);
    TEST_EQUAL(opt, 'd');
    TEST_EQUAL(longindex, 1);
    TEST_EQUAL(optarg, "=");
    TEST_EQUAL(test02arg.values()[optind], "ZZZ");

    printf("TEST RESULT: PASS %d(%.2f%%), FAIL %d(%.2f%%)\n",
         pass_count, 100.0 * pass_count/(pass_count + fail_count),
         fail_count, 100.0 * fail_count/(pass_count + fail_count));
	return 0;
}
