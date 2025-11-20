#pragma once

#include <cstring>


struct State {
    int prod_count;
    char prod[100][100];

    State() {
        prod_count = 0;
        for(int i=0; i<100; i++) std::memset(prod[i], 0, 100);
    }
};