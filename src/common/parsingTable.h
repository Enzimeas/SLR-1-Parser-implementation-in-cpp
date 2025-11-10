#pragma once

#include <vector>
#include <map>
#include <string>



enum class ActionType
{
    SHIFT,
    REDUCE,
    ACCEPT,
    ERROR
};


struct Action
{
    ActionType type = ActionType::ERROR;
    int value = -1;
};


class ParsingTable
{
    public:
    
        std::vector<std::map<int, Action>> action_table;
        std::vector<std::map<int, int>> goto_table;

        void setNumberOfStates(int numStates)
        {
            action_table.resize(numStates);
            goto_table.resize(numStates);
        }
        
        void saveTable(const std::string& filepath) const
        {
            // vai salvar a tabela
        }

        void loadTable(const std::string& filepath) const
        {
            // vai carregar uma tabela
        }
};