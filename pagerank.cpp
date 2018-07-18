#include<iostream>
#include<math.h>
#include<memory.h>
#include<string>
#include<sstream>
#include<algorithm>
#include"pagerank.h"

int main(int argc, char* argv[]) {
    if(argc != 3) {
        cout<<"参数个数不为2"<<endl;
        return 0;
    }
    int N = 0;
    int M = 0;
    ifstream in(argv[1]);
    ofstream out_url(argv[2]);
    TriSparMatrix tsm;
    vector<string> url_list;
    //read file,init martix
    if(in) {
        memset(tsm.col_count, 0, sizeof(tsm.col_count));
        string line,x,y;
        while(getline(in, line)) {
            if(line.empty() || line == "\r") {
                break;
            }
            N++;
            stringstream ss(line);
            ss >> x;
            url_list.push_back(x);
        }
        tsm.mu = tsm.nu = N;
        while(getline(in, line)) {
            if(line.empty()) {
                break;
            }
            M++;
            Triple tri;
            stringstream ss(line);
            ss >> tri.col;
            ss >> tri.row;
            tri.e = 1;
            tsm.col_count[tri.col]++;
            tsm.data.push_back(tri);
        }
        tsm.tu = M;
    } else {
        cout<< "no such file"<<endl;
        return 0;
    }
    //calculate Markov probabilistic transfer matrix
    tsm.de = 0;
    for(int i=0;i< M;i++) {
        tsm.data[i].e = 0.85 * (tsm.data[i].e * 1.0 / tsm.col_count[tsm.data[i].col]);
    }
    //Power iteration
    float precision = 0.00001;
    vector<float>* x = new vector<float>(N,1/N);
    vector<float>* r = new vector<float>(N,1/N);
    do {
        delete(x);
        x = r;
        r = new vector<float>(N,0);
        multiply(tsm, x, r);
        for(int i=0;i<N;i++) {
            (*r)[i] += 0.15/N;
        }

    }while(getDValue(*x, *r) >= precision);
    //init sort struct
    vector<Rank> queue;
    for(int i=0;i<r->size();i++) {
        Rank rank;
        rank.id = i;
        rank.rank_index = (*r)[i];
        queue.push_back(rank);
    }
    sort(&queue[0],&queue[N-1] + 1,compare);
    //output
    for(int i=0;i< 10;i++) {
        cout << url_list[queue[i].id] << " " << queue[i].rank_index << endl;
        out_url << url_list[queue[i].id] << " " << queue[i].rank_index << endl; 
    }
    ofstream out_matrix("matrix.txt"); 
    if(out_matrix) {
        storageMatrix(tsm, out_matrix);
    } else {
        cout << "矩阵文件不存在" << endl;
    }
    delete(x);
    delete(r);
    return 0;
}

float getDValue(vector<float> x, vector<float> r) {
    int size = x.size();
    float sum_r = 0;
    float sum_x = 0;
    for(int i=0;i<size;i++) {
        sum_r += r[i];
        sum_x += x[i];
    }
    return fabs(sum_r - sum_x);
}

void multiply(TriSparMatrix tsm, vector<float>* x,vector<float>* r) {
    int tu = tsm.data.size(); 
    int N = x->size();
    float sum = 0;
    for(int i=0;i<N;i++) {
        sum += x->at(i);
    }
    vector<float> rest(N, sum);
    for(int i=0;i<tu;i++) {
        Triple tri = tsm.data[i];
        int row = tri.row;
        int col = tri.col;
        (*r)[row - 1] += tsm.data[i].e * (*x)[col - 1]; 
        rest[row - 1] -= (*x)[col - 1];
    }
    for(int i=0;i<N;i++) {
        (*r)[i] += tsm.de * rest[i];
    }
} 

bool compare(Rank a, Rank b) {
    return a.rank_index > b.rank_index;
}

void storageMatrix(TriSparMatrix tsm, ofstream &out) {
    vector<Triple> data = tsm.data;
    int size = data.size();
    vector<int> rows;
    vector<int> cols;
    vector<float> values;
    for(int i=0;i<size;i++) {
        Triple tri = data[i];
        rows.push_back(tri.row);
        cols.push_back(tri.col);
        values.push_back(tri.e);
    }
    out << "row_count:" << tsm.mu << " " << "col_count" << tsm.nu <<endl;
    out << "row indices: ";
    for(int i=0;i<size;i++) {
        out << rows[i] << " ";
    }
    out << endl;
    out << "col indices: ";
    for(int i=0;i<size;i++) {
        out << cols[i] << " ";
    }
    out << endl;
    out << "values: ";
    for(int i=0;i<size;i++) {
        out << values[i] << " ";
    }
    out << endl;
}
