#include "grammar.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <set>
#include <memory>


Grammar::Grammar(const std::string& filepath)
{

    EofSymbol = findOrCreateSymbol("$");
    EofSymbol->id = 0;
    m_symbolCounter = 1;

    loadFromFile(filepath);

    identifyNonTerminals();

    augmentGrammar();
    
    std::cout << "Gramática carregada e aumentada" << std::endl;
    std::cout << m_productions.size() << " produções e " << m_symbols.size() << " símbolos." << std::endl;
    std::cout << "Símbolo Inicial: " << m_augmentedStartSymbol->name << std::endl;
}


void Grammar::loadFromFile(const std::string& filepath)
{
    std::ifstream file(filepath);

    if (!file.is_open())
        throw std::runtime_error("Não foi possível abrir o txt da gramática: " + filepath);

    std::string line;
    bool firstRule = true;

    while (std::getline(file, line))
    {

        if (line.empty() || line[0] == '#')
            continue;

        std::stringstream ss(line);
        std::string token;
        
        ss >> token;

        if (token.empty())
            continue;
        
        Symbol* head = findOrCreateSymbol(token);
        
        if (firstRule)
        {
            m_originalStartSymbol = head;
            firstRule = false;
        }

        ss >> token;

        if (token != "->")
            throw std::runtime_error("Formato de regra inválido (esperava '->'): " + line);

        Production* prod = addNewProduction(head);

        while (ss >> token)
        {
            Symbol* bodySymbol = findOrCreateSymbol(token);
            prod->body.push_back(bodySymbol);
        }
    }
}


void Grammar::identifyNonTerminals()
{
    std::set<Symbol*> nonTerminals;

    for (const auto& prod : m_productions)
        nonTerminals.insert(prod->head);
    
    for (Symbol* nt : nonTerminals)
        nt->type = SymbolType::NON_TERMINAL;
}


void Grammar::augmentGrammar()
{
    if (m_originalStartSymbol == nullptr)
        throw std::runtime_error("Gramática está vazia");
    
    std::string newStartName = m_originalStartSymbol->name + "'";
    m_augmentedStartSymbol = findOrCreateSymbol(newStartName);
    m_augmentedStartSymbol->type = SymbolType::NON_TERMINAL;
    
    Production* augProd = new Production(m_productionCounter++, m_augmentedStartSymbol);
    augProd->body.push_back(m_originalStartSymbol);
    augProd->body.push_back(EofSymbol);
    
    m_productions.insert(m_productions.begin(), std::make_unique<Production>(*augProd));
    
    for(size_t i = 0; i < m_productions.size(); ++i)
        m_productions[i]->id = i;
}


Symbol* Grammar::findOrCreateSymbol(const std::string& name)
{
    if (m_symbol_map.count(name))
        return m_symbol_map[name];
    
    int newId = m_symbolCounter++;
    Symbol* newSymbol = new Symbol(newId, name, SymbolType::TERMINAL);
    
    m_symbols.push_back(std::unique_ptr<Symbol>(newSymbol));
    m_symbol_map[name] = newSymbol;
    
    return newSymbol;
}


Production* Grammar::addNewProduction(Symbol* head)
{
    int newId = m_productionCounter++;
    Production* newProd = new Production(newId, head);
    
    m_productions.push_back(std::unique_ptr<Production>(newProd));
    return newProd;
}


Symbol* Grammar::getSymbolByName(const std::string& name) const
{
    if (m_symbol_map.count(name)) 
        return m_symbol_map.at(name);

    return nullptr;
}