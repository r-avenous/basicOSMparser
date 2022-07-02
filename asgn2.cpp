/*
20CS10012
Atulya Sharma
Assignment 2
*/

#include<bits/stdc++.h>
#include<cmath>
#include"rapidxml.hpp"
#include"rapidxml_utils.hpp"
#include"rapidxml_print.hpp"

using namespace rapidxml;

#define out std::cout
#define in std::cin
#define vec std::vector
#define stll std::stoll
#define umap std::unordered_map

inline bool equalChar(char a, char b) {if(a == b or abs(a-b) == 'a' - 'A') return true; return false;}

xml_node<>* g_utilNode;

double haversineDistance(xml_node<> *n1, xml_node<> *n2)
{
    double lon1 = atof(n1->last_attribute()->value()), lon2 = atof(n2->last_attribute()->value()), 
    lat1 = atof(n1->last_attribute()->previous_attribute()->value()), lat2 = atof(n2->last_attribute()->previous_attribute()->value());
    
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;
    lat1 = (lat1) * M_PI / 180.0;
    lat2 = (lat2) * M_PI / 180.0;
    double a = pow(sin(dLat / 2), 2) + pow(sin(dLon / 2), 2) * cos(lat1) * cos(lat2);
    double rad = 6371;
    double c = 2 * asin(sqrt(a));
    return rad * c;
}

bool comp(xml_node<>* n1, xml_node<>* n2) {return haversineDistance(n1, g_utilNode) > haversineDistance(n2, g_utilNode);}

bool isSubstring(char *strng, char *toSearch)
{
    for(int i=0; strng[i] != '\0'; i++)
    {
        if(equalChar(strng[i], toSearch[0]))
        {
            for(int j=1; toSearch[j] != '\0' and strng[i+j] != '\0' and equalChar(strng[i+j], toSearch[j]); j++)
            {
                if(toSearch[j+1] == '\0') return true;
            }
        }
    }
    return false;
}

void setUpOSM(xml_node<> *og, int &nodes, int &ways, vec<xml_node<>*> &vect, umap<long long int, xml_node<>*> &nodeIDreference, umap<xml_node<>*, vec<xml_node<>*>> &adjList)
{
    nodes = 0;
    ways = 0;
    for (xml_node<>* x = og->first_node("node"); x != nullptr; x = x->next_sibling("node"))
    {
        vect.push_back(x);
        nodeIDreference.insert(std::pair<long long int, xml_node<>*>(stll(x->first_attribute()->value()), x));
        adjList.insert(std::pair<xml_node<>*, vec<xml_node<>*>>(x, vec<xml_node<>*>()));
        nodes++;
    }
    for(xml_node<>* x = og->first_node("way"); x != nullptr; x = x->next_sibling("way"))
    {
        ways++;
        for(xml_node<>* ch = x->first_node("nd")->next_sibling("nd"); ch != nullptr; ch = ch->next_sibling("nd"))
        {
            adjList[nodeIDreference[stll(ch->first_attribute()->value())]].push_back(nodeIDreference[stll(ch->previous_sibling("nd")->first_attribute()->value())]);
            adjList[nodeIDreference[stll(ch->previous_sibling("nd")->first_attribute()->value())]].push_back(nodeIDreference[stll(ch->first_attribute()->value())]);
        }
    }
}

vec<xml_node<>*> searchNodesWithName(xml_node<> *x, char *keyword)
{
    vec<xml_node<>*> res;
    for (x = x->first_node("node"); x != nullptr; x = x->next_sibling("node"))
    {
        for(xml_node<> * i = x->first_node(); i != nullptr; i = i->next_sibling())
        {
            if(strcmp(i->first_attribute()->value(), "name") == 0)
            {
                if(isSubstring(i->first_attribute()->next_attribute()->value(), keyword)) res.push_back(i->parent());
                break;
            }
        }
    }
    return res;
}

void printNodeInfo(xml_node<>* n)
{
    out << n->first_attribute()->value() << "           ";
    for(xml_node<> * i = n->first_node(); i != nullptr; i = i->next_sibling())
    {
        if(strcmp(i->first_attribute()->value(), "name") == 0)
        {
            out << i->first_attribute()->next_attribute()->value() << "\n";
            return;
        }
    }
    out << "UNNAMED\n";
}

vec<xml_node<>*> kNearestNodes(xml_node<>* target, vec<xml_node<>*> vect, int k)
{
    g_utilNode = target;
    std::make_heap(vect.begin(), vect.end(), comp);
    vec<xml_node<>*> res;
    while(k--)
    {
        std::pop_heap(vect.begin(), vect.end(), comp);
        vect.pop_back();
        res.push_back(vect.front());
    }
    return res;
}

void printPathNode(xml_node<>* src, xml_node<>* x, umap<xml_node<>*, xml_node<>*> parent)
{
    if(parent[x] == src) return;
    printPathNode(src, parent[x], parent);
    printNodeInfo(x);
}

double shortestDistance(vec<xml_node<>*> nodes, xml_node<>* source, xml_node<>* desti, umap<xml_node<>*, vec<xml_node<>*>> adjList)
{
    std::unordered_set<xml_node<>*> included;
    std::priority_queue<std::pair<double, xml_node<>*>> pq;
    umap<xml_node<>*, double> distance;
    umap<xml_node<>*, xml_node<>*> parent;
    for(xml_node<>* i: nodes)
    {
        distance.insert(std::pair<xml_node<>*, double>(i, 50000));
        parent.insert(std::pair<xml_node<>*, xml_node<>*>(i, nullptr));
    }
    parent[source] = source;
    distance[source] = 0;
    pq.push(std::pair<double, xml_node<>*>(0, source));
    while(!pq.empty())
    {
        xml_node<>* u = pq.top().second;
        pq.pop();
        included.insert(u);
        for(xml_node<>* v: adjList[u])
        {
            if(distance[u] + haversineDistance(u, v) < distance[v] && included.find(v) == included.end())
            {
                distance[v] = distance[u] + haversineDistance(u, v);
                pq.push(std::pair<double, xml_node<>*>(distance[v], v));
                parent[v] = u;
            }
        }
    }
    out << "       ID       |     Name\n";
    if(distance[desti] < 40000) printPathNode(source, desti, parent);
    return distance[desti];
}

int main()
{
    file<> file("map.osm");
    xml_document<> doc;
    doc.parse<0>(file.data());

    int noOfNodes, noOfWays;
    umap<long long int, xml_node<>*> nodeIDreference;
    xml_node<>* x = doc.first_node();
    vec<xml_node<>*> nodes;
    umap<xml_node<>*, vec<xml_node<>*>> adjList;
    setUpOSM(x, noOfNodes, noOfWays, nodes, nodeIDreference, adjList);
    
    out << "OSM file has been setup.\nNumber of nodes are: " << noOfNodes << "\nNumber of ways are: " << noOfWays << "\n";
    char command[100];
    while(1)
    {
        out << "Enter 'n' to use name as input, 'i' to use ID as input, and anything else to quit the application. ";
        in >> command;
        if(strcmp(command, "n") == 0)
        {
            out << "Enter a name to get the matching node results. ";
            in >> command;
            vec<xml_node<>*> searchRes = searchNodesWithName(x, command);
            out << "S.No. |      ID      |     Name\n";
            for(int i=0; i<searchRes.size(); i++)
            {
                out << i+1 << "       ";
                printNodeInfo(searchRes[i]);
            }
            out << "Enter Serial Number of the node of choice: ";
            int sno;
            in >> sno;
            out << "Enter 'k' to search for nearest nodes any anything else to get shortest distance from another node. ";
            in >> command;
            if(strcmp(command, "k"))
            {
                out << "Enter a name to get the matching node results for the destination.\n";
                in >> command;
                vec<xml_node<>*> searchRes2 = searchNodesWithName(x, command);
                out << "S.No.   |        ID         |     Name\n";
                for(int i=0; i<searchRes2.size(); i++)
                {
                    out << i+1 << "       ";
                    printNodeInfo(searchRes2[i]);
                }
                out << "Enter Serial Number of the node of choice: ";
                int snoDestination;
                in >> snoDestination;
                double dist = shortestDistance(nodes, searchRes[sno-1], searchRes2[snoDestination-1], adjList);
                if(dist < 40000) out << "The shortest distance between these 2 is: " << dist << " KM\n";
                else out << "Unreachable\n";
            }
            else
            {
                int k;
                out << "Enter the number of nearest nodes. ";
                in >> k;
                vec<xml_node<>*> searchRes3 = kNearestNodes(searchRes[sno], nodes, k);
                for(xml_node<>* i: searchRes3) printNodeInfo(i);
            }
        }
        else if(strcmp(command, "i") == 0)
        {
            out << "Enter an ID. ";
            in >> command;
            xml_node<>* src = nodeIDreference[stll(command)];
            out << "Enter 'k' to search for nearest nodes any anything else to get shortest distance from another node. ";
            in >> command;
            if(strcmp(command, "k"))
            {
                out << "Enter the ID of destination.\n";
                in >> command;
                xml_node<>* dest = nodeIDreference[stll(command)];
                double dist = shortestDistance(nodes, src, dest, adjList);
                if(dist < 40000) out << "The shortest distance between these 2 is: " << dist << " KM\n";
                else out << "Unreachable\n";
            }
            else
            {
                int k;
                out << "Enter the number of nearest nodes. ";
                in >> k;
                vec<xml_node<>*> searchRes3 = kNearestNodes(src, nodes, k);
                for(xml_node<>* i: searchRes3) printNodeInfo(i);
            }
        }
        else break;
    }
    return 0;
}