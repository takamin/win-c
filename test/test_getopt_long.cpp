#include <iostream>
#include <string>
#include <string.h>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

using namespace std;

class Arg {
public:
    Arg(char const* command_line) : argc(0), argv(0) {
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
            }
        }
    }
    return argc;
}
static void test_equal(char const* name, int A, int B, bool condition) {
    if(condition) {
        cout << "PASS ( " << name << " )" << A << "==" << B << endl;
    } else {
        cout << "FAIL ( " << name << " )" << A << "!=" << B << endl;
    }
}
#define TEST_EQUAL(A,B) test_equal("" #A " == " #B, A, B, A==B)
int main(int argc, char* argv[]) {
    vector<string> elements;
    Arg test_arg("A\\'B\\'C \"D\\\"E\\\"F\" 'G\\H\\I' ");

    struct option opt01[] = {
        {"aaa", no_argument, NULL, 'a', },
        {"bbb", no_argument, NULL, 'b', },
        {"ccc", no_argument, NULL, 'c', },
        { NULL, 0, NULL, 0, },
    };
    int longindex = 0;
    int opt = 0;
    Arg test01arg("a.out --aaa --bbb --ccc");
    opt = getopt_long(test01arg.count(), test01arg.values(), "abc", opt01, &longindex);
    TEST_EQUAL(opt, 'a');
    TEST_EQUAL(longindex, 0);
    opt = getopt_long(test01arg.count(), test01arg.values(), "abc", opt01, &longindex);
    TEST_EQUAL(opt, 'b');
    TEST_EQUAL(longindex, 1);
    opt = getopt_long(test01arg.count(), test01arg.values(), "abc", opt01, &longindex);
    TEST_EQUAL(opt, 'c');
    TEST_EQUAL(longindex, 2);
    return 0;
}
