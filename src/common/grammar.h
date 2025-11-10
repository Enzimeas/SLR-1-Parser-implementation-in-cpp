#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>



enum class SymbolType
{
    TERMINAL,
    NON_TERMINAL
};


struct Symbol
{
    int id;
    std::string name;
    SymbolType type;

    Symbol(int id, const std::string& name, SymbolType type)
        : id(id), name(name), type(type) {}
    
    // para std::map e std::set
    bool operator<(const Symbol& other) const {
        return id < other.id;
    }
};


struct Production
{
    int id;
    Symbol* head;
    std::vector<Symbol*> body;

    Production(int id, const Symbol& head,const std::vector<Symbol*>& body)
        id(id), head(head), type(type) {}
};


class Grammar
{
    public:

        explicit Grammar(const std:string& filepath);

        const std::vector<std::unique_ptr<Production>>& getProductions() const { return m_productions; }
        const std::vector<std::unique_ptr<Symbol>>& getSymbols() const { return m_symbols; }
        Symbol* getStartSymbol() const { return m_augmentedStartSymbol; }
        Symbol* getOriginalStartSymbol() const { return m_originalStartSymbol; }
        Symbol* getEofSymbol() const { return EofSymbol; }
        Symbol* getSymbolByName(const std::string& name) const;

    private:

        std::vector<std::unique_ptr<Symbol>> m_symbols;
        std::vector<std::unique_ptr<Production>> m_productions;

        std::map<std::string, Symbol*> m_symbol_map;

        Symbol* m_originalStartSymbol = nullptr;
        Symbol* m_augmentedStartSymbol = nullptr;
        Symbol* EofSymbol = nullptr;

        int m_symbolCounter = 0;
        int m_productionCounter = 0;

        void loadFromFile(const std::string& filepath);

        Symbol* findOrCreateSymbol(const std::string& name);
        
        Production* addNewProduction(Symbol* head);

        void identifyNonTerminals();

        void augmentGrammar();
};