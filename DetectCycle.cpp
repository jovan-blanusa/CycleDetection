//
// Created by Rohit on 21-Jun-17.
//

#include "DetectCycle.h"
#include "FileIndexer.h"
#include "Split.h"
#include "Timer.h"
#include "MemoryMonitor.h"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <limits>

using namespace std;


map<nodeid, timeGroup> rootNodes;
map<nodeid, long> ct;//closing times
std::set<string> resultAllPath;
map<nodeid, set<pair<nodeid, long>>> U;//unblock list
double_llist last_updated_time_list;
map<nodeid, nodeSummary> completeSummaryAdvanced;

int findRootNodesAdv(std::string input, std::string output, int window, bool timeInMsec, int cleanUpLimit,
                     bool reverseEdge) {


    std::vector<std::string> templine;
    ifstream infile(input.c_str());
    string src, dst;
    long negativeTimestamp;
    string line;
    Platform::Timer timer;
    timer.Start();
    long timestamp;
    long temptime = 0l;
    int count = 0;
    bool cycleFound = false;
    long window_bracket = window * 60 * 60;
    double ptime = 0.0;
    string tempnode = "";
    ofstream result;
    result.open(output.c_str());
    node *position_in_time_list;
    if (timeInMsec) {
        window_bracket = window_bracket * 1000;//convert time in milli seconds
    }

    while (infile >> line) {
        try {
            templine = Tools::Split(line, ',');
            src = templine[0];
            dst = templine[1];
            if (reverseEdge) {
                src = templine[1];
                dst = templine[0];
            }
            timestamp = stol(templine[2].c_str());

            if (src.compare(dst) == 0) {
                //self loop ignored
            } else {

                //add src in the destination summary
                completeSummaryAdvanced[dst].summary[-1 * timestamp].insert(src);
                //update the position of dst node in time list
                position_in_time_list = last_updated_time_list.add_begin(timestamp, dst);
                if (completeSummaryAdvanced[dst].position_in_time_list != NULL) {
                    last_updated_time_list.delete_element(completeSummaryAdvanced[dst].position_in_time_list);
                }
                //save the position of the dst node in time list in the summary
                completeSummaryAdvanced[dst].position_in_time_list = position_in_time_list;
                //if src summary exist transfer it to dst  if it is in window prune away whats not in window
                if (completeSummaryAdvanced.count(src) > 0) {
                    for (map<long, set<nodeid>>::iterator it = completeSummaryAdvanced[src].summary.begin();
                         it != completeSummaryAdvanced[src].summary.end(); ++it) {
                        if ((-1 * it->first) > timestamp - window_bracket) {

                            if (it->second.count(dst) > 0) {
                                //the destination is already in src summary hence a cycle exist
                                set<nodeid> candidates = getCandidates(completeSummaryAdvanced[src].summary,
                                                                       -1 * it->first,
                                                                       (-1 * it->first) + window_bracket);
                                candidates.erase(dst);
                                candidates.insert(src);
                                if (candidates.size() > 1) {//only cycles having more than 1 nodes

                                    result << dst << ",";
                                    result << (-1 * it->first) << ",";//start of cycle
                                    result << timestamp << ","; //end of cycle
                                    for (string x:candidates) {
                                        result << x << ",";

                                    }
                                    result << "\n";

                                }
                                // add other in the summary
                                for (auto x:it->second) {
                                    if (x.compare(dst) != 0) {
                                        completeSummaryAdvanced[dst].summary[it->first].insert(x);
                                    }
                                }
                            } else {
                                completeSummaryAdvanced[dst].summary[it->first].insert(it->second.begin(),
                                                                                       it->second.end());
                            }
                        } else {
                            completeSummaryAdvanced[src].summary.erase(it, completeSummaryAdvanced[src].summary.end());
                            break;


                        }
                    }
                }


            }
            count++;
            if (count % cleanUpLimit == 0) {
                //do cleanup

                double parseTime = timer.LiveElapsedSeconds() - ptime;
                ptime = timer.LiveElapsedSeconds();

                std::cout << "finished parsing, count," << count << "," << parseTime << "," << getMem();
                cout << ",summary size," << completeSummaryAdvanced.size() << " delete count,";
                   //  << cleanupAdv(timestamp, window_bracket);
                std::cout << " ,clean time," << timer.LiveElapsedSeconds() - ptime << std::endl;
            }

        } catch (const std::exception &e) {
            std::cout << "Caught exception \"" << e.what() << "\"\n";
        }
    }
    std::cout << "finished parsing all " << timer.LiveElapsedSeconds()
              << std::endl;
    timer.Stop();

    result.close();
    return 0;
}

int cleanupAdv(long timestamp, long window_bracket) {

    vector<string> deletelist = last_updated_time_list.get_expired_nodes(timestamp + window_bracket + 1);
    for (auto x: deletelist) {
      //  completeSummaryAdvanced.erase(x);
    }
  //  last_updated_time_list.delete_expired(timestamp + window_bracket + 1);
    return deletelist.size();

}


int findRootNodes(std::string input, std::string output, int window, bool timeInMsec, int cleanUpLimit,
                  bool reverseEdge) {
    map<nodeid, map<long, set<nodeid>>> completeSummary;


    std::vector<std::string> templine;
    ifstream infile(input.c_str());
    string src, dst;
    long negativeTimestamp;
    string line;
    Platform::Timer timer;
    timer.Start();
    long timestamp;
    long temptime = 0l;
    int count = 0;
    bool cycleFound = false;
    long window_bracket = window * 60 * 60;
    double ptime = 0.0;
    string tempnode = "";
    ofstream result;
    result.open(output.c_str());

    if (timeInMsec) {
        window_bracket = window_bracket * 1000;//convert time in milli seconds
    }

    while (infile >> line) {
        templine = Tools::Split(line, ',');
        src = templine[0];
        dst = templine[1];
        if (reverseEdge) {
            src = templine[1];
            dst = templine[0];
        }
        timestamp = stol(templine[2].c_str());

        if (src.compare(dst) == 0) {
            //self loop ignored
        } else {

            //add src in the destination summary
            completeSummary[dst][-1 * timestamp].insert(src);

            //if src summary exist transfer it to dst  if it is in window prune away whats not in window
            if (completeSummary.count(src) > 0) {
                for (map<long, set<nodeid>>::iterator it = completeSummary[src].begin();
                     it != completeSummary[src].end(); ++it) {
                    if ((-1 * it->first) > timestamp - window_bracket) {

                        if (it->second.count(dst) > 0) {
                            //the destination is already in src summary hence a cycle exist
                            set<nodeid> candidates = getCandidates(completeSummary[src], -1 * it->first,
                                                                   (-1 * it->first) + window_bracket);
                            candidates.erase(dst);
                            candidates.insert(src);
                            if (candidates.size() > 1) {//only cycles having more than 1 nodes

                                result << dst << ",";
                                result << (-1 * it->first) << ",";//start of cycle
                                result << timestamp << ","; //end of cycle
                                for (string x:candidates) {
                                    result << x << ",";

                                }
                                result << "\n";

                            }
                            // add other in the summary
                            for (auto x:it->second) {
                                if (x.compare(dst) != 0) {
                                    completeSummary[dst][it->first].insert(x);
                                }
                            }
                        } else {
                            completeSummary[dst][it->first].insert(it->second.begin(), it->second.end());
                        }
                    } else {
                        completeSummary[src].erase(it, completeSummary[src].end());
                        break;


                    }
                }
            }


        }
        count++;
        if (count % cleanUpLimit == 0) {
            //do cleanup

            double parseTime = timer.LiveElapsedSeconds() - ptime;
            ptime = timer.LiveElapsedSeconds();
            std::cout << "finished parsing, count," << count << "," << parseTime << "," << getMem();
            cout << ",summary size," << completeSummary.size() << " delete count,"
                 << cleanup(&completeSummary, timestamp, window_bracket);
            std::cout << " ,clean time," << timer.LiveElapsedSeconds() - ptime << std::endl;
        }
    }
    std::cout << "finished parsing all " << timer.LiveElapsedSeconds()
              << std::endl;
    timer.Stop();

    result.close();
    return 0;
}

int cleanup(map<nodeid, map<long, set<nodeid>>> *completeSummary, long timestamp, long window_bracket) {
    int size = 0;
    string src = "";
    vector<string> deletelist;
    for (map<nodeid, map<long, set<nodeid>>>::iterator it = completeSummary->begin();
         it != completeSummary->end(); ++it) {

        size = it->second.size();
        src = it->first;
        if (size > 0) {
            for (map<long, set<nodeid>>::iterator itinner = it->second.begin();
                 itinner != it->second.end(); ++itinner) {
                if ((-1 * itinner->first) < timestamp - window_bracket) {
                    it->second.erase(itinner, it->second.end());
                    break;
                }

            }
        } else {

            //completeSummary->erase(it);
            deletelist.push_back(it->first);

        }

    }
    for (auto x: deletelist) {
        completeSummary->erase(x);
    }
    return deletelist.size();
}

set<nodeid> getCandidates(map<long, set<nodeid>> summary, long t_s, long t_e) {
    set<nodeid> candidates;
    long time;
    for (map<long, set<nodeid>>::iterator it = summary.begin(); it != summary.end(); ++it) {
        time = -1 * it->first;
        if (time >= t_s && time < t_e) {

            candidates.insert(it->second.begin(), it->second.end());
        }
    }
    return candidates;
}

int findAllCycle(std::string dataFile, std::string rootNodeFile, std::string output, int window, bool timeInMsec,
                 bool isCompressed, bool reverseEdge) {
    long window_bracket = window * 60 * 60;
    double ptime = 0.0;
    if (timeInMsec) {
        window_bracket = window_bracket * 1000;
    }
    ofstream cycleResult;
    cycleResult.open(output.c_str());
    string line;
    Platform::Timer timer;
    timer.Start();
    readFile(dataFile, reverseEdge);//creates a data structure of type <srcNode:<time:dstNode>>
    ptime = timer.LiveElapsedSeconds();
    std::cout << "finished reading " << ptime
              << std::endl;

    std::vector<std::string> templine;
    ifstream infile(rootNodeFile.c_str());
    string rootnode;
    long t_s;
    long t_end;
    int i = 0;

    while (infile >> line) {
        templine = Tools::Split(line, ',');
        set<string> candidateset;
        rootnode = templine[0];
        t_s = stol(templine[1].c_str());
        t_end = stol(templine[2].c_str());
        t_end = stol(templine[2].c_str());
        for (i = 3; i < templine.size(); i++) {
            candidateset.insert(templine[i]);
        }
        candidateset.insert(rootnode);
        // findCycle(rootnode, t_s, &candidateset, window_bracket);
        if (candidateset.size() > 2) {
            //run only for cycle with lenght greater than 2
            DynamicDFS(rootnode, t_s, t_end + 1, candidateset, window_bracket, isCompressed);
        }

    }
    int cycleLengthArray[50] = {0};
    int cycleLenght;
    int maxCycleLenght = 0;
    for (auto x:resultAllPath) {
        templine = Tools::Split(x, ',');
        cycleLenght = stoi(templine[1]);
        if (cycleLenght == 6) {
            cout << x << endl;
        }
        if (cycleLenght < 50) {
            cycleLengthArray[cycleLenght]++;
            if (cycleLenght > maxCycleLenght) {
                maxCycleLenght = cycleLenght;
            }
        } else
            cout << "cycle of length greather than 50 found";
        if (output.compare("") != 0) {
            cycleResult << x << "\n";
        }
        //cout << x << endl;
    }
    for (int i = 1; i <= maxCycleLenght; i++) {
        cout << i << "," << cycleLengthArray[i] << endl;
    }
}

void findCycle(std::string rootNode, long t_s, std::set<std::string> *candidates, long window_bracket) {
    set<pedge> neighbours = getFilteredData(rootNode, t_s);
    vector<std::string> initialpath;
    set<std::string> seen;
    for (set<pedge>::iterator eit = neighbours.begin(); eit != neighbours.end(); ++eit) {
        initialpath.clear();
        seen.clear();
        initialpath.push_back(rootNode + ":");
        initialpath.push_back(to_string(eit->time) + "," + eit->toVertex);
        seen.insert(eit->toVertex);
        if (findTemporalPath(eit->toVertex, rootNode, t_s, t_s + window_bracket, &initialpath, seen, candidates)) {
            std::cout << "Found cycle: " << initialpath.size() << " : ";
            for (int i = 0; i < initialpath.size(); i++) {
                std::cout << "->" << initialpath[i];
            }
            cout << endl;
        }
    }

}

bool findTemporalPath(std::string src, std::string dst, long t_s, long t_end, vector<std::string> *path_till_here,
                      set<string> seen, std::set<std::string> *candidates) {

    vector<string> path_till_here_bkp;
    for (int i = 0; i < path_till_here->size(); i++) {
        path_till_here_bkp.push_back((*path_till_here)[i]);
    }

    set<pedge> Y = getFilteredData(src, t_s, t_end, candidates);
    bool found = false;
    for (set<pedge>::iterator edgeIt = Y.begin(); edgeIt != Y.end(); ++edgeIt) {
        if (edgeIt->toVertex.compare(dst) == 0) {
            path_till_here->push_back(to_string(edgeIt->time) + "," + edgeIt->toVertex);
            return true;
        } else if (seen.count(edgeIt->toVertex) == 0) {
            seen.insert(edgeIt->toVertex);
            path_till_here->push_back(to_string(edgeIt->time) + "," + edgeIt->toVertex);
            found = findTemporalPath(edgeIt->toVertex, dst, edgeIt->time + 1, t_end, path_till_here, seen, candidates);

            if (found) {
                //   std::cout << "Found cycle: " << path_till_here->size() << " : ";
                for (int i = 0; i < path_till_here->size(); i++) {
                    //     std::cout << "->" << (*path_till_here)[i];

                }
                path_till_here->clear();
                for (int i = 0; i < path_till_here_bkp.size(); i++) {
                    path_till_here->push_back(path_till_here_bkp[i]);
                }
                cout << endl;
                found = false;
            } else {
                return found;
            }

        }
    }
    return found;

}

void unblock(nodeid v, long t_v, long t_e) {
    if (ct.count(v) > 0) {
        if (t_v > ct[v]) {
            ct[v] = t_v;
            if (U.count(v) > 0) {
                set<pair<nodeid, long>> newV;
                for (set<pair<nodeid, long>>::iterator it = U[v].begin(); it != U[v].end(); ++it) {
                    long timew = it->second;
                    if (timew < t_v) {
                        long t_max = getMaxTime(it->first, v, t_v);
                        string nodew = it->first;
                        unblock(nodew, t_max, t_e);

                        //   U[v].erase(make_pair(nodew,timew));
                        long t_min = getMinTime(nodew, v, t_v, t_e);
                        if (t_min != std::numeric_limits<long>::max()) {
                            newV.insert(std::make_pair(nodew, t_min));

                        }
                    } else {
                        newV.insert(*it);
                    }
                }
                U[v].clear();
                U[v] = newV;
            }
        }
    }
}


bool
allPath(nodeid w, nodeid rootnode, long t_s, long t_e, vector<std::string> path_till_here,
        std::set<std::string> candidates) {
    ct[w] = t_s;
    long lastp = 0;
    set<pedge> E = getFilteredData(w, t_s, t_e, &candidates);
    set<nodeid> V;
    for (auto x: E) {
        V.insert(x.toVertex);
        if (x.toVertex.compare(rootnode) == 0) {
            x.time > lastp;
            lastp = x.time;
            if (path_till_here.size() + 1 > 2) {
                //   std::cout << "Found cycle, " << path_till_here.size() + 1 << " , ";
                std::string resultline = "Found cycle," + to_string(path_till_here.size() + 1) + ",";
                for (int i = 0; i < path_till_here.size(); i++) {
                    // std::cout << "->" << (path_till_here)[i];
                    resultline = resultline + "," + (path_till_here)[i];

                }
                //  std::cout << "->" << w << "," << rootnode << "," << x.time << endl;
                resultline = resultline + "," + w + "," + rootnode + "," + to_string(x.time);
                resultAllPath.insert(resultline);
            }
        }
    }
    V.erase(rootnode);
    set<long> time_x;
    long t_min;
    for (auto x: V) {
        time_x = getAllTime(E, x);
        while (!time_x.empty()) {
            t_min = *time_x.begin();
            set<string> newcand = candidates;
            newcand.erase(x);
            vector<std::string> newpath = path_till_here;
            newpath.push_back(w + "," + x + "," + to_string(t_min));
            bool pathFound = allPath(x, rootnode, t_min + 1, t_e, newpath, newcand);
            if (ct[x] <= t_min || !pathFound) {
                time_x.clear();
                U[x].insert(make_pair(w, t_min));
            } else {
                time_x.erase(t_min);
                if (t_min > lastp) {
                    lastp = t_min;
                }
            }
        }

    }
    if (lastp > 0) {
        unblock(w, lastp, t_e);
    }
    return (lastp > 0);
}

set<long> getAllTime(set<pedge> E, nodeid dst) {
    set<long> times;
    for (auto x: E) {
        if (x.toVertex.compare(dst) == 0) {
            times.insert(x.time);
        }
    }
    return times;
}

void DynamicDFS(nodeid rootnode, long t_s, long t_end, std::set<std::string> candidates, long window_bracket,
                bool isCompressed) {
    ct.clear();
    U.clear();
    candidates.insert(rootnode);
    for (auto x:candidates) {
        ct[x] = std::numeric_limits<long>::max();
        set<pair<nodeid, long>> temp;
        U[x] = temp;
    }
    set<pedge> neighbours;
    if (isCompressed) {
        neighbours = getFilteredData(rootnode, t_s,
                                     t_end);// all the edges of type rootnode,*,t where t is between t_s and t_end
    } else {
        neighbours = getFilteredData(rootnode, t_s);// all the edges of type rootnode,*,t_s
    }

    for (auto x:neighbours) {
        if (candidates.count(x.toVertex) > 0) {
            std::set<std::string> tempcandidate = candidates;

            tempcandidate.erase(x.toVertex);
            vector<std::string> path_till_here;
            path_till_here.push_back(rootnode + "," + x.toVertex + "," + to_string(x.time));
            allPath(x.toVertex, rootnode, x.time + 1, t_end, path_till_here, tempcandidate);
        }
    }
}

void findAllCycleNaive(std::string inputGraph, std::string resultFile, long window, long timeInMsec, bool reverseEdge) {
    long window_bracket = window * 60 * 60;
    double ptime = 0.0;
    int count = 0;
    if (timeInMsec) {
        window_bracket = window_bracket * 1000;
    }
    string line;
    Platform::Timer timer;
    timer.Start();

    int cyclecount = 0;
    std::vector<std::string> templine;
    ifstream infile(inputGraph.c_str());
    string src, dst;
    long t_s;
    int i = 0;
    map<nodeid, vector<tpath>> allpaths;
    map<nodeid, vector<tpath>>::iterator pathiterator;

    ofstream result;
    result.open(resultFile.c_str());
    int selfloop = 0;
    //  map<nodeid,vector<pair<vector<pedge>,set<nodeid>>>*> pathendpointers;
    while (infile >> line) {
        templine = Tools::Split(line, ',');
        set<string> candidateset;
        src = templine[0];
        dst = templine[1];
        if (reverseEdge) {
            src = templine[1];
            dst = templine[0];
        }
        t_s = stol(templine[2].c_str());
        if (src.compare(dst) != 0) {
            pedge newedge;
            newedge.fromVertex = src;
            newedge.toVertex = dst;
            newedge.time = t_s;
            tpath newpath;
            newpath.path.push_back(newedge);
            newpath.t_start = t_s;
            newpath.rootnode = src;
            newpath.seen.insert(dst);
            newpath.seen.insert(src);
            allpaths[dst].push_back(newpath);

            //get all the paths ending with dst
            if (allpaths.count(src) > 0) {
                for (int index = 0; index < allpaths[src].size(); index++) {
                    if (t_s - allpaths[src][index].t_start > window_bracket) {
                        allpaths[src].erase(allpaths[src].begin() + index);
                        index--;
                    } else {
                        if (allpaths[src][index].rootnode.compare(dst) == 0) {//root of the path is destination
//cycle found
                            if (allpaths[src][index].path.size() > 1) {
                                result << "Found cycle, " << allpaths[src][index].path.size() + 1 << " , ";
                                cyclecount++;
                                for (int j = 0; j < allpaths[src][index].path.size(); j++) {

                                    result << allpaths[src][index].path[j].fromVertex << ","
                                           << allpaths[src][index].path[j].toVertex << ","
                                           << allpaths[src][index].path[j].time << ",";

                                }
                                //    std::cout << "->" << line << endl;
                                result << line + "\n";
                            }
                        } else if (allpaths[src][index].seen.count(dst) == 0) {// dst is not yet seen
                            tpath extendedpath;
                            extendedpath.rootnode = allpaths[src][index].rootnode;
                            extendedpath.seen = allpaths[src][index].seen;
                            extendedpath.seen.insert(dst);
                            extendedpath.t_start = allpaths[src][index].t_start;
                            extendedpath.path = allpaths[src][index].path;
                            extendedpath.path.push_back(newedge);
                            allpaths[dst].push_back(extendedpath);
                        }
                    }
                }


            }

        } else {
            //self loop
            selfloop++;
        }
        count++;
        if (count % 10000 == 0) {


            std::cout << "finished parsing, count," << count << " , " << timer.LiveElapsedSeconds() - ptime
                      << ", ";

            std::cout << allpaths.size() << " Memory, " << getMem() << " Cycle, " << cyclecount << endl;
            ptime = timer.LiveElapsedSeconds();
        }
    }
    result.close();
    std::cout << "self loop, " << selfloop << endl;
    std::cout << "finished parsing all " << timer.LiveElapsedSeconds() << " Cycle, " << cyclecount
              << std::endl;

    timer.Stop();


}