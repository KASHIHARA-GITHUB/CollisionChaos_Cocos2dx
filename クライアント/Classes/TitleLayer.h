#ifndef __TITLELAYER__H__
#define __TITLELAYER__H__

#include "cocos2d.h"
#include "Background.h"
#include "Audio.h"
#include "Tap.h"
#include "GameLayer.h"

// タイトル画面クラス
class TitleLayer : public cocos2d::Layer {
protected:
    void createBackground();        // 背景の生成
    void createTitleAndButton();    // タイトルとボタンを生成
    void tapAnimation(cocos2d::Touch* pm_Touch);   // タップアニメーション

    // シングルタップイベント
    virtual bool onTouchBegan(cocos2d::Touch* pm_Touch, cocos2d::Event* pm_Unused_event);

public:
    static cocos2d::Scene* createScene();   // シーンの生成
    virtual bool init();        // 初期化
    virtual void onEnter();     // レイヤー表示処理
    CREATE_FUNC(TitleLayer);    // Create関数生成マクロ
};

#endif // __TITLELAYER__H__
