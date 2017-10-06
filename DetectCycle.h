//
// Created by Rohit on 21-Jun-17.
//

#ifndef CYCLEDETECTION_DETECTCYCLEROOT_H
#define CYCLEDETECTION_DETECTCYCLEROOT_H

#include <string>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include "FileIndexer.h"
#include "double_llist.h"



typedef int nodeid;

typedef std::set<nodeid> nodeSet;
typedef std::map<int, nodeSet> timeGroup;

struct approxCandidates {
    std::set<pair<int, int>> neighbours_time;
    int root_node;

    int end_time;
    bloom_filter candidates_nodes;

    bool operator==(const approxCandidates &rhs) const {
        if (root_node==rhs.root_node & end_time == rhs.end_time) {
            return true;
        } else {
            false;
        }
    }

    bool operator<(const approxCandidates &rhs) const {

        if (root_node==rhs.root_node) {

            return end_time < rhs.end_time;

        } else
            return root_node<rhs.root_node;
    }

};
struct timeBundle{
    set<int> times;
    bool operator==(const timeBundle &rhs) const {
        return times==rhs.times;

    }
    bool operator<(const timeBundle &rhs) const {
        if(times.size()==rhs.times.size()){
            return *times.end()<*rhs.times.end();
        }else{
            return times.size()>rhs.times.size();
        }
    }
    string string_format(){
        string result="(";
        int count=0;
        for(int x:times){
            if(count==0){
                count++;
                result=result+to_string(x);
            }else {
                result = result + ":" + to_string(x);
            }
        }
        result=result+")";
        return result;
    }
    int size(){
        return times.size();
    }
    int getMinTime(){
        if(size()>0){
            return *times.begin();
        }else{
            return -1;
        }
    }
    int getMaxTime(){
        if(size()>0){

            return *times.rbegin();
        }else{
            return -1;
        }
    }
};

struct edgeBundle{
    int from_node;
    int to_node;
    timeBundle time;
    bool operator==(const edgeBundle &rhs) const {
        return from_node==rhs.from_node & to_node==rhs.to_node & time==rhs.time;
    }
    bool operator<(const edgeBundle &rhs) const {

        if (from_node==rhs.from_node) {

            if(to_node==rhs.to_node){
                return time<rhs.time;
            }else{
                return to_node<rhs.to_node;
            }

        } else
            return from_node<rhs.from_node;
    }
    string printEdgeBundle(){
        return to_string(from_node)+","+to_string(to_node)+","+time.string_format();
    }
    string edgeSignature(){
        return to_string(from_node)+","+to_string(to_node);
    }
};
struct pathBundle{
    vector<edgeBundle> path;
    int getRootNode(){
        if(path.size()>0){
            return path[0].from_node;
        }else{
            return -1;
        }
    }
    edgeBundle getLastEdge(){
        if(path.size()==0){
            cout<<"error"<<endl;
        }
        return path[path.size()-1];
    }

    string printPath(){
        string result="Path Length,"+to_string(path.size())+",";
        for(edgeBundle eb:path){
            result=result+eb.printEdgeBundle()+",";
        }
        return result;
    }
    string pathSignature()const {
        string result="";
        for(edgeBundle eb:path){
            result=result+eb.edgeSignature()+",";
        }
        return result;
    }
    bool operator<(const pathBundle &rhs) const {

        if(path.size()==rhs.path.size()&path.size()>0){
            if(path[0].from_node==rhs.path[0].from_node){
                if(path[0].to_node==rhs.path[0].to_node) {
                    string lhs_sig=pathSignature();
                    string rhs_sig=rhs.pathSignature();

                    if(lhs_sig.compare(rhs_sig)!=0){
                        return lhs_sig<rhs_sig;
                    }else {
                        timeBundle rhs_t = rhs.path[0].time;
                        timeBundle lhs_t = path[0].time;
                        if (lhs_t.getMinTime() == rhs_t.getMinTime()) {

                            return lhs_t.size() > rhs_t.size();
                        } else {

                            return lhs_t.getMinTime() < rhs_t.getMinTime();
                        }
                    }
                }else{
                    return path[0].to_node < rhs.path[0].to_node;
                }
            }else{
                return path[0].from_node < rhs.path[0].from_node;
            }

        }else{
            return path.size() < rhs.path.size();
        }

    }

};



struct edge {
    int node;
    int time;

    bool operator==(const edge &rhs) const {
        if ((time == rhs.time & node == rhs.node)) {
            return true;
        } else {
            false;
        }
    }

    bool operator<(const edge &rhs) const {

        if (node == rhs.node) {

            return time < rhs.time;

        } else
            return node < rhs.node;
    }
};
struct exactCandidates {
    std::set<edge> neighbours_time; //set of nodeid,start_time
    int root_node;

    int end_time;
    set<int> candidates_nodes;

    bool operator==(const exactCandidates &rhs) const {
        if (root_node==rhs.root_node & end_time == rhs.end_time) {
            return true;
        } else {
            false;
        }
    }

    bool operator<(const exactCandidates &rhs) const {

        if (root_node==rhs.root_node) {

            return end_time < rhs.end_time;

        } else
            return root_node<rhs.root_node;
    }

};
struct tpath {
    std::vector<pedge> path;
    std::set<nodeid> seen;
    nodeid rootnode;
    long t_start;

    bool operator<(const tpath &rhs) const {
        if (rhs.t_start == t_start) {
            return rhs.path < path;
        } else
            return rhs.t_start < t_start;
    }

    bool operator==(const tpath &rhs) const {
        if (rhs.rootnode==rootnode) {
            if (rhs.t_start == t_start) {
                if (path == rhs.path) {
                    std::cout << "p" << std::endl;
                    return true;
                } else {
                    return false;
                }


            } else {
                return false;
            }
        } else {
            return false;
        }
    }
};


struct nodeSummary {
    std::map<int, std::set<nodeid>> summary;
    node *position_in_time_list=NULL;


};
int findAllCycle(std::string dataFile, std::string rootNodeFile, std::string output, int window,
                 bool isCompressed,bool reverseEdge,bool candidates_provided, bool use_bundle);
void unblock(nodeid v, int t_v, int t_e);
void findCycle(nodeid rootNode, int t_s, std::set<nodeid> *candidates, int window_bracket);

bool findTemporalPath(nodeid src, nodeid dst, int t_s, int t_end, std::vector<std::string> *path_till_here,
                      std::set<nodeid> seen, std::set<nodeid> *candidates);

std::set<nodeid> getCandidates(std::map<int, std::set<nodeid>> summary, int t_s, int t_e);

int findRootNodes(std::string input, std::string output, int window,int cleanUpLimit,bool reverseEdge);

set<int> getAllTime(std::set<pedge> E, nodeid dst);

bool allPath(nodeid w, nodeid rootnode, int t_s, int t_e, std::vector <std::string> path_till_here,
             std::set<nodeid> candidates, vector<int> *cycleLengthArray);
bool allPathWithoutCandidate(nodeid w, nodeid rootnode, int t_s, int t_e, std::vector <std::string> path_till_here,
             std::set<nodeid> seen_node, vector<int> *cycleLengthArray);

void DynamicDFS(nodeid rootnode, int t_s, int t_end, std::set<int> candidates, int window_bracket,
                bool isCompressed, bool candidates_provided, bool use_bundle,vector<int> *cycleLengthArray);
void findAllCycleNaive(std::string inputGraph,std::string resultFile,int window,bool reverseEdge);
int cleanup(std::map<nodeid,std::map<int, std::set<nodeid>>> *completeSummary,int timestamp,int window_bracket);
int cleanupAdv(int timestamp,int window_bracket,double_llist *last_updated_time_list);

int findRootNodesAdv(std::string input, std::string output, int window,int cleanUpLimit,bool reverseEdge);
void DynamicDFSApprox(approxCandidates candidate, int window_bracket);
bool allPathApprox(int w, int rootnode, int t_s, int t_e, vector<std::string> path_till_here,
              bloom_filter candidates);
int findAllCycleUsingBloom(std::string dataFile, set<approxCandidates> *root_candidates, std::string output,
                           int window, bool reverseEdge);
void DynamicDFSExact(exactCandidates candidate, int window_bracket,bool use_bundle, vector<int> *cycleLengthArray);
int findAllCycleUsingSet(std::string dataFile, set<exactCandidates> *root_candidates, std::string output,
                         int window, bool reverseEdge,bool  use_bundle);
bool allPathExact(int w, int rootnode, int t_s, int t_e, vector<std::string> path_till_here,
             set<int> candidates);


int allPathBundle(pathBundle path_bundle_till_here,  int t_e,  std::set<int> candidates, set<int> *cycleFound);
pathBundle expandPathBundle(pathBundle current_path, edgeBundle new_edge);
int pathCount(pathBundle pb);
void testCountPath();
// To compare two points
class myComparator
{
public:
    int operator() (const pair<int,int>& p1, const pair<int,int>& p2)
    {
        return p1.first > p2.first;
    }
};

bool is_overlapping(pathBundle *pathBundle1,pathBundle *pathBundle2);
#endif //CYCLEDETECTION_DETECTCYCLEROOT_H
