#ifndef PAGERANK_H
#define PAGERANK_H

#include<vector>
#include<fstream>

using namespace std;

#define MAX_URL_NUM 170000

struct Triple {
    int row;
    int col;
    float e;
};

struct TriSparMatrix {
    int mu,nu,tu;
    float de;
    int col_count[MAX_URL_NUM+1];
    vector<Triple> data;
};

struct Rank {
    int id;
    float rank_index;
};

void multiply(TriSparMatrix tsm, vector<float>* x, vector<float>* r); 

float getDValue(vector<float> x, vector<float> r);

bool compare(Rank a, Rank b);

void storageMatrix(TriSparMatrix tsm, ofstream &out);


#endif
