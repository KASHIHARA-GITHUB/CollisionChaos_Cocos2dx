#ifndef __BACKGROUND__H__
#define __BACKGROUND__H__

#include "cocos2d.h"
#include "Common.h"

class Background : public cocos2d::Sprite {
protected:
    bool setPhyscs();                               // 物理演算をセット
    std::string fileName(int pm_LayerNumber);       // 画像名取得

public:
    static Background* create(int pm_LayerNumber);	// インスタンス生成
    virtual bool init(int pm_LayerNumber);          // 初期化
};

#endif // __BACKGROUND__H__
