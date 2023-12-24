#ifndef __COMMON__H__
#define __COMMON__H__

#pragma execution_character_set("utf-8")

#include "cocos2d.h"

USING_NS_CC;

#define POSX Director::getInstance()->getWinSize().width / 2
#define POSY Director::getInstance()->getWinSize().height / 2

#define STAGENUMX 3
#define STAGENUMY 10

// 共通
namespace Common
{
    struct StageInfo {
        Vec2 position;
        int  blockType;
    };

    //タグ
    enum Tag {
        T_Background = 1,      // 背景
        T_Title,               // タイトル
        T_SearchRoom,          // 部屋を探していますテキスト
        T_NormalBlock,         // ノーマルブロック
        T_DamageBlock1,        // ダメージブロック1
        T_DamageBlock2,        // ダメージブロック2
        T_HealBlock,           // 回復ブロック
        T_PlayerDamageBlock1,  // プレイヤー1ダメージブロック
        T_PlayerDamageBlock2,  // プレイヤー2ダメージブロック
        T_MenuBack,            // ホームメニューボタン
        T_Tap,                 // タップ
        T_Turn,                // ターン
        T_Animation,           // アニメーション
        T_MyTurnAnimation,     // 自分のターンアニメーション
        T_Player1 = 500,       // プレイヤー1
        T_Player2,             // プレイヤー2
    };

    //Zオーダー
    enum ZOrder {
        Z_Background = 1,      // 背景
        Z_Title,               // タイトル
        Z_Block,               // ブロック
        Z_Tap,                 // タップ
        Z_MyTurnAnimation,     // 自分のターンアニメーション
        Z_Ball,                // ボール(プレイヤー)
        Z_Arrow,               // 矢印
        Z_Gage,                // ゲージ
        Z_Button,              // ボタン
        Z_Turn,                // ターン
        Z_Outcome,             // 勝敗結果
        Z_Animation,           // アニメーション
        Z_BackgroundBehind,    // 背景外枠
    };

    //種類
    enum BlockType {
        NormalBlock = 1,
        DamageBlock1,
        DamageBlock2,
        HealBlock,
        PlayerDamageBlock1,
        PlayerDamageBlock2,
    };

    enum PlayerNum {
        P_Num1 = 0,
        P_Num2
    };

    //レイヤー
    enum LayerType {
        L_Title = 1,
        L_Select,
        L_Bg1,
        L_Bg2,
        L_Bg3,
        L_Game,
    };

    //地面
    enum GroundType {
        G_Normal = 1,
        G_Ice,
        G_Bounce,
    };

    enum Sound {
        S_Title,
        S_Select,
        S_Game,
    };

    enum SoundEffect {
        SE_Button,
        SE_Contact,
        SE_BlockContact,
        SE_Damage,
        SE_Heal,
        SE_Judgement,
    };

    enum Text {
        TXT_Title1,
        TXT_RoomSearch,
        TXT_MyTurn,
        TXT_YourTurn,
        TXT_Disconnect,
    };

    enum Font {
        F_Google,
    };

    // サウンド
    class cSound {
    public:
        static char* getSoundName(int pm_BGM) {
            char* w_BGM[10] = { "audio/bgm.mp3" ,
                                "audio/selectbgm.mp3",
                                "audio/gamebgm.mp3" };

            return w_BGM[pm_BGM];
        }

        static char* getSEName(int pm_Sound) {
            char* w_Sound[10] = { "audio/button.mp3",
                                  "audio/contact.mp3",
                                  "audio/blockcontact.mp3",
                                  "audio/damage.mp3",
                                  "audio/heal.mp3",
                                  "audio/judgement.mp3"};

            return w_Sound[pm_Sound];
        }
    };

    // テキスト
    class cText {
    public:
        static char* getText(int pm_Text) {
            char* w_Text[10] = { "画面をタップ!",
                                 "相手を探しています",
                                 "自分のターンです",
                                 "相手のターンです",
                                 "相手との通信が切れました"};

            return w_Text[pm_Text];
        }

        static char* getFont(int pm_Font) {
            char* w_Font[10] = { "fonts/GoogleMoji.ttf" };

            return w_Font[pm_Font];
        }
    };

    // ボール位置
    class cBallPos {
    private:
        // プレイヤーの位置を取得
        static Vec2 getBallPos(float pm_PosIndexX, float pm_PosIndexY) {
            int w_BSize = 50;

            float pm_PosX = POSX + w_BSize * pm_PosIndexX;
            float pm_PosY = POSY + w_BSize * pm_PosIndexY;

            return Vec2(pm_PosX, pm_PosY);
        }
    public:
        // プレイヤー位置インデックスを取得
        static Vec2 getBallPos(int pm_PlayerNum) {

            Vec2 BallPos[30] = {
                Vec2(getBallPos(-8, -4)),   // プレイヤー1位置
                Vec2(getBallPos(8,  4)),   // プレイヤー2位置
            };

            return BallPos[pm_PlayerNum - 1];
        }
    };

    // ステージ情報
    class cStageInfo {
    private:
        // ブロック位置を取得
        static Vec2 getBlockPos(float pm_PosIndexX, float pm_PosIndexY) {
            int w_BSize = 50;

            float pm_PosX = POSX + w_BSize * pm_PosIndexX;
            float pm_PosY = POSY + w_BSize * pm_PosIndexY;

            return Vec2(pm_PosX, pm_PosY);
        }
        // 地面
        static std::vector<StageInfo> getStageGround(int pm_StageType, std::vector<StageInfo> pm_StageInfo) {
            //地面
            for (int i = 0; i < 23; i++) {
                if (0 == i % 2) {
                    pm_StageInfo.push_back({ Vec2(getBlockPos(i - 11, -6)) , DamageBlock1 });
                    pm_StageInfo.push_back({ Vec2(getBlockPos(i - 11, 6)) , DamageBlock2 });
                }
            }
            for (int i = 0; i < 11; i++){
                if (0 == i % 2) {
                    pm_StageInfo.push_back({ Vec2(getBlockPos(-11, 5 - i)) , DamageBlock2 });
                    pm_StageInfo.push_back({ Vec2(getBlockPos(11, 5 - i)) , DamageBlock1 });
                }
            }

            return pm_StageInfo;
        }
    public:
        // ステージのブロック情報を取得
        static std::vector<StageInfo> getStagePos(int pm_StageNumber) {

            std::vector<StageInfo> w_StageInfo;
            w_StageInfo.clear();

            switch (pm_StageNumber) {
                case 11:
                    w_StageInfo = cStageInfo::getStageGround(G_Normal, w_StageInfo);
                    w_StageInfo.push_back({ Vec2(getBlockPos(0, 0)) ,   HealBlock });
                    w_StageInfo.push_back({ Vec2(getBlockPos(0, 2)) ,   PlayerDamageBlock1 });
                    w_StageInfo.push_back({ Vec2(getBlockPos(0, 4)) ,   PlayerDamageBlock2 });
                    w_StageInfo.push_back({ Vec2(getBlockPos(0, -2)) ,  PlayerDamageBlock1 });
                    w_StageInfo.push_back({ Vec2(getBlockPos(0, -4)) ,  PlayerDamageBlock2 });
                    w_StageInfo.push_back({ Vec2(getBlockPos(5, 0)) ,   PlayerDamageBlock1 });
                    w_StageInfo.push_back({ Vec2(getBlockPos(7, 0)) ,   PlayerDamageBlock1 });
                    w_StageInfo.push_back({ Vec2(getBlockPos(-5, 0)) ,  PlayerDamageBlock2 });
                    w_StageInfo.push_back({ Vec2(getBlockPos(-7, 0)) ,  PlayerDamageBlock2 });
                    w_StageInfo.push_back({ Vec2(getBlockPos(5, 2)) ,   NormalBlock });
                    w_StageInfo.push_back({ Vec2(getBlockPos(-5, 2)) ,  HealBlock });
                    w_StageInfo.push_back({ Vec2(getBlockPos(5, -2)) ,  HealBlock });
                    w_StageInfo.push_back({ Vec2(getBlockPos(-5, -2)) , NormalBlock });
                    w_StageInfo.push_back({ Vec2(getBlockPos(8, -4)) ,  NormalBlock });
                    w_StageInfo.push_back({ Vec2(getBlockPos(9, -4)) ,  NormalBlock });
                    w_StageInfo.push_back({ Vec2(getBlockPos(9, -3)) ,  NormalBlock });
                    w_StageInfo.push_back({ Vec2(getBlockPos(-8, 4)) ,  NormalBlock });
                    w_StageInfo.push_back({ Vec2(getBlockPos(-9, 4)) ,  NormalBlock });
                    w_StageInfo.push_back({ Vec2(getBlockPos(-9, 3)) ,  NormalBlock });
                    break;
                case 12:
                    for (int i = 0; i < 5; i++) {
                        for (int j = 0; j < 3; j++) {
                            w_StageInfo.push_back({ Vec2(getBlockPos(i, j - 5)) , NormalBlock });
                        }
                    }
                    break;
                case 13:
                    for (int i = 0; i < 8; i++) {
                        for (int j = 0; j < i; j++) {
                            w_StageInfo.push_back({ Vec2(getBlockPos(i - 3, j - 5)) , NormalBlock });
                        }
                    }
                    break;
            }

            return w_StageInfo;
        }
    };
};

#endif // __COMMON__H__
