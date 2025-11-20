#include <iostream>
#include <string>
#include "core/grammar.h"
#include "parser/slrparser.h"
#include "parser/SyntaxVerifier.h"

using namespace std;

int main() {
    Grammar grammar;
    grammar.readInput();
    grammar.computeFirstFollow();

    SLRGenerator slr(&grammar);
    slr.buildCanonicalCollection();
    slr.fillParsingTable();

    SyntaxVerifier verifier(&slr);

    char continuar;
    do {
        string palavra;
        cout << "\nDigite uma palavra para verificar (ex: id*id): ";
        cin >> palavra;
        verifier.verify(palavra);
        cout << "Verificar outra? (s/n): ";
        cin >> continuar;
    } while(continuar == 's' || continuar == 'S');

    return 0;
}