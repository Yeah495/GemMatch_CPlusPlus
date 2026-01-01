#pragma once
#ifndef GRAVITYSYSTEM_H
#define GRAVITYSYSTEM_H

#include "Board.h"
#include <queue>

class GravitySystem {
public:
    static bool applyGravity(Board& board , int gemTypeCount = 3);
};

#endif // GRAVITYSYSTEM_H