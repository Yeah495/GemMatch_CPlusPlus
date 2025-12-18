#pragma once
/*● Gem.h (结构体/类)
  ○ 描述：单个宝石。
  ○ 属性： 定义宝石的颜色  type, state (正常/消除中/下落中)。*/


#ifndef GEM_H
#define GEM_H

#include "Config.h"

struct Gem {
    GemType type;
    GemState state;
    bool markedForDeletion; // 标记是否待消除
    
    //构造函数默认 type=Empty、state=Static、markedForDeletion=false。
    Gem(GemType t = GemType::Empty)
        : type(t), state(GemState::Static), markedForDeletion(false) {
    }

    // 重载相等运算符，方便比较
    bool operator==(const Gem& other) const {
        return type == other.type;
    }
};

#endif // GEM_H