#ifndef __TAP__H__
#define __TAP__H__

#include "cocos2d.h"
#include "Common.h"

// タップクラス
class Tap : public cocos2d::Sprite {
private:
    bool setAnimation();    //アニメーション表示

public:
    static Tap* create();	//インスタンス生成
    virtual bool init();    //初期化
};

#endif // __TAP__H__
