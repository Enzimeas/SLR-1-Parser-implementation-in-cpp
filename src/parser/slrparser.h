#ifndef SLR_GENERATOR_HPP
#define SLR_GENERATOR_HPP

#include <map>
#include "../core/grammar.h"
#include "../models/action.h"

using namespace std;

class SLRGenerator {
public:
    Grammar* grammar;
    State I[50];
    int state_count;
    char goto_table_char[100][100];
    map<pair<int, char>, Action> parsing_table;

    SLRGenerator(Grammar* g) : grammar(g), state_count(1) {
        for(int i=0; i<100; i++)
            for(int j=0; j<100; j++) goto_table_char[i][j] = '~';
    }

    void buildCanonicalCollection() {
        if(grammar->dotted_grammar.prod[0][0] == 'S')
             strcpy(I[0].prod[0], "Z->.S");
        else {
             strcpy(I[0].prod[0], "S->.");
             I[0].prod[0][4] = grammar->dotted_grammar.prod[0][0];
             I[0].prod[0][5] = '\0';
        }
        I[0].prod_count = 1;
        closure(&grammar->dotted_grammar, &I[0]);

        for(int i=0; i < state_count; i++){
            char characters[20] = {};
            for(int z=0; z < I[i].prod_count; z++)
                if(!in_array(characters, char_after_dot(I[i].prod[z])))
                    characters[strlen(characters)] = char_after_dot(I[i].prod[z]);

            for(size_t j=0; j < strlen(characters); j++){
                goto_state(&I[i], &I[state_count], characters[j]);
                closure(&grammar->dotted_grammar, &I[state_count]);

                int flag = 0;
                for(int k=0; k < state_count; k++){
                    if(same_state(&I[k], &I[state_count])){
                        cleanup_prods(&I[state_count]);
                        flag = 1;
                        goto_table_char[i][k] = characters[j];
                        break;
                    }
                }
                if(flag == 0){
                    goto_table_char[i][state_count] = characters[j];
                    state_count++;
                }
            }
        }
    }

    void fillParsingTable() {
        for(int i=0; i < state_count; i++){
            for(int j=0; j < state_count; j++){
                if(goto_table_char[i][j] != '~'){
                    char sym = goto_table_char[i][j];
                    if (grammar->is_non_terminal(sym)) {
                         parsing_table[{i, sym}] = {SHIFT, j};
                    } else {
                         parsing_table[{i, sym}] = {SHIFT, j};
                    }
                }
            }
            parsing_table[{1, '$'}] = {ACCEPT, 0};
        }

        for(int i=0; i < state_count; i++) {
            for(int item=0; item < I[i].prod_count; item++) {
                string s = I[i].prod[item];
                if(s.length() > 0 && s.back() == '.') {
                    string raw_prod = s.substr(0, s.length()-1);
                    if(raw_prod.find("S->") == 0 || raw_prod.find("Z->") == 0) continue;

                    int rule_idx = -1;
                    for(int r=0; r < grammar->augmented_grammar.prod_count; r++) {
                        if(string(grammar->augmented_grammar.prod[r]) == raw_prod) {
                            rule_idx = r; break;
                        }
                    }

                    if(rule_idx != -1) {
                        char lhs = raw_prod[0];
                        int lhs_idx = grammar->return_index(lhs) - grammar->no_t;
                        if(lhs_idx >= 0) {
                            for(size_t f=0; f < strlen(grammar->follow[lhs_idx]); f++) {
                                char sym = grammar->follow[lhs_idx][f];
                                parsing_table[{i, sym}] = {REDUCE, rule_idx};
                            }
                        }
                    }
                }
            }
        }
    }


    void imprimirTodosEstados()
    {

        for(int i=0; i < state_count; i++)
        {
            cout << "Estado I" << i << ":\n";
            cout << "---------\n";

            for(int j=0; j < I[i].prod_count; j++)
            {
                cout << "  " << I[i].prod[j] << endl;
            }

            bool temTransicoes = false;
            for(int k=0; k < state_count; k++)
            {
                if(goto_table_char[i][k] != '~')
                {
                    if(!temTransicoes)
                        cout << "  Transicoes:\n"; temTransicoes = true;

                    cout << "    Lendo '" << goto_table_char[i][k] << "' -> vai para I" << k << endl;
                }
            }
            cout << endl;
        }
    }

    void printShiftTable()
    {
        cout << endl << "******** ACOES SHIFT (GOTO) *********" << endl << endl;
        cout << "\t";
        for(int i=0; i < grammar->no_t; i++) cout << grammar->terminals[i] << "\t";
        for(int i=0; i < grammar->no_nt; i++) cout << grammar->non_terminals[i] << "\t";
        cout << endl;

        for(int i=0; i < state_count; i++)
        {
            int arr[200]; memset(arr, -1, sizeof(arr));
            for(int j=0; j < state_count; j++)
            {
                if(goto_table_char[i][j] != '~')
                    arr[grammar->return_index(goto_table_char[i][j])] = j;
            }

            cout << "I" << i << "\t";

            for(int j=0; j < grammar->no_nt + grammar->no_t; j++)
            {
                if(i == 1 && j == grammar->no_t-1)
                    cout << "ACC" << "\t";

                if(arr[j] == -1)
                {
                    cout << "\t";
                }
                else
                {
                    if(j < grammar->no_t) cout << "S" << arr[j] << "\t";
                    else cout << arr[j] << "\t";
                }
            }
            cout << "\n";
        }
    }

    void printReduceTable()
    {
        cout << "\n********** ACOES REDUCE **********" << endl << endl;
        cout << "\t";
        for(int i=0; i < grammar->no_t; i++) cout << grammar->terminals[i] << "\t";
        cout << endl;

        for(int i=0; i < state_count; i++)
        {
            cout << "I" << i << "\t";
            int reductions[100]; for(int k=0; k<100; k++) reductions[k] = -1;

            for(int item=0; item < I[i].prod_count; item++)
            {
                string s = I[i].prod[item];
                if(s.length() > 0 && s.back() == '.')
                {
                    string raw_prod = s.substr(0, s.length()-1);
                    if(raw_prod.find("S->") == 0 || raw_prod.find("Z->") == 0) continue;

                    int rule_idx = -1;
                    for(int r=0; r < grammar->augmented_grammar.prod_count; r++)
                    {
                        if(string(grammar->augmented_grammar.prod[r]) == raw_prod)
                            rule_idx = r; break;
                    }

                    if(rule_idx != -1)
                    {
                        char lhs = raw_prod[0];
                        int lhs_idx = grammar->return_index(lhs) - grammar->no_t;
                        if(lhs_idx >= 0)
                        {
                            for(size_t f=0; f < strlen(grammar->follow[lhs_idx]); f++)
                            {
                                char sym = grammar->follow[lhs_idx][f];
                                for(int t=0; t < grammar->no_t; t++)
                                {
                                    if(grammar->terminals[t] == sym) reductions[t] = rule_idx + 1;
                                }
                            }
                        }
                    }
                }
            }
            for(int t=0; t < grammar->no_t; t++) {
                if(reductions[t] != -1) cout << "R" << reductions[t] << "\t";
                else cout << "\t";
            }
            cout << endl;
        }
    }


private:
    char char_after_dot(char a[100]) {
        for(size_t i=0; i < strlen(a); i++) if(a[i] == '.') return a[i+1];
        return '\0';
    }

    char* move_dot(char b[100], int len) {
        static char a[100]; memset(a, 0, 100); strcpy(a, b);
        for(int i=0; i < len; i++){
            if(a[i] == '.'){ swap(a[i], a[i+1]); break; }
        }
        return a;
    }

    bool in_state(State *state, char *a) {
        for(int i=0; i < state->prod_count; i++) if(!strcmp(state->prod[i], a)) return true;
        return false;
    }

    bool same_state(State *I0, State *I_new) {
        if (I0->prod_count != I_new->prod_count) return false;
        for (int i=0; i < I0->prod_count; i++) {
            int flag = 0;
            for (int j=0; j < I_new->prod_count; j++)
                if (strcmp(I0->prod[i], I_new->prod[j]) == 0) flag = 1;
            if (flag == 0) return false;
        }
        return true;
    }

    void closure(State *source_grammar, State *I0) {
        char a;
        for(int i=0; i < I0->prod_count; i++){
            a = char_after_dot(I0->prod[i]);
            if(grammar->is_non_terminal(a)){
                for(int j=0; j < source_grammar->prod_count; j++){
                    if(source_grammar->prod[j][0] == a){
                        if(!in_state(I0, source_grammar->prod[j])){
                            strcpy(I0->prod[I0->prod_count], source_grammar->prod[j]);
                            I0->prod_count++;
                        }
                    }
                }
            }
        }
    }

    void goto_state(State *I_src, State *S_dest, char a) {
        int time = 1;
        S_dest->prod_count = 0;
        for(int i=0; i < I_src->prod_count; i++){
            if(char_after_dot(I_src->prod[i]) == a){
                if(time == 1) time++;
                strcpy(S_dest->prod[S_dest->prod_count], move_dot(I_src->prod[i], strlen(I_src->prod[i])));
                S_dest->prod_count++;
            }
        }
    }

    bool in_array(char a[20], char b) {
        for(size_t i=0; i < strlen(a); i++) if(a[i] == b) return true;
        return false;
    }

    void cleanup_prods(State *I) {
        for(int i=0; i < I->prod_count; i++) memset(I->prod[i], 0, 100);
        I->prod_count = 0;
    }
};

#endif