#ifndef __BLOCK__H__
#define __BLOCK__H__

#include "cocos2d.h"
#include "Common.h"

using namespace Common;

// ブロッククラス
class Block : public cocos2d::Sprite
{
protected:
    // ゴールの衝突オブジェクトをセット
    std::string fileName(BlockType type);   // ファイル名取得
    void setAnimation();                    // アニメーションをセット
    bool setPhyscs(BlockType type);         // 物理演算をセット
    bool setBlockTag(BlockType type);       // タグをセット
    void setAction(BlockType type);         // 動きをセット

    cocos2d::Vec2 _WarpTo;  // ワープの宛先

public:
    static Block* create(BlockType pm_Type);	        // インスタンス生成
    virtual bool init(BlockType pm_Type);               // 初期化
};

#endif // __BLOCK__H__
