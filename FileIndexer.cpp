//
// Created by Rohit on 22-Jun-17.
//

#include "FileIndexer.h"
#include <iostream>
#include <map>
#include <utility>
#include <set>
#include <sstream>
#include <fstream>
#include <vector>
#include "Split.h"

//struct pedge;



using namespace std;
map<string, map<long, set<string>>> sorteddata;

int readFile(std::string inputFile) {

    ifstream infile(inputFile.c_str());
    string line;
    std::vector<std::string> templine;
    long timestamp;
    string src, dst;
    while (infile >> line) {
        templine = Tools::Split(line, ',');
        src = templine[0];
        dst = templine[1];
        timestamp = stol(templine[2].c_str());
        sorteddata[src][timestamp].insert(dst);
    }
    return 0;
}

std::set<pedge> getFilteredData(string src, long t_s, long t_end, set<string> *candidates) {
    std::set<pedge> result;
    set<string>::iterator xit;

    if (sorteddata.count(src) > 0) {
        std::map<long, set<string>> m = sorteddata[src];
        for (std::map<long, set<string>>::iterator low = m.lower_bound(t_s - 1); low != m.end(); ++low) {
            long t = low->first;
            if (low->first > t_end) {
                break;
            }
            //std::cout << low->first << ' ' << low->second << std::endl;
            for (xit = low->second.begin(); xit != low->second.end(); ++xit) {
                string node=*xit;
                if (candidates->count(*xit) > 0) {
                    pedge edge1;
                    edge1.fromVertex = src;
                    edge1.toVertex = *xit;
                    edge1.time = low->first;
                    result.insert(edge1);
                }
            }
        }

    }
    return result;
}

std::set<pedge> getFilteredData(string src, long t_s) {
    std::set<pedge> result;

    if (sorteddata.count(src) > 0) {
        set<string> m = sorteddata[src][t_s];
        for (auto x:m) {
            pedge edge1;
            edge1.fromVertex = src;
            edge1.toVertex = x;
            edge1.time = t_s;
            result.insert(edge1);
        }

    }
    return result;
}

