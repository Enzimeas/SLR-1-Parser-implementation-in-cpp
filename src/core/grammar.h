#ifndef GRAMMAR_HPP
#define GRAMMAR_HPP

#include <iostream>
#include <vector>
#include <cstring>
#include "../models/State.hpp"

using namespace std;

class Grammar {
public:
    char terminals[100];
    int no_t;
    char non_terminals[100];
    int no_nt;
    char follow[20][20];
    char first[20][20];
    
    State original_grammar;
    State augmented_grammar;
    State dotted_grammar;

    Grammar() {
        no_t = 0; no_nt = 0;
        memset(terminals, 0, sizeof(terminals));
        memset(non_terminals, 0, sizeof(non_terminals));
        memset(follow, 0, sizeof(follow));
        memset(first, 0, sizeof(first));
    }

    bool is_non_terminal(char a) {
        return (a >= 'A' && a <= 'Z');
    }

    int return_index(char a) {
        for(int i=0; i < no_t; i++) if(terminals[i] == a) return i;
        for(int i=0; i < no_nt; i++) if(non_terminals[i] == a) return no_t + i;
        return -1;
    }

    void readInput() {
        State *init = &original_grammar;
        cout << "Insira o numero de producoes:\n"; cin >> init->prod_count;
        cout << "Insira o numero de nao terminais:\n"; cin >> no_nt;
        cout << "Insira os nao terminais:\n";
        for(int i=0; i < no_nt; i++) cin >> non_terminals[i];
        cout << "Insira o numero de terminais:\n"; cin >> no_t;
        cout << "Insira os terminais:\n";
        for(int i=0; i < no_t; i++) cin >> terminals[i];
        cout << "Insira as producoes (S->ABc):\n";
        for(int i=0; i < init->prod_count; i++) cin >> init->prod[i];

        augmented_grammar = *init;
        dotted_grammar = *init;
        add_dots(&dotted_grammar);

        terminals[no_t] = '$';
        no_t++;
    }

    void computeFirstFollow() {
        find_follow(&augmented_grammar);
    }

private:
    void add_dots(State *I) {
        for(int i=0; i < I->prod_count; i++){
            for (int j=99; j>3; j--) I->prod[i][j] = I->prod[i][j-1];
            I->prod[i][3] = '.';
        }
    }
    
    void add_to_follow(int n, char b) {
        for(size_t i=0; i<strlen(follow[n]); i++) if(follow[n][i]==b) return;
        follow[n][strlen(follow[n])] = b;
    }

    void add_to_follow(int m, int n) {
        for(size_t i=0; i<strlen(follow[n]); i++){
            int flag=0;
            for(size_t j=0; j<strlen(follow[m]); j++) if(follow[n][i] == follow[m][j]) flag=1;
            if(flag==0) add_to_follow(m, follow[n][i]);
        }
    }

    void find_follow(State *grammar) {
        add_to_follow(0, '$');
        for(int i=0; i < no_nt; i++){
            for(int j=0; j < grammar->prod_count; j++){
                int len = strlen(grammar->prod[j]);
                for(int k=3; k < len; k++){
                    if(grammar->prod[j][k] == non_terminals[i]){
                        if(k+1 < len && !is_non_terminal(grammar->prod[j][k+1])){
                            add_to_follow(i, grammar->prod[j][k+1]);
                        }
                    }
                }
            }
        }
        for(int p=0; p<2; p++) {
            for(int i=0; i < no_nt; i++){
                for(int j=0; j < grammar->prod_count; j++){
                     int len = strlen(grammar->prod[j]);
                     if(grammar->prod[j][len-1] == non_terminals[i]) {
                         char lhs = grammar->prod[j][0];
                         int lhs_idx = return_index(lhs) - no_t;
                         if(lhs_idx >= 0) add_to_follow(i, lhs_idx);
                     }
                }
            }
        }
    }
};

#endif