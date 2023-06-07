#include <cstdio>
#include <vector>
#include <string>
#include <fstream>
#include <set>
#include <strstream>
#include <sstream>

#include "xxhash64.h"

using namespace std;

typedef struct idx {
    int row;
    int col;
} idx;

typedef struct state {
    idx idx_;
    bool visited;
} state;

typedef struct globals {

    std::vector<idx> pushed_idx;
    std::vector<std::vector<state>> state_stack;

} globals;

globals gs;


int clampv(int v, int max) {
    return v < 0 ? 0 : v >= max ? max : v;
}


void retdirs(int row, int col, std::vector<idx> &out) {
    int i = clampv(row - 1, 3);
    int ii = clampv(row + 1, 3);

    int j = clampv(col - 1, 3);
    int jj = clampv(col + 1, 3);

    for (int ll = i; ll <= ii; ll++) {
        for (int kk = j; kk <= jj; kk++) {
            //printf("%d, %d \n", ll, kk);
            idx rr;
            rr.row = ll;
            rr.col = kk;
            out.push_back(rr);
        }
    }
}

std::string letters[4][4];

bool first_time = true;


std::set<uint64_t> hashed_words;
std::set<std::string> found_words;
std::set<std::string> possible_words;
std::vector<std::vector<idx>> path_letters;

void write_path(const char *file_name);

void compute_paths(int row, int col) {

    std::vector<idx> ins;
    retdirs(row, col, ins);

    std::vector<state> state_list;
    for (const idx &i: ins) {
        if (i.row == row && i.col == col) continue;
        //printf("%d %d \n", i.row, i.col);
        state s0;
        s0.idx_ = idx{i.row, i.col};
        s0.visited = false;
        state_list.push_back(s0);
    }

    gs.state_stack.push_back(state_list);
    gs.pushed_idx.push_back(idx{row, col});


    int stack_idx = 0;
    while (!gs.state_stack.empty()) {
        int not_vis_idx = 0;
        bool all_not_visited = true;
        for (const state &s: gs.state_stack[stack_idx]) {
            if (!s.visited) {
                all_not_visited = false;
                break;
            }
            not_vis_idx++;
        }
        if (all_not_visited) {
            gs.pushed_idx.pop_back();
            gs.state_stack.pop_back();
            stack_idx--;
            continue;
        }


        state &s = gs.state_stack[stack_idx][not_vis_idx];
        s.visited = true;
        gs.pushed_idx.push_back(s.idx_);

        std::vector<idx> ins0;
        retdirs(s.idx_.row, s.idx_.col, ins0);

        //push what is computed
        std::vector<state> state_list0;
        for (const idx &i: ins0) {
            if (i.row == row && i.col == col) continue;

            bool is_in = false;
            for (const auto &gid: gs.pushed_idx) {
                if (i.row == gid.row && i.col == gid.col) is_in = true;
            }
            //printf("%d %d \n", i.row, i.col);
            if (is_in) continue;


            state s0;
            s0.idx_ = idx{i.row, i.col};
            s0.visited = false;
            state_list0.push_back(s0);
        }

        if (state_list0.size() == 1) {

            // continue;
        }

        //pop if empty
        if (state_list0.empty()) {


            //add word

            std::vector<idx> vc;
            for (const auto &gid: gs.pushed_idx) {
                //printf("%c", letters[gid.row][gid.col]);
                vc.push_back(gid);
            }
            // printf("\n");

            path_letters.push_back(vc);



            // printf("\n\n");
            gs.pushed_idx.pop_back();
            gs.pushed_idx.pop_back();
            gs.state_stack.pop_back();
            stack_idx--;
            continue;
        }

        gs.state_stack.push_back(state_list0);
        stack_idx++;

    }


}


int main(int argc, char **argv) {

    //fetch letters from standard input
    if (argc < 17) {
        printf("Too few number of arguments. \n A 4x4 Wordament should contain 16 letters/ pair of letters.");
        return EXIT_FAILURE;
    } else {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                letters[i][j] = std::string(argv[i * 4 + j + 1]);
            }
        }
    }

    //load dictionary
    fstream dict_file("dict.txt");

    if (dict_file.is_open()) {
        printf("Dictionary File Opened Successfully.\n");
    } else {
        printf("Failed to Open Dictionary File.\n");
    }


    dict_file.seekg(0, std::ios_base::end);
    unsigned long long size = dict_file.tellp();

    dict_file.seekp(0);


    while (!dict_file.eof()) {
        char buff[255];
        if (dict_file.getline(&buff[0], 255, '\n')) {
            std::string str(buff);

            //compute string hash and store it to map
            uint64_t hash = XXHash64::hash(str.c_str(), str.size(), 0);
            hashed_words.insert(hash);
        }

    }
    printf("Total number of hashed dictionary: %d\n", hashed_words.size());


    dict_file.close();


    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            compute_paths(i, j);
            printf("From Cell [%d, %d]  %u Paths Found.\n", i, j, path_letters.size());

            //writing computed paths to files so as to not to recomputed
            //every time this program runs. to store computed paths in files
            //uncomment every commented line of codes and comment the above
            //two lines of code the codes down bellow


//            std::strstream fname;
//            fname << i <<"_"<< j<<".txt"<<'\0';
//
//            write_path(fname.str());
//
//            path_letters.clear();
        }
    }



//parse computed paths from file

//    for (int i = 0; i < 4; i++) {
//
//        for (int j = 0; j < 4; j++) {
//            std::strstream fname;
//            fname << i <<"_"<< j<<".txt"<<'\0';
//
//            char* file_name = fname.str();
//
//            fstream fstream1(file_name);
//
//            if(fstream1.is_open()){
//                printf("%s", file_name);
//            }else{
//                printf("file not found");
//                return EXIT_FAILURE;
//            }
//
//
//            std::string string1;
//            while(std::getline(fstream1, string1)){
//                std::stringstream stringstream1(string1);
//                std::string tmps;
//                std::vector<idx> pathid;
//                while (std::getline(stringstream1, tmps, ' ')){
//                    int rowi = stoi(tmps.substr(0, 1));
//                    int colj = stoi(tmps.substr(2, 1));
//                   // printf(" %d %d ", rowi, colj);
//                    idx idx1{rowi, colj};
//                    pathid.push_back(idx1);
//                }
//                //printf("\n");
//                path_letters.push_back(pathid);
//            }
//
//            fstream1.close();
//
//        }
//
//    }


    printf("Total Number of Paths %u\n", path_letters.size());

    for (const auto &iddx: path_letters) {
        std::string string1;
        int i = 0;
        for (const idx &id: iddx) {
            std::string p = letters[id.row][id.col];
            for (const auto &c: p)
                string1.push_back(c);

            if (i >= 2)
                possible_words.insert(string1);

            i++;
        }

    }


    printf("Comparing Hashes With %u Possible Words\n", possible_words.size());

    for (auto &ss: possible_words) {
        uint64_t hash = XXHash64::hash(ss.c_str(), ss.size(), 0);
        if (hashed_words.find(hash) != hashed_words.end()) found_words.insert(ss);
    }

    printf("\n!!!!!!!!!!!!!%u WORDS FOUND!!!!!!!!!!!!!\n", found_words.size());
    for (auto &ss: found_words) {

        printf("%s\n", ss.c_str());
    }


    return 0;
}

//void write_path(const char *file_name) {
//    ofstream out_file(file_name);
//
//    if (out_file.is_open()) printf("File successfully opened.");
//
//    for (auto &idx: path_letters) {
//        for (const auto &id: idx) {
//            out_file << id.row << "," << id.col << " ";
//
//        }
//        out_file << "\n";
//    }
//
//    out_file.close();
//}
