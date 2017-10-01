//
// Created by Rohit on 21-Jun-17.
//

#include "DetectCycle.h"
#include "Split.h"
#include "Timer.h"
#include "MemoryMonitor.h"
#include <time.h>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;


map<nodeid, timeGroup> rootNodes;
map<nodeid, int> ct;//closing times
std::set<string> resultAllPath;
map<nodeid, set<pair<nodeid, int>>> U;//unblock list
map<int, int> edgeCount;

int findRootNodesAdv(std::string input, std::string output, int window, int cleanUpLimit,
                     bool reverseEdge) {

    double_llist last_updated_time_list;
    map<nodeid, nodeSummary> completeSummaryAdvanced;
    std::vector<std::string> templine;
    ifstream infile(input.c_str());
    int src, dst;
    int negativeTimestamp;
    string line;
    Platform::Timer timer;
    timer.Start();
    int timestamp;
    int temptime = 0;
    int count = 0;
    bool cycleFound = false;
    int window_bracket = window * 60 * 60;
    double ptime = 0.0;
    string tempnode = "";
    ofstream result;
    result.open(output.c_str());
    node *position_in_time_list;


    while (infile >> line) {
        try {
            templine = Tools::Split(line, ',');
            src = stoi(templine[0]);
            dst = stoi(templine[1]);
            if (reverseEdge) {
                src = stoi(templine[1]);
                dst = stoi(templine[0]);
            }
            timestamp = stol(templine[2].c_str());

            if (src == dst) {
                //self loop ignored
            } else {

                //add src in the destination summary
                completeSummaryAdvanced[dst].summary[-1 * timestamp].insert(src);
                //update the position of dst node in time list
                position_in_time_list = last_updated_time_list.add_begin(timestamp, dst);
                if (completeSummaryAdvanced[dst].position_in_time_list != NULL) {
                    cout << "before delete" << endl;
                    last_updated_time_list.display_dlist();
                    last_updated_time_list.delete_element(completeSummaryAdvanced[dst].position_in_time_list);
                    cout << "after delete" << endl;
                    last_updated_time_list.display_dlist();
                }
                //save the position of the dst node in time list in the summary
                completeSummaryAdvanced[dst].position_in_time_list = position_in_time_list;
                //if src summary exist transfer it to dst  if it is in window prune away whats not in window
                if (completeSummaryAdvanced.count(src) > 0) {
                    for (map<int, set<nodeid>>::iterator it = completeSummaryAdvanced[src].summary.begin();
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
                                    for (int x:candidates) {
                                        result << x << ",";

                                    }
                                    result << "\n";

                                }
                                // add other in the summary
                                for (auto x:it->second) {
                                    if (x != dst) {
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
                cout << cleanupAdv(timestamp, window_bracket, &last_updated_time_list);
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

int cleanupAdv(int timestamp, int window_bracket, double_llist *last_updated_time_list) {

    int time = timestamp + window_bracket + 1;
    vector<int> deletelist = last_updated_time_list->get_expired_nodes(time);
    for (auto x: deletelist) {
        //  completeSummaryAdvanced.erase(x);
    }
    //  last_updated_time_list.delete_expired(timestamp + window_bracket + 1);

    return deletelist.size();

}


int findRootNodes(std::string input, std::string output, int window, int cleanUpLimit,
                  bool reverseEdge) {
    map<nodeid, map<int, set<nodeid>>> completeSummary;


    std::vector<std::string> templine;
    ifstream infile(input.c_str());
    int src, dst;
    int negativeTimestamp;
    string line;
    Platform::Timer timer;
    timer.Start();
    int timestamp;
    int temptime = 0l;
    int count = 0;
    bool cycleFound = false;
    int window_bracket = window * 60 * 60;
    double ptime = 0.0;
    string tempnode = "";
    ofstream result;
    result.open(output.c_str());


    while (infile >> line) {
        templine = Tools::Split(line, ',');
        src = stoi(templine[0]);
        dst = stoi(templine[1]);
        if (reverseEdge) {
            src = stoi(templine[1]);
            dst = stoi(templine[0]);
        }
        timestamp = stol(templine[2].substr(0, 10).c_str());

        if (src != dst) {
            //self loop ignored
        } else {

            //add src in the destination summary
            completeSummary[dst][-1 * timestamp].insert(src);

            //if src summary exist transfer it to dst  if it is in window prune away whats not in window
            if (completeSummary.count(src) > 0) {
                for (map<int, set<nodeid>>::iterator it = completeSummary[src].begin();
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
                                for (int x:candidates) {
                                    result << x << ",";

                                }
                                result << "\n";

                            }
                            // add other in the summary
                            for (auto x:it->second) {
                                if (x != dst) {
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
                }//for loop
            }


        }
        count++;
        if (count % cleanUpLimit == 0) {
            //do cleanup

            double parseTime = timer.LiveElapsedSeconds() - ptime;
            ptime = timer.LiveElapsedSeconds();
            std::cout << "finished parsing, count," << count << "," << parseTime << "," << getMem();
            cout << ",summary size," << completeSummary.size();
            cout << ",memory," << getMem();
            cout << " ,delete count," << cleanup(&completeSummary, timestamp, window_bracket);
            std::cout << " ,clean time," << timer.LiveElapsedSeconds() - ptime << std::endl;
        }
    }
    std::cout << "finished parsing all " << timer.LiveElapsedSeconds()
              << std::endl;
    timer.Stop();

    result.close();
    return 0;
}

int cleanup(map<nodeid, map<int, set<nodeid>>> *completeSummary, int timestamp, int window_bracket) {
    int size = 0;
    int src;
    vector<int> deletelist;
    for (map<nodeid, map<int, set<nodeid>>>::iterator it = completeSummary->begin();
         it != completeSummary->end(); ++it) {

        size = it->second.size();
        src = it->first;
        if (size > 0) {
            for (map<int, set<nodeid>>::iterator itinner = it->second.begin();
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

set<nodeid> getCandidates(map<int, set<nodeid>> summary, int t_s, int t_e) {
    set<nodeid> candidates;
    int time;
    for (map<int, set<nodeid>>::iterator it = summary.begin(); it != summary.end(); ++it) {
        time = -1 * it->first;
        if (time >= t_s && time < t_e) {

            candidates.insert(it->second.begin(), it->second.end());
        }
    }
    return candidates;
}

int findAllCycle(std::string dataFile, std::string rootNodeFile, std::string output, int window,
                 bool isCompressed, bool reverseEdge, bool candidates_provided) {
    int window_bracket = window * 60 * 60;
    double ptime = 0.0;

    time_t now;

    ofstream cycleResult;
    cycleResult.open(output.c_str());
    string line;
    Platform::Timer timer;
    Platform::Timer monitor;
    double monitorTime = 0.0;


    timer.Start();
    readFile(dataFile, reverseEdge);//creates a data structure of type <srcNode:<time:dstNode>>
    ptime = timer.LiveElapsedSeconds();

    std::cout << "finished reading " << ptime
              << std::endl;

    std::vector<std::string> templine;
    ifstream infile(rootNodeFile.c_str());
    nodeid rootnode;
    int t_s;
    int t_end;
    int i = 0;

    while (infile >> line) {
        templine = Tools::Split(line, ',');
        set<nodeid> candidateset;
        rootnode = stoi(templine[0]);
        t_s = stol(templine[1].c_str());
        t_end = stol(templine[2].c_str());
        for (i = 3; i < templine.size(); i++) {
            candidateset.insert(stoi(templine[i]));
        }
        candidateset.insert(rootnode);
        // findCycle(rootnode, t_s, &candidateset, window_bracket);
        if (candidateset.size() > 2) {
            //run only for cycle with lenght greater than 2
            // monitor.Start();
            //  monitorTime = time(&now);

            set<int> cyclesfound = DynamicDFS(rootnode, t_s, t_end + 1, candidateset, window_bracket, isCompressed,
                                              candidates_provided);
            /*
            cout << "Monitor Time," << monitor.LiveElapsedSeconds() << ",rootnode, ";
            cout << rootnode << ",start time," << t_s << ",candidate set size,";
            cout << candidateset.size() << ",#cycles," << cyclesfound.size();
            for (auto c:cyclesfound) {
                cout << "," << c;
            }
            cout << endl;
            */
            //  monitor.Stop();
        }


    }
    for (map<int, int>::iterator iterator1 = edgeCount.begin(); iterator1 != edgeCount.end(); iterator1++) {
        cout << iterator1->first << "," << iterator1->second << endl;
    }
    int cycleLengthArray[50] = {0};
    int cycleLenght;
    int maxCycleLenght = 0;
    for (auto x:resultAllPath) {
        templine = Tools::Split(x, ',');
        cycleLenght = stoi(templine[1]);

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
    cout << "cycles:" << endl;
    for (int i = 1; i <= maxCycleLenght; i++) {
        cout << i << "," << cycleLengthArray[i] << endl;
    }

}

int findAllCycleUsingBloom(std::string dataFile, set<approxCandidates> *root_candidates, std::string output,
                           int window, bool reverseEdge) {
    int window_bracket = window * 60 * 60;
    double ptime = 0.0;
    set<approxCandidates> &root_candidate_approx = *root_candidates;
    set<approxCandidates>::iterator root_candidate_approx_itr;
    time_t now;

    ofstream cycleResult;
    cycleResult.open(output.c_str());
    string line;
    Platform::Timer timer;
    timer.Start();
    readFile(dataFile, reverseEdge);//creates a data structure of type <srcNode:<time:dstNode>>
    ptime = timer.LiveElapsedSeconds();

    std::cout << "finished reading " << ptime << std::endl;


    int t_s;
    int t_end;
    int i = 0;
    int rootnode;
    int root_neigbour;
    bloom_filter candidateset;
    for (root_candidate_approx_itr = root_candidate_approx.begin();
         root_candidate_approx_itr != root_candidate_approx.end(); ++root_candidate_approx_itr) {
        DynamicDFSApprox(*root_candidate_approx_itr, window_bracket);

    }

    int cycleLengthArray[50] = {0};
    int cycleLenght;
    int maxCycleLenght = 0;

    std::vector<std::string> templine;
    for (auto x:resultAllPath) {
        templine = Tools::Split(x, ',');
        cycleLenght = stoi(templine[1]);

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
    cout << "cycles:" << endl;
    for (int i = 1; i <= maxCycleLenght; i++) {
        cout << i << "," << cycleLengthArray[i] << endl;
    }

}

int findAllCycleUsingSet(std::string dataFile, set<exactCandidates> *root_candidates, std::string output,
                         int window, bool reverseEdge) {
    int window_bracket = window * 60 * 60;
    double ptime = 0.0;
    set<exactCandidates> &root_candidate_approx = *root_candidates;
    set<exactCandidates>::iterator root_candidate_approx_itr;
    time_t now;

    ofstream cycleResult;
    cycleResult.open(output.c_str());
    string line;
    Platform::Timer timer;
    timer.Start();
    readFile(dataFile, reverseEdge);//creates a data structure of type <srcNode:<time:dstNode>>
    ptime = timer.LiveElapsedSeconds();

    std::cout << "finished reading " << ptime << std::endl;


    int t_s;
    int t_end;
    int i = 0;
    int rootnode;
    int root_neigbour;
    set<int> candidateset;
    for (root_candidate_approx_itr = root_candidate_approx.begin();
         root_candidate_approx_itr != root_candidate_approx.end(); ++root_candidate_approx_itr) {
        DynamicDFSExact(*root_candidate_approx_itr, window_bracket);

    }

    int cycleLengthArray[50] = {0};
    int cycleLenght;
    int maxCycleLenght = 0;

    std::vector<std::string> templine;
    for (auto x:resultAllPath) {
        templine = Tools::Split(x, ',');
        cycleLenght = stoi(templine[1]);

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
    cout << "cycles:" << endl;
    for (int i = 1; i <= maxCycleLenght; i++) {
        cout << i << "," << cycleLengthArray[i] << endl;
    }

}

void findCycle(nodeid rootNode, int t_s, std::set<nodeid> *candidates, int window_bracket) {
    set<pedge> neighbours = getFilteredData(rootNode, t_s);
    vector<std::string> initialpath;
    set<nodeid> seen;
    for (set<pedge>::iterator eit = neighbours.begin(); eit != neighbours.end(); ++eit) {
        initialpath.clear();
        seen.clear();
        initialpath.push_back(to_string(rootNode) + ":");
        initialpath.push_back(to_string(eit->time) + "," + to_string(eit->toVertex));
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

bool findTemporalPath(int src, int dst, int t_s, int t_end, vector<std::string> *path_till_here,
                      set<int> seen, std::set<int> *candidates) {

    vector<string> path_till_here_bkp;
    for (int i = 0; i < path_till_here->size(); i++) {
        path_till_here_bkp.push_back((*path_till_here)[i]);
    }

    set<pedge> Y = getFilteredData(src, t_s, t_end, candidates);
    bool found = false;
    for (set<pedge>::iterator edgeIt = Y.begin(); edgeIt != Y.end(); ++edgeIt) {
        if (edgeIt->toVertex == dst) {
            path_till_here->push_back(to_string(edgeIt->time) + "," + to_string(edgeIt->toVertex));
            return true;
        } else if (seen.count(edgeIt->toVertex) == 0) {
            seen.insert(edgeIt->toVertex);
            path_till_here->push_back(to_string(edgeIt->time) + "," + to_string(edgeIt->toVertex));
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

void unblock(nodeid v, int t_v, int t_e) {
    if (ct.count(v) > 0) {
        if (t_v > ct[v]) {
            ct[v] = t_v;
            if (U.count(v) > 0) {
                set<pair<nodeid, int>> newV;
                for (set<pair<nodeid, int>>::iterator it = U[v].begin(); it != U[v].end(); ++it) {
                    int timew = it->second;
                    if (timew < t_v) {
                        int t_max = getMaxTime(it->first, v, t_v);
                        int nodew = it->first;
                        unblock(nodew, t_max, t_e);

                        //   U[v].erase(make_pair(nodew,timew));
                        int t_min = getMinTime(nodew, v, t_v, t_e);
                        if (t_min != std::numeric_limits<int>::max()) {
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
allPath(nodeid w, nodeid rootnode, int t_s, int t_e, vector<std::string> path_till_here,
        std::set<int> candidates, set<int> *cycleFound) {
    ct[w] = t_s;
    int lastp = 0;
    set<pedge> E;
    set<nodeid> V;
    set<pedge> remove_E;
    E = getFilteredData(w, t_s + 1, t_e, &candidates);

    for (auto x: E) {
        V.insert(x.toVertex);
        if (ct.count(x.toVertex) == 0) {// if seen for the first time initialize close time and dependent list
            ct[x.toVertex] = std::numeric_limits<int>::max();
            set<pair<nodeid, int>> temp;
            U[x.toVertex] = temp;
        } else {
            if (ct[x.toVertex] <= x.time) {
                remove_E.insert(x);
            }
        }
    }
    for (auto x:remove_E) {
        E.erase(x);
    }
    edgeCount[E.size()]++;


    int cyclelenght;
    for (auto x: E) {
        if (candidates.count(x.toVertex) > 0) {
            V.insert(x.toVertex);
        }
    }
    if (V.count(rootnode) > 0) {
        for (auto x: E) {
            if (x.toVertex == rootnode) {
                if (x.time > lastp) {
                    lastp = x.time;
                }
                if (path_till_here.size() + 1 > 2) {
                    //   std::cout << "Found cycle, " << path_till_here.size() + 1 << " , ";
                    cyclelenght = path_till_here.size() + 1;
                    std::string resultline = "Found cycle," + to_string(cyclelenght) + ",";
                    cycleFound->insert(cyclelenght);
                    for (int i = 0; i < path_till_here.size(); i++) {
                        // std::cout << "->" << (path_till_here)[i];
                        resultline = resultline + "," + (path_till_here)[i];

                    }
                    //  std::cout << "->" << w << "," << rootnode << "," << x.time << endl;
                    resultline = resultline + "," + to_string(w) + "," + to_string(rootnode) + "," + to_string(x.time);
                    resultAllPath.insert(resultline);
                }
            }
        }
    }
    V.erase(rootnode);
    set<int> time_x;
    int t_min;
    for (auto x: V) {
        if (ct.count(x) == 0) {
            ct[x] = std::numeric_limits<int>::max();
            set<pair<nodeid, int>> temp;
            U[x] = temp;
        }
        time_x = getAllTime(E, x);
        while (!time_x.empty()) {
            t_min = *time_x.begin();
            set<int> newcand = candidates;
            newcand.erase(x);
            vector<std::string> newpath = path_till_here;
            newpath.push_back(to_string(w) + "," + to_string(x) + "," + to_string(t_min));
            bool pathFound = allPath(x, rootnode, t_min + 1, t_e, newpath, newcand, cycleFound);
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

bool
allPathWithoutCandidate(nodeid w, nodeid rootnode, int t_s, int t_e, vector<std::string> path_till_here,
                        std::set<int> seen_node, set<int> *cycleFound) {
    ct[w] = t_s;
    int lastp = 0;
    set<pedge> E;
    E = getFilteredData(w, t_s + 1, t_e);


    edgeCount[E.size()]++;

    set<nodeid> V;
    int cyclelenght;
    for (auto x: E) {
        if (seen_node.count(x.toVertex) == 0) {
            V.insert(x.toVertex);
        }
    }
    if (V.count(rootnode) > 0) {
        for (auto x: E) {
            if (x.toVertex == rootnode) {
                if (x.time > lastp) {
                    lastp = x.time;
                }
                if (path_till_here.size() + 1 > 2) {
                    //   std::cout << "Found cycle, " << path_till_here.size() + 1 << " , ";
                    cyclelenght = path_till_here.size() + 1;
                    std::string resultline = "Found cycle," + to_string(cyclelenght) + ",";
                    cycleFound->insert(cyclelenght);
                    for (int i = 0; i < path_till_here.size(); i++) {
                        // std::cout << "->" << (path_till_here)[i];
                        resultline = resultline + "," + (path_till_here)[i];

                    }
                    //  std::cout << "->" << w << "," << rootnode << "," << x.time << endl;
                    resultline = resultline + "," + to_string(w) + "," + to_string(rootnode) + "," + to_string(x.time);
                    resultAllPath.insert(resultline);
                }
            }
        }
    }
    V.erase(rootnode);
    set<int> time_x;
    int t_min;
    for (auto x: V) {
        if (ct.count(x) == 0) {
            ct[x] = std::numeric_limits<int>::max();
            set<pair<nodeid, int>> temp;
            U[x] = temp;
        }
        time_x = getAllTime(E, x);
        while (!time_x.empty()) {
            t_min = *time_x.begin();
            set<int> new_seen_node = seen_node;
            new_seen_node.insert(x);
            vector<std::string> newpath = path_till_here;
            newpath.push_back(to_string(w) + "," + to_string(x) + "," + to_string(t_min));
            bool pathFound = allPathWithoutCandidate(x, rootnode, t_min + 1, t_e, newpath, new_seen_node, cycleFound);
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

set<int> getAllTime(set<pedge> E, nodeid dst) {
    set<int> times;
    for (auto x: E) {
        if (x.toVertex == dst) {
            times.insert(x.time);
        }
    }
    return times;
}


set<int> DynamicDFS(nodeid rootnode, int t_s, int t_end, std::set<int> candidates, int window_bracket,
                    bool isCompressed, bool candidates_provided) {
    ct.clear();
    U.clear();
    set<int> cycleFound;

    candidates.insert(rootnode);

    for (auto x:candidates) {
        ct[x] = std::numeric_limits<int>::max();
        set<pair<nodeid, int>> temp;
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
            if (candidates_provided) {
                std::set<int> tempcandidate = candidates;

                tempcandidate.erase(x.toVertex);
                vector<std::string> path_till_here;
                path_till_here.push_back(to_string(rootnode) + "," + to_string(x.toVertex) + "," + to_string(x.time));
                allPath(x.toVertex, rootnode, x.time + 1, t_end, path_till_here, tempcandidate, &cycleFound);
            } else {
                std::set<int> seen_node;
                seen_node.insert(x.toVertex);

                vector<std::string> path_till_here;
                path_till_here.push_back(to_string(rootnode) + "," + to_string(x.toVertex) + "," + to_string(x.time));
                allPathWithoutCandidate(x.toVertex, rootnode, x.time + 1, t_end, path_till_here, seen_node,
                                        &cycleFound);
            }
        }
    }

    return cycleFound;
}


void findAllCycleNaive(std::string inputGraph, std::string resultFile, int window, bool reverseEdge) {
    int window_bracket = window * 60 * 60;
    double ptime = 0.0;
    int count = 0;

    string line;
    Platform::Timer timer;
    timer.Start();

    int cyclecount = 0;
    std::vector<std::string> templine;
    ifstream infile(inputGraph.c_str());
    int src, dst;
    int t_s;
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
        src = stoi(templine[0]);
        dst = stoi(templine[1]);
        if (reverseEdge) {
            src = stoi(templine[1]);
            dst = stoi(templine[0]);
        }
        t_s = stol(templine[2].c_str());
        if (src != dst) {
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
                        if (allpaths[src][index].rootnode == dst) {//root of the path is destination
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

void DynamicDFSApprox(approxCandidates candidate, int window_bracket) {
    ct.clear();
    U.clear();


    int root_node = candidate.root_node;
    for (auto x:candidate.neighbours_time) {
        ct.clear();
        U.clear();
        vector<std::string> path_till_here;
        path_till_here.push_back(to_string(root_node) + "," + to_string(x.first) + "," + to_string(x.second));
        allPathApprox(x.first, root_node, x.second + 1, candidate.end_time, path_till_here, candidate.candidates_nodes);
    }

}

void DynamicDFSExact(exactCandidates candidate, int window_bracket) {
    ct.clear();
    U.clear();


    int root_node = candidate.root_node;
    for (auto x:candidate.neighbours_time) {
        ct.clear();
        U.clear();
        if (candidate.root_node == 13140 & x.first == 11161 & x.second == 1196833689) {
            cout << "" << endl;
        }
        vector<std::string> path_till_here;
        path_till_here.push_back(to_string(root_node) + "," + to_string(x.first) + "," + to_string(x.second));
        allPathExact(x.first, root_node, x.second, candidate.end_time, path_till_here, candidate.candidates_nodes);

    }

}

bool
allPathApprox(int w, int rootnode, int t_s, int t_e, vector<std::string> path_till_here,
              bloom_filter candidates) {
    ct[w] = t_s;
    int lastp = 0;
    set<pedge> E;
    set<pedge> remove_E;

    E = getFilteredData(w, t_s + 1, t_e, &candidates);
    set<nodeid> V;
    int cyclelenght;
    for (auto x: E) {
        V.insert(x.toVertex);
        if (ct.count(x.toVertex) == 0) {// if seen for the first time initialize close time and dependent list
            ct[x.toVertex] = std::numeric_limits<int>::max();
            set<pair<nodeid, int>> temp;
            U[x.toVertex] = temp;
        } else {
            if (ct[x.toVertex] <= x.time) {
                remove_E.insert(x);
            }
        }
    }
    for (auto x:remove_E) {
        E.erase(x);
    }
    if (V.count(rootnode) > 0) {
        for (auto x: E) {
            if (x.toVertex == rootnode) {
                if (x.time > lastp) {
                    lastp = x.time;
                }
                if (path_till_here.size() + 1 > 2) {
                    //   std::cout << "Found cycle, " << path_till_here.size() + 1 << " , ";
                    cyclelenght = path_till_here.size() + 1;
                    std::string resultline = "Found cycle," + to_string(cyclelenght) + ",";

                    for (int i = 0; i < path_till_here.size(); i++) {
                        // std::cout << "->" << (path_till_here)[i];
                        resultline = resultline + "," + (path_till_here[i]);

                    }
                    //  std::cout << "->" << w << "," << rootnode << "," << x.time << endl;
                    resultline = resultline + "," + to_string(w) + "," + to_string(rootnode) + "," + to_string(x.time);
                    resultAllPath.insert(resultline);
                }
            }
        }
    }
    V.erase(rootnode);
    set<int> time_x;
    int t_min;
    for (auto x: V) {

        time_x = getAllTime(E, x);
        while (!time_x.empty()) {
            t_min = *time_x.begin();
            vector<std::string> newpath = path_till_here;
            newpath.push_back(to_string(w) + "," + to_string(x) + "," + to_string(t_min));
            bool pathFound = allPathApprox(x, rootnode, t_min + 1, t_e, newpath, candidates);
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

bool
allPathExact(int w, int rootnode, int t_s, int t_e, vector<std::string> path_till_here,
             set<int> candidates) {
    ct[w] = t_s;
    int lastp = 0;
    set<pedge> E;
    set<pedge> remove_E;

    E = getFilteredData(w, t_s + 1, t_e, &candidates);
    set<nodeid> V;
    int cyclelenght;
    for (auto x: E) {

        V.insert(x.toVertex);
        if (ct.count(x.toVertex) == 0) {// if seen for the first time initialize close time and dependent list
            ct[x.toVertex] = std::numeric_limits<int>::max();
            set<pair<nodeid, int>> temp;
            U[x.toVertex] = temp;
        } else {
            if (ct[x.toVertex] < x.time) {
                int temp = ct[x.toVertex];
                remove_E.insert(x);
            }
        }

    }
    for (auto x:remove_E) {
        E.erase(x);
    }
    if (V.count(rootnode) > 0) {
        for (auto x: E) {
            if (x.toVertex == rootnode) {
                if (x.time > lastp) {
                    lastp = x.time;
                }
                if (path_till_here.size() + 1 > 2) {
                    //   std::cout << "Found cycle, " << path_till_here.size() + 1 << " , ";
                    cyclelenght = path_till_here.size() + 1;
                    std::string resultline = "Found cycle," + to_string(cyclelenght) + ",";

                    for (int i = 0; i < path_till_here.size(); i++) {
                        // std::cout << "->" << (path_till_here)[i];
                        resultline = resultline + "," + (path_till_here[i]);

                    }
                    //  std::cout << "->" << w << "," << rootnode << "," << x.time << endl;
                    resultline = resultline + "," + to_string(w) + "," + to_string(rootnode) + "," + to_string(x.time);
                    resultAllPath.insert(resultline);
                }
            }
        }
    }
    V.erase(rootnode);
    set<int> time_x;
    int t_min;
    for (auto x: V) {

        time_x = getAllTime(E, x);

        while (!time_x.empty()) {
            t_min = *time_x.begin();
            vector<std::string> newpath = path_till_here;
            newpath.push_back(to_string(w) + "," + to_string(x) + "," + to_string(t_min));

            bool pathFound = allPathExact(x, rootnode, t_min + 1, t_e, newpath, candidates);
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
