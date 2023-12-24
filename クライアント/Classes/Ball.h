#ifndef __BALL__H__
#define __BALL__H__

#include "cocos2d.h"
#include "Common.h"

class Ball : public cocos2d::Sprite {
    public:
        bool setImage(int pm_PlayerNum);        // ボールのイメージを取得
        bool setPhyscs();                       // 物理演算をセット
        static Ball* create(int pm_PlayerNum);	// インスタンス生成
        virtual bool init(int pm_PlayerNum);    // 初期化
};

#endif // __BALL__H__
