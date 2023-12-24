#include "Background.h"

USING_NS_CC;
using namespace Common;

// 背景生成
Background* Background::create(int pm_LayerNumber) {
    Background* w_pRet = new Background();
    if (w_pRet && w_pRet->init(pm_LayerNumber)) {
        w_pRet->autorelease();
        return w_pRet;
    } else {
        delete w_pRet;
        w_pRet = nullptr;
        return nullptr;
    }
}

// ブロックの初期化
bool Background::init(int pm_LayerNumber) {
    // 画像を設定
    if (!Sprite::initWithFile(fileName(pm_LayerNumber)))
        return false;

    // 物理演算を設定
    if(!(pm_LayerNumber == L_Title  || 
         pm_LayerNumber == L_Select ||
         pm_LayerNumber == L_Game)) {
        if (!setPhyscs())
            return false;
    }
    
    return true;
}

// 背景画像
std::string Background::fileName(int pm_LayerNumber) {
    // 画像名を取得
    switch (pm_LayerNumber) {
        case L_Title:  return "title.png";
        case L_Select: return "select.png";
        case L_Bg1:    return "world.png";
        case L_Bg2:    return "background.png";
        case L_Bg3:    return "background.png";
        case L_Game:   return "background.png";
        default:       return "";
    }
}

// 物理演算
bool Background::setPhyscs() {
    // 物質的特徴の設定
    PhysicsMaterial w_Material;
    w_Material.density = 1;
    w_Material.restitution = 1;
    w_Material.friction = 0;

    // 物理構造の設定
    auto w_Body = PhysicsBody::createEdgeBox(Size(this->getContentSize()), w_Material, 30);
    w_Body->setDynamic(false);

    // 背景枠ノードの作成
    auto w_Node = Node::create();
    w_Node->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
    w_Node->setPhysicsBody(w_Body);
    w_Node->setPosition(Vec2(this->getContentSize()/2));
    addChild(w_Node);

    return true;
}