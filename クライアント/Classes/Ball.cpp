#include "Ball.h"

USING_NS_CC;
using namespace Common;

#define PLAYERNUM 19 // プレイヤー番号初期値
#define GAGEPOSX -25 // ゲージの位置X
#define GAGEPOSY 40  // ゲージの位置Y
#define HPTAG    300 // 体力タグ

// シーン生成
Ball* Ball::create(int pm_PlayerNum) {
    Ball* w_pRet = new Ball();
    if (w_pRet && w_pRet->init(pm_PlayerNum)) {
        w_pRet->autorelease();
        return w_pRet;
    } else {
        delete w_pRet;
        w_pRet = nullptr;
        return nullptr;
    }
}

// ブロックの初期化
bool Ball::init(int pm_PlayerNum) {
    // 画像を設定
    if (!setImage(pm_PlayerNum))
        return false;

    // 物理演算を設定
    if (!setPhyscs())
        return false;
    
    return true;
}

// ボールの画像をセット
bool Ball::setImage(int pm_PlayerNum) {
    // ボールの生成
    Sprite::initWithFile("ball.png");

    // プレイヤーに色を付ける
    switch (pm_PlayerNum){
        case 1:
            setColor(Color3B::RED);
            break;
        case 2:
            setColor(Color3B::GREEN);
            break;
    }

    // タグ値設定
    int w_TagNum = pm_PlayerNum + PLAYERNUM;

    // 矢印の生成
    auto w_Arrow = Sprite::create("arrow.png");
    auto w_BallPos = getContentSize() / 2;
    w_Arrow->setPosition(Point(w_BallPos));
    w_Arrow->setTag(w_TagNum);
    w_Arrow->setVisible(false);
    addChild(w_Arrow, Z_Arrow);

    // ダメージゲージを生成
    auto w_DamageGage = Sprite::create("damage.png");
    w_DamageGage->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    w_DamageGage->setPosition(Point(GAGEPOSX, w_BallPos.height + GAGEPOSY));
    addChild(w_DamageGage, Z_Gage);

    // 体力ゲージを生成
    auto w_HpGage = Sprite::create("hp.png");
    w_HpGage->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    w_HpGage->setPosition(Point(GAGEPOSX, w_BallPos.height + GAGEPOSY));
    w_HpGage->setTag(HPTAG + pm_PlayerNum - 1);
    addChild(w_HpGage, Z_Gage);

    return true;
}

// 物理演算
bool Ball::setPhyscs() {
    // 物質的特徴の設定
    PhysicsMaterial w_Material;
    w_Material.density = 1;
    w_Material.restitution = 1;
    w_Material.friction = 0;

    // 物理構造の設定
    auto w_Body = PhysicsBody::createCircle(getContentSize().width * 0.47, w_Material);
    w_Body->setDynamic(true);
    w_Body->setRotationEnable(false);
    w_Body->setContactTestBitmask(0x01);
    w_Body->setLinearDamping(0.4f);
    setPhysicsBody(w_Body);

    return true;
}