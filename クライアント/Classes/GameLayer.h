#ifndef __GAMELAYER_H__
#define __GAMELAYER_H__

#pragma once

#include "cocos2d.h"
#include "Common.h"
#include "Block.h"
#include "Ball.h"
#include "Background.h"
#include "TitleLayer.h"

#include "network/WebSocket.h"

USING_NS_CC;

// ゲームレイヤー
class GameLayer : public cocos2d::Layer,cocos2d::network::WebSocket::Delegate {
protected:
    // サーバにアクション情報を通知するメッセージ
    enum actionInfo {
        A_RoomIn,
        A_PlayerNumGet,
        A_StopSync,
        A_MoveSync,
        A_CollisionSync,
        A_Disconnect,
    };

    // プレイヤー情報
    struct Player {
        Vec2 velocity1;
        Vec2 position1;
        Vec2 velocity2;
        Vec2 position2;
        Vec2 velocity;
        Vec2 position;
        int  playerNum;
    };

    bool _turnAllow = true;         // ターン表示許可
    bool _touchAllow = false;       // タッチ許可
    bool _shotAfterSync = false;    // 停止後許可
    int  _myTurnNum = 0;            // ターン値
    int  _playerNum = -1;           // プレイヤー番号 
    int  _p1Hp = 100;               // プレイヤー1体力
    int  _p2Hp = 100;               // プレイヤー2体力
    cocos2d::network::WebSocket* _websocket = nullptr; // Websocketオブジェクト
    cocos2d::Vec2 _onTouchPosition; // タッチした位置

    void outcome(int pm_OutcomeNum); // 勝敗結果処理
    void disconnect();               // 切断処理
    void showSearchRoom();           // 部屋を検索テキストを表示
    void afterSearchRoom(float dt);  // サーバに検索要求通知
    void afterCreate(float dt);      // サーバから検索応答(正常)後処理
    void websocketInit();            // Websocket初期化
    void createBackground();         // 背景を生成
    void createPlayer();             // プレイヤーを生成
    void createBlock();              // ブロックを生成
    void setPlayerNum();             // プレイヤー番号を設定
    void showHp();                   // プレイヤー体力を表示
    void showTurn(int pm_MyTurn);    // ターン表示
    void showMyTurn();               // 自分のプレイヤーのアニメーション表示
    void sendMessage(int pm_ActionNum, Player pm_Player);   // メッセージをサーバに送信
    void receiveMessage(std::string data);                  // メッセージをサーバから受信
    void onHomeButtonCallback(cocos2d::Ref* pSender);       // ホームボタンコールバック関数
    void contactAnimation(PhysicsContact& pm_Contact, Vec2 pm_PlayerPos, int pm_AnimNumber); // 衝突アニメーション
    void myTurnAnimation(Vec2 pm_PlayerPos);                // 自分のターンアニメーション生成
    void battleJudge();         // 勝敗結果表示
    void update(float dt);      // フレーム処理
    void afterContactBegin(float dt, int argument); // 衝突後処理(サーバにプレイヤーの位置、ベクトル情報を同期要求)

    // Websocketイベント
    virtual void onOpen(cocos2d::network::WebSocket* ws);
    virtual void onMessage(cocos2d::network::WebSocket* ws, const cocos2d::network::WebSocket::Data& data);
    virtual void onClose(cocos2d::network::WebSocket* ws);
    virtual void onError(cocos2d::network::WebSocket* ws, const cocos2d::network::WebSocket::ErrorCode& error);

    // タップイベント
    virtual bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* unused_event);
    virtual void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* unused_event);
    virtual void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* unused_event);
    virtual void onTouchCancelled(cocos2d::Touch* touch, cocos2d::Event* unused_event);

    //衝突イベント
    bool onContactBegin(cocos2d::PhysicsContact& contact);

public:
    virtual void onEnter(); //レイヤー表示処理
    static cocos2d::Scene* createScene(); // シーンを生成
    virtual bool init();    // 初期化
    
    CREATE_FUNC(GameLayer);
};

#endif // __GAMELAYER_H__
