#include "Block.h"

USING_NS_CC;

using namespace Common;

// シーン生成
Block* Block::create(BlockType pm_Type) {
    Block* w_pRet = new Block();
    if (w_pRet && w_pRet->init(pm_Type)) {
        w_pRet->autorelease();
        return w_pRet;
    } else {
        delete w_pRet;
        w_pRet = nullptr;
        return nullptr;
    }
}

// ブロックの初期化
bool Block::init(BlockType pm_Type) {
    if (!Sprite::initWithFile(fileName(pm_Type)))
        return false;

    if (!setPhyscs(pm_Type))
        return false;

    if (!setBlockTag(pm_Type))
        return false;

    setAction(pm_Type);
    
    return true;
}

// アニメーションをセット
void Block::setAction(BlockType type) {
    // 変数を宣言
    FiniteTimeAction* w_RotateOne;
    FiniteTimeAction* w_RotateTwo;
    MoveBy* w_MoveFrom;
    MoveBy* w_MoveTo;
    Sequence* w_Seq;
    RepeatForever* w_Repeat;
;
    // 動きをセット
    /*switch (type) {
        default: 
            break;
    }*/
}

// 画像のテキストを取得
std::string Block::fileName(BlockType type) {
    // 画像名を取得
    switch (type) {
        case NormalBlock:  return  "block.png";
        case DamageBlock1: return "damageblock1.png";
        case DamageBlock2: return "damageblock2.png";
        case HealBlock:    setAnimation();
                           return "block.png";
        case PlayerDamageBlock1: return "playerdamageblock1.png";
        case PlayerDamageBlock2: return "playerdamageblock2.png";
        default:     return "null.png";
    }
}

// アニメーションをセット
void Block::setAnimation() {
    // アニメーション用スプライトを生成
    auto w_Sprite = Sprite::create("animation/null.png");
    w_Sprite->setPosition(Vec2(25, 25));

    // アニメーションを生成
    auto w_Animation = Animation::create();
    w_Animation->addSpriteFrameWithFile("animation/healBlock-0.png");
    w_Animation->addSpriteFrameWithFile("animation/healBlock-1.png");
    w_Animation->addSpriteFrameWithFile("animation/healBlock-2.png");
    w_Animation->addSpriteFrameWithFile("animation/healBlock-3.png");
    w_Animation->addSpriteFrameWithFile("animation/healBlock-4.png");
    w_Animation->addSpriteFrameWithFile("animation/healBlock-5.png");
    w_Animation->setDelayPerUnit(0.05f); // フレームごとの遅延時間;

    // アニメーションを表示
    auto w_Animate = RepeatForever::create(Animate::create(w_Animation));
    w_Sprite->runAction(w_Animate);
    addChild(w_Sprite, Z_MyTurnAnimation, T_MyTurnAnimation);
}

// 物理演算
bool Block::setPhyscs(BlockType pm_Type) {
    // 衝突オブジェクトオプション
    PhysicsMaterial w_Material;

    // 衝突オブジェクトのオプションを設定
    switch (pm_Type) {
        case NormalBlock:
        case DamageBlock1:
        case DamageBlock2:
        case HealBlock:
        case PlayerDamageBlock1:
        case PlayerDamageBlock2:
            w_Material = PhysicsMaterial(1.0, 1.0, 0.0);
            break;
        default:
            break;
    }

    // 衝突オブジェクトを設定
    PhysicsBody* w_Body;
    switch (pm_Type) {
        case NormalBlock:
        case DamageBlock1:
        case DamageBlock2:
        case HealBlock:
        case PlayerDamageBlock1:
        case PlayerDamageBlock2:
            w_Body = PhysicsBody::createBox(getContentSize(), w_Material);
            break;
        default: 
            w_Body = nullptr;
    }

    // 物理判定がないブロックは貫通ブロックとみなす
    if (w_Body) {
        w_Body->setDynamic(false);
        w_Body->setContactTestBitmask(0x01);
        setPhysicsBody(w_Body);
    }

    return true;
}

// ブロックにタグをセット
bool Block::setBlockTag(BlockType pm_Type) {
    switch (pm_Type) {
        case NormalBlock:        setTag(T_NormalBlock);        return true;
        case DamageBlock1:       setTag(T_DamageBlock1);       return true;
        case DamageBlock2:       setTag(T_DamageBlock2);       return true;
        case HealBlock:          setTag(T_HealBlock);          return true;
        case PlayerDamageBlock1: setTag(T_PlayerDamageBlock1); return true;
        case PlayerDamageBlock2: setTag(T_PlayerDamageBlock2); return true;
        default: return true;
    }

    return false;
}