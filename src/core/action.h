#pragma once

enum ActionType { ERROR, SHIFT, REDUCE, ACCEPT };

struct Action {
    ActionType type;
    int value;
};