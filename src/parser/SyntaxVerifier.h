#ifndef SYNTAX_VERIFIER_HPP
#define SYNTAX_VERIFIER_HPP

#include <iostream>
#include <stack>
#include <string>
#include <vector>
#include "../parser/slrparser.h"

using namespace std;

class SyntaxVerifier {
public:
    SLRGenerator* generator;

    SyntaxVerifier(SLRGenerator* gen) : generator(gen) {}

    void verify(string entrada) {
        cout << "\nVerificacao da Palavra: " << entrada << "\n";
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

            printDebug(pilha, entrada, ptr);

            if(generator->parsing_table.find({s, a}) == generator->parsing_table.end()) {
                cout << "ERRO SINTATICO (Sem acao para [" << s << ", " << a << "])" << endl;
                break;
            }

            Action acao = generator->parsing_table[{s, a}];

            if(acao.type == SHIFT) {
                cout << "Shift " << acao.value << endl;
                pilha.push(acao.value);
                ptr++;
            }
            else if(acao.type == REDUCE) {
                int regra_idx = acao.value;
                string regra = generator->grammar->augmented_grammar.prod[regra_idx];
                int rhs_len = regra.length() - 3;

                cout << "Reduce " << regra << endl;

                for(int k=0; k<rhs_len; k++) {
                    if(!pilha.empty()) pilha.pop();
                }

                if(pilha.empty()) { cout << "Erro pilha vazia." << endl; break; }

                int topo_atual = pilha.top();
                char lhs = regra[0];

                if(generator->parsing_table.find({topo_atual, lhs}) != generator->parsing_table.end()) {
                    pilha.push(generator->parsing_table[{topo_atual, lhs}].value);
                } else {
                     cout << "ERRO no GOTO (Sem transicao [" << topo_atual << ", " << lhs << "])" << endl;
                     break;
                }
            }
            else if(acao.type == ACCEPT) {
                cout << "ACEITO" << endl;
                aceito = true;
                break;
            }
        }

        if(aceito) cout << "\n>> Palavra Aceita Sintaticamente\n";
        else cout << "\n>> Palavra Rejeitada.\n";
    }

private:
    void printDebug(stack<int> pilha, string entrada, int ptr) {
        stack<int> temp = pilha;
        vector<int> p_view;
        while(!temp.empty()) { p_view.push_back(temp.top()); temp.pop(); }
        for(int x=p_view.size()-1; x>=0; x--) cout << p_view[x] << " ";
        cout << "\t\t" << entrada.substr(ptr) << "\t\t";
    }
};

#endif