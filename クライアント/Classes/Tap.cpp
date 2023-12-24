#include "Tap.h"

#define ImageMaxX 5 // アニメーション画像位置X
#define ImageMaxY 2 // アニメーション画像位置Y
#define AnimationSize 192   // アニメーション画像一枚のサイズ
#define AnimationDuration  0.05f    // アニメーションの切り替わり時間間隔

USING_NS_CC;
using namespace Common;

//シーン生成
Tap* Tap::create() {
    Tap* w_pRet = new Tap();
    if (w_pRet && w_pRet->init()) {
        w_pRet->autorelease();
        return w_pRet;
    } else {
        delete w_pRet;
        w_pRet = nullptr;
        return nullptr;
    }
}

//ブロックの初期化
bool Tap::init() {
    //タップアニメーション
    if (!setAnimation())
        return false;
    
    return true;
}

// アニメーション処理
bool Tap::setAnimation() {
    // タップ画像取得
    auto w_BaseSprite = Sprite::create("tap.png");

    // タップ画像からテクスチャーの取得
    auto w_TextureSource = w_BaseSprite->getTexture();

    // アニメーションを格納するオブジェクトの作成
    auto w_Animation = Animation::create();
    w_Animation->setDelayPerUnit(AnimationDuration); // 0.05秒間隔で無限に繰り返す
    w_Animation->setRestoreOriginalFrame(true);

    // テクスチャーから画像を切り出して追加
    for (int y = 0; y < ImageMaxY; y++) {
        for (int x = 0; x < ImageMaxX; x++) {
            w_Animation->addSpriteFrameWithTexture(
                w_TextureSource, Rect(AnimationSize * x,
                                      AnimationSize * y,
                                      AnimationSize, 
                                      AnimationSize)
            );
        }
    }

    // アニメーション終了後、タップオブジェクトを削除
    auto w_Func = CallFunc::create([&]() {
        auto w_TapAnim = getChildByTag(T_Tap);
        removeChild(w_TapAnim,true);
    });

    // タップオブジェクトを生成
    auto w_Seq = Sequence::create(Animate::create(w_Animation), w_Func, nullptr);
    w_BaseSprite->runAction(w_Seq);
    addChild(w_BaseSprite, Z_Tap, T_Tap);

    return true;
}