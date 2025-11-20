#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>
#include <stack>
#include <map>

using namespace std;

// --- Estruturas ---
struct State {
    int prod_count;
    char prod[100][100];

    State() {
        prod_count = 0;
        for(int i=0; i<100; i++) memset(prod[i], 0, 100);
    }
};

enum ActionType { ERROR, SHIFT, REDUCE, ACCEPT };

struct Action {
    ActionType type;
    int value;
};

class SLRParser {
private:
    char terminals[100];
    int no_t;
    char non_terminals[100];
    int no_nt;

    char goto_table_char[100][100];
    char follow[20][20];
    char first[20][20];

    State I[50];
    int state_count;

    State augmented_grammar; // Gramatica limpa (para verificar Reduce)
    State dotted_grammar;    // Gramatica com pontos iniciais (para usar no Closure)

    map<pair<int, char>, Action> parsing_table;

    // --- Auxiliares Lógicos ---
    bool is_non_terminal(char a) {
        return (a >= 'A' && a <= 'Z');
    }

    bool in_state(State *state, char *a) {
        for(int i=0; i < state->prod_count; i++){
            if(!strcmp(state->prod[i], a)) return true;
        }
        return false;
    }

    char char_after_dot(char a[100]) {
        for(int i=0; i < strlen(a); i++)
            if(a[i] == '.') return a[i+1];
        return '\0';
    }

    char* move_dot(char b[100], int len) {
        static char a[100];
        memset(a, 0, 100);
        strcpy(a, b);
        for(int i=0; i < len; i++){
            if(a[i] == '.'){
                swap(a[i], a[i+1]);
                break;
            }
        }
        return a;
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

    void add_dots(State *I) {
        for(int i=0; i < I->prod_count; i++){
            for (int j=99; j>3; j--)
                I->prod[i][j] = I->prod[i][j-1];
            I->prod[i][3] = '.';
        }
    }

    void closure(State *source_grammar, State *I0) {
        char a;
        for(int i=0; i < I0->prod_count; i++){
            a = char_after_dot(I0->prod[i]);
            if(is_non_terminal(a)){
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
        for(int i=0; i < strlen(a); i++)
            if(a[i] == b) return true;
        return false;
    }

    void cleanup_prods(State *I) {
        for(int i=0; i < I->prod_count; i++)
            memset(I->prod[i], 0, 100);
        I->prod_count = 0;
    }

    int return_index(char a) {
        for(int i=0; i < no_t; i++) if(terminals[i] == a) return i;
        for(int i=0; i < no_nt; i++) if(non_terminals[i] == a) return no_t + i;
        return -1;
    }

    void add_to_follow(int n, char b) {
        for(int i=0; i<strlen(follow[n]); i++) if(follow[n][i]==b) return;
        follow[n][strlen(follow[n])] = b;
    }

    void add_to_follow(int m, int n) {
        for(int i=0; i<strlen(follow[n]); i++){
            int flag=0;
            for(int j=0; j<strlen(follow[m]); j++){
                if(follow[n][i] == follow[m][j]) flag=1;
            }
            if(flag==0) add_to_follow(m, follow[n][i]);
        }
    }

    // --- First & Follow ---
    void find_follow(State *grammar) {
        add_to_follow(0, '$');
        for(int i=0; i < no_nt; i++){
            for(int j=0; j < grammar->prod_count; j++){
                int len = strlen(grammar->prod[j]);
                for(int k=3; k < len; k++){
                    if(grammar->prod[j][k] == non_terminals[i]){
                        if(k+1 < len){
                            if(!is_non_terminal(grammar->prod[j][k+1])){
                                add_to_follow(i, grammar->prod[j][k+1]);
                            }
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
                         if(lhs_idx >= 0)
                             add_to_follow(i, lhs_idx);
                     }
                }
            }
        }
    }

public:
    SLRParser() {
        no_t = 0; no_nt = 0; state_count = 1;
        memset(terminals, 0, sizeof(terminals));
        memset(non_terminals, 0, sizeof(non_terminals));
        memset(follow, 0, sizeof(follow));
        memset(first, 0, sizeof(first));
        for(int i=0; i<100; i++) for(int j=0; j<100; j++) goto_table_char[i][j] = '~';
    }

    void inputGrammar() {
        State *init = &I[0];
        cout << "Insira o numero de producoes:\n";
        cin >> init->prod_count;

        cout << "Insira o numero de nao terminais:" << endl;
        cin >> no_nt;
        cout << "Insira os nao terminais um por vez (insere e da enter):" << endl;
        for(int i=0; i < no_nt; i++) cin >> non_terminals[i];

        cout << "Insira o numero de terminais:" << endl;
        cin >> no_t;
        cout << "Insira os terminais um por vez (insere e da enter):" << endl;
        for(int i=0; i < no_t; i++) cin >> terminals[i];

        cout << "Insira as producoes uma por uma, da forma: (S->ABc):\n";
        for(int i=0; i < init->prod_count; i++){
            cin >> init->prod[i];
        }

        augmented_grammar = *init; // Salva limpa
        dotted_grammar = *init;    // Salva para adicionar pontos
        add_dots(&dotted_grammar); // Adiciona pontos na gramatica de referência

        // Configura I[0]
        if(dotted_grammar.prod[0][0] == 'S')
            strcpy(I[0].prod[0], "Z->.S");
        else {
            strcpy(I[0].prod[0], "S->.");
            I[0].prod[0][4] = dotted_grammar.prod[0][0];
            I[0].prod[0][5] = '\0'; // Correção do bug de string anterior
        }
        I[0].prod_count = 1;

        // Closure agora usa dotted_grammar
        closure(&dotted_grammar, &I[0]);

        terminals[no_t] = '$';
        no_t++;
    }

    void generateTables() {
        for(int i=0; i < state_count; i++){
            char characters[20] = {};
            for(int z=0; z < I[i].prod_count; z++)
                if(!in_array(characters, char_after_dot(I[i].prod[z])))
                    characters[strlen(characters)] = char_after_dot(I[i].prod[z]);

            for(int j=0; j < strlen(characters); j++){
                goto_state(&I[i], &I[state_count], characters[j]);

                // CORRECAO PRINCIPAL: Usa dotted_grammar aqui
                closure(&dotted_grammar, &I[state_count]);

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

        find_follow(&augmented_grammar);
    }

    void imprimirTodosEstados() {
        cout << "\n=========================================";
        cout << "\n      LISTA DE ESTADOS CANONICOS";
        cout << "\n=========================================\n";

        for(int i=0; i < state_count; i++) {
            cout << "Estado I" << i << ":\n";
            cout << "---------\n";
            for(int j=0; j < I[i].prod_count; j++) {
                cout << "  " << I[i].prod[j] << endl;
            }

            bool temTransicoes = false;
            for(int k=0; k < state_count; k++) {
                if(goto_table_char[i][k] != '~') {
                    if(!temTransicoes) { cout << "  Transicoes:\n"; temTransicoes = true; }
                    cout << "    Lendo '" << goto_table_char[i][k] << "' -> vai para I" << k << endl;
                }
            }
            cout << endl;
        }
    }

    void printAndStoreShiftTable() {
        cout << endl << "******** ACOES SHIFT (GOTO) *********" << endl << endl;
        cout << "\t";
        for(int i=0; i < no_t; i++) cout << terminals[i] << "\t";
        for(int i=0; i < no_nt; i++) cout << non_terminals[i] << "\t";
        cout << endl;

        for(int i=0; i < state_count; i++){
            int arr[200]; memset(arr, -1, sizeof(arr));
            for(int j=0; j < state_count; j++){
                if(goto_table_char[i][j] != '~')
                    arr[return_index(goto_table_char[i][j])] = j;
            }

            cout << "I" << i << "\t";
            for(int j=0; j < no_nt + no_t; j++){
                if(i == 1 && j == no_t-1) {
                    cout << "ACC" << "\t";
                    parsing_table[{i, '$'}] = {ACCEPT, 0};
                }

                if(arr[j] == -1) cout << "\t";
                else {
                    if(j < no_t) {
                        cout << "S" << arr[j] << "\t";
                        parsing_table[{i, terminals[j]}] = {SHIFT, arr[j]};
                    } else {
                        cout << arr[j] << "\t";
                        parsing_table[{i, non_terminals[j-no_t]}] = {SHIFT, arr[j]};
                    }
                }
            }
            cout << "\n";
        }
    }

    void printAndStoreReduceTable() {
        cout << "********** ACOES REDUCE **********" << endl << endl;
        cout << "\t";
        for(int i=0; i < no_t; i++) cout << terminals[i] << "\t";
        cout << endl;

        for(int i=0; i < state_count; i++) {
            cout << "I" << i << "\t";

            int reductions[100];
            for(int k=0; k<100; k++) reductions[k] = -1;

            for(int item=0; item < I[i].prod_count; item++) {
                string s = I[i].prod[item];

                if(s.length() > 0 && s.back() == '.') {
                    string raw_prod = s.substr(0, s.length()-1);

                    if(raw_prod.find("S->") == 0 || raw_prod.find("Z->") == 0) continue;

                    int rule_idx = -1;
                    for(int r=0; r < augmented_grammar.prod_count; r++) {
                        if(string(augmented_grammar.prod[r]) == raw_prod) {
                            rule_idx = r;
                            break;
                        }
                    }

                    if(rule_idx != -1) {
                        char lhs = raw_prod[0];
                        int lhs_idx = return_index(lhs) - no_t;

                        if(lhs_idx >= 0) {
                            int len_follow = strlen(follow[lhs_idx]);
                            for(int f=0; f < len_follow; f++) {
                                char sym = follow[lhs_idx][f];
                                for(int t=0; t < no_t; t++) {
                                    if(terminals[t] == sym) {
                                        reductions[t] = rule_idx + 1;
                                        if(parsing_table.find({i, sym}) == parsing_table.end()) {
                                            parsing_table[{i, sym}] = {REDUCE, rule_idx};
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            for(int t=0; t < no_t; t++) {
                if(reductions[t] != -1) cout << "R" << reductions[t] << "\t";
                else cout << "\t";
            }
            cout << endl;
        }
    }

    void verificarSintaxe(string entrada) {
        cout << "\n=== Verificacao da Palavra: " << entrada << " ===\n";
        entrada += "$";
        stack<int> pilha;
        pilha.push(0);

        int ptr = 0;
        bool aceito = false;

        cout << "Pilha\t\tEntrada\t\tAcao\n";
        cout << "-----\t\t-------\t\t----\n";

        while(true) {
            int s = pilha.top();
            char a = entrada[ptr];

            stack<int> temp = pilha;
            vector<int> p_view;
            while(!temp.empty()) { p_view.push_back(temp.top()); temp.pop(); }
            for(int x=p_view.size()-1; x>=0; x--) cout << p_view[x] << " ";
            cout << "\t\t" << entrada.substr(ptr) << "\t\t";

            if(parsing_table.find({s, a}) == parsing_table.end()) {
                cout << "ERRO SINTATICO (Sem acao para [" << s << ", " << a << "])" << endl;
                break;
            }

            Action acao = parsing_table[{s, a}];

            if(acao.type == SHIFT) {
                cout << "Shift " << acao.value << endl;
                pilha.push(acao.value);
                ptr++;
            }
            else if(acao.type == REDUCE) {
                int regra_idx = acao.value;
                string regra = augmented_grammar.prod[regra_idx];
                int rhs_len = regra.length() - 3;

                cout << "Reduce " << regra << endl;

                for(int k=0; k<rhs_len; k++) {
                    if(!pilha.empty()) pilha.pop();
                }

                if(pilha.empty()) { cout << "Erro pilha vazia." << endl; break; }

                int topo_atual = pilha.top();
                char lhs = regra[0];

                if(parsing_table.find({topo_atual, lhs}) != parsing_table.end()) {
                    int proximo = parsing_table[{topo_atual, lhs}].value;
                    pilha.push(proximo);
                } else {
                     cout << "ERRO GOTO (Sem transicao [" << topo_atual << ", " << lhs << "])" << endl;
                     break;
                }

            }
            else if(acao.type == ACCEPT) {
                cout << "ACEITO" << endl;
                aceito = true;
                break;
            }
        }

        if(aceito) cout << "\n>> Palavra Aceita Sintaticamente!\n";
        else cout << "\n>> Palavra Rejeitada.\n";
    }
};

int main() {
    SLRParser parser;
    parser.inputGrammar();
    parser.generateTables();
    parser.imprimirTodosEstados();
    parser.printAndStoreShiftTable();
    cout << endl << endl;
    parser.printAndStoreReduceTable();

    char continuar;
    do {
        string palavra;
        cout << "\nDigite uma palavra para verificar (ex: id*id): ";
        cin >> palavra;
        parser.verificarSintaxe(palavra);
        cout << "Verificar outra? (s/n): ";
        cin >> continuar;
    } while(continuar == 's' || continuar == 'S');

    return 0;
}