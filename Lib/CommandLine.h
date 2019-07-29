#ifndef _COMMANDLINE_H_
#define _COMMANDLINE_H_

#include <unistd.h>

#include <map>
#include <utility>
#include <iostream>
#include <functional>
#include <algorithm>

class CommandLine {
  private:
    struct Entry {
        std::string info;
        std::string value;

        bool isSet;
    };

    struct pString {
        std::string s;
        pString(const std::string &s) : s(s) {};

        void assertNUM() {
            if (std::any_of(s.begin(), s.end(),
                            [](const char c) { return !std::isdigit(c); })) {
                std::cerr << s << " is not a number!\n";
                exit(0);
            }
        }

        operator int() {
            assertNUM();
            return std::stoi(s);
        }

        operator uint64_t() {
            assertNUM();
            return std::stoull(s);
        }

        operator std::string() { return s; }

        friend inline std::ostream &operator<<(std::ostream &os,
                                               const pString &ps) {
            os << ps.s;
            return os;
        }
    };

    int argc;
    char **argv;
    bool all;
    std::string usage;

    std::map<char, Entry> vs;

  public:
    CommandLine(int argc, char *argv[], bool all = true)
        : argc(argc), argv(argv), all(all) {}

    void addCommand(char c, const std::string &info) {
        usage.append(std::string("   -") + c + "\t\t");
        usage.append(info);
        usage.push_back('\n');

        vs[c] = Entry{ info, "", false };
    };

    void addCommand(char c, const std::string &info, const std::string &v) {

        usage.append(std::string("   -") + c + "\t\t");
        usage.append(info);
        usage.append(std::string(" [default: ") + v + "]\n");

        vs[c] = Entry{ info, v, true };
    }

    void parseCommand() {

        std::string s("");
        for (auto &i : vs) {
            s.append(1, i.first);
            s.append(":");
        }

        char c;
        while ((c = getopt(argc, argv, s.c_str())) != -1) {
            auto iter = vs.find(c);
            if (iter != vs.end()) {
                iter->second.value = optarg;
                iter->second.isSet = true;
            } else {
                std::cerr << usage;
                exit(0);
            }
        }

        if (all && std::any_of(vs.begin(), vs.end(),
                               [](const std::pair<char, Entry> &e) {
                       return !e.second.isSet;
                   })) {
            std::cerr << usage;
            exit(0);
        }
    }

    pString operator[](char i) {
        auto it = vs.find(i);
        if (it == vs.cend()) {
            std::cerr << "there no valid arg '" << i << "'\n";
            exit(0);
        } else {
            return pString(it->second.value);
        }
    }

    void printAll() {
        for (auto &t : vs) {
            std::cout << t.first << " : " << t.second.value << std::endl;
        }
    }
};

#endif
