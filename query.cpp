#include <string>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

int main(int nargs, const char* args[]){
    //Must use as `./query <guid> <cutoff>`
    if(nargs != 3){
        cout << "Incorrect usage. " << endl;
        return 1;
    }

    string guid = args[1];
    int cutoff = stoi(args[2]);
    
    char ch;
    vector<string> line;
    string acc;
    fstream fin("outputs/all.txt", fstream::in);
    int i = 0;
    while (fin >> noskipws >> ch) {
        i++;
        if(ch == ' '){
            line.push_back(acc);
            acc = "";
            continue;
        }
        else if(ch == '\n'){
            //We've parsed a line, so convert and compare
            if(line.size() != 2){
                cout << "??? Line " << i << ": " << line.size() << endl;
                continue;
            }
            string guid1 = line.at(0);
            string guid2 = line.at(1);
            int snp = stoi(acc);
            if(guid == guid1 && snp <= cutoff){
                cout << guid1 << ": " << snp << endl;
            }
            if(guid == guid2 && snp <= cutoff){
                cout << guid2 << ": " << snp << endl;
            }

            line = {};
            acc = "";

        }
        else{
            acc += ch;
        }
    }
    fin.close();
}