#include "GameLayer.h"
#include "../external/rapidjson/include/rapidjson/document.h"
#include "../external/rapidjson/include/rapidjson/writer.h"
#include "../external/rapidjson/include/rapidjson/stringbuffer.h"

#define WINSIZE Director::getInstance()->getWinSize()
USING_NS_CC;

using namespace CocosDenshion;

// デバッグ情報
#define P1POSTEXT_TAG 200       // プレイヤー1のPosition
#define P1VELTEXT_TAG 201       // プレイヤー1のVelocity
#define CONATEXT_TAG  202       // 衝突Aタグ
#define CONBTEXT_TAG  203       // 衝突Bタグ
#define RECTEXT_TAG   204       // Websocket受信タグ
#define PLAYERNUMTEXT_TAG 205   // プレイヤー番号タグ
#define HPTEXT_TAG 1200         // 体力タグ

// プレイヤー情報
#define PLAYER_SUM 2      // プレイヤー合計
#define PLAYER_TAG 500    // プレイヤーのタグ
#define HP_TAG     300    // プレイヤーのHPタグ
#define ARROW_TAG  20     // 矢印のタグ
#define MAXHP      100.0f // プレイヤー最大HP

// 衝突イベント
#define CONNUMBER1 1 // プレイヤー1とプレイヤー2の衝突時イベント
#define CONNUMBER2 2 // プレイヤー1とブロックの衝突時イベント
#define CONNUMBER3 3 // プレイヤー2とブロックの衝突時イベント

// 勝敗結果
#define OUTCOME1 0 // 勝利
#define OUTCOME2 1 // 敗北
#define OUTCOME3 2 // 引き分け

// アニメーション
#define ANIMNUM1 0 // ダメージアニメーション
#define ANIMNUM2 1 // 回復アニメーション
#define ANIMNUM3 2 // ブロック衝突アニメーション
#define ANIMNUM4 3 // プレイヤーダメージアニメーション

#define SearchRoomTextSize      50  // タイトルボタン大きさ

// シーン作成
Scene* GameLayer::createScene()
{
    auto w_Scene = Scene::createWithPhysics(); // シーンを生成する
    auto w_Layer = GameLayer::create();        // TitleLayerクラスのレイヤーを生成する
    w_Scene->addChild(w_Layer);                // シーンに対してレイヤーを追加する

    return w_Scene;
}

// 初期化処理
bool GameLayer::init()
{
    if ( !Layer::init() ) {
        return false;
    }

    // キャッシュ削除
    Director::getInstance()->getTextureCache()->removeAllTextures();

    // シングルタップイベント取得
    auto listener = EventListenerTouchOneByOne::create();

    // イベント関数の割り当て
    listener->onTouchBegan = CC_CALLBACK_2(GameLayer::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(GameLayer::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(GameLayer::onTouchEnded, this);
    listener->onTouchCancelled = CC_CALLBACK_2(GameLayer::onTouchCancelled, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 衝突イベントの取得
    auto w_ContactListener = EventListenerPhysicsContact::create();
    w_ContactListener->onContactBegin = CC_CALLBACK_1(GameLayer::onContactBegin, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(w_ContactListener, this);

    // Websocket接続
    websocketInit();

    return true;
}

// レイヤー初期化
void GameLayer::onEnter() {

    //レイヤー処理
    Layer::onEnter();

    // 物理演算設定
    auto w_Vect = Vect(0, 0);
    auto w_Scene = dynamic_cast<Scene*>(this->getParent());
    w_Scene->getPhysicsWorld()->setGravity(w_Vect);
    //w_Scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

    // 背景を生成
    createBackground();

    // 部屋を探す画面表示
    showSearchRoom();
}

// 背景を生成
void GameLayer::createBackground() {
    // 背景の後ろ設定
    auto w_BackBehind = Background::create(L_Game);
    w_BackBehind->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    w_BackBehind->setPosition(Vec2(WINSIZE / 2));
    addChild(w_BackBehind, Z_BackgroundBehind);

    // 背景をセット
    auto w_Background = Background::create(L_Bg1);
    w_Background->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    w_Background->setPosition(Point(WINSIZE / 2));
    addChild(w_Background, Z_Background, T_Background);
}

// 部屋を探す
void GameLayer::showSearchRoom() {
    // 部屋を探すのアニメーション(フェードイン・アウト)
    auto w_Delay   = DelayTime::create(1);
    auto w_FadeOut = FadeTo::create(1.0f, 0);
    auto w_FadeIn  = FadeTo::create(1.0f, 255);
    auto w_Seq     = Sequence::create(w_Delay, w_FadeOut, w_FadeIn, nullptr);
    auto w_Repeat  = RepeatForever::create(w_Seq);

    // 部屋を探すを表示
    auto w_StartButton = Label::createWithTTF(cText::getText(TXT_RoomSearch), cText::getFont(F_Google), SearchRoomTextSize);
    w_StartButton->setPosition(Vec2(WINSIZE / 2));
    w_StartButton->setTextColor(Color4B::WHITE);
    w_StartButton->runAction(w_Repeat);
    addChild(w_StartButton, Z_Title, T_SearchRoom);

    this->scheduleOnce(CC_CALLBACK_1(GameLayer::afterSearchRoom, this), 0.5f, "uniqueKey");
}

// 待機部屋に入ったことをサーバに通知
void GameLayer::afterSearchRoom(float dt) {
    _websocket->send("inRoom");
}

// Websocket初期化
void GameLayer::websocketInit() {
    // Websocket削除
    if (_websocket != nullptr) {
        return;
    }

    // Websocket生成
    if (_websocket == nullptr) {
        _websocket = new network::WebSocket();
    }

    // WebsocketURL
    std::string w_Port = "8080";
    std::string w_IpAddress = "54.199.187.202";
    std::string w_WebsocketDest = "ws://";
    w_WebsocketDest += w_IpAddress;
    w_WebsocketDest += ':';
    w_WebsocketDest += w_Port;

    // Websocket接続
    _websocket->init(*this, w_WebsocketDest);
}

// Websocket接続
void GameLayer::onOpen(cocos2d::network::WebSocket* ws) {
    CCLOG("Websocket Open");
}

// Websocket受信
void GameLayer::onMessage(cocos2d::network::WebSocket* ws, const cocos2d::network::WebSocket::Data& data) {
    receiveMessage(data.bytes);
}

// Websocket切断
void GameLayer::onClose(cocos2d::network::WebSocket* ws) {
    CCLOG("Websocket Close");
}

// Websocketエラー
void GameLayer::onError(cocos2d::network::WebSocket* ws, const cocos2d::network::WebSocket::ErrorCode& error) {
    CCLOG("Failure");
}

// メッセージ送信
void GameLayer::sendMessage(int pm_ActionNum, Player pm_Player) {

    // 背景始点X位置取得
    float w_StartPos = (WINSIZE.width - 1136.0f) / 2.0f;

    // JSON初期化
    rapidjson::Document w_Doc;
    rapidjson::Document::AllocatorType& w_Allocator = w_Doc.GetAllocator();
    w_Doc.SetObject();

    // JSONに値追加
    rapidjson::Value w_JsonValue;

    // アクションをセット
    switch (pm_ActionNum) {
        case A_StopSync:
            w_JsonValue.SetInt(A_StopSync); break;
        case A_PlayerNumGet:
        case A_MoveSync:
        case A_CollisionSync:
            w_JsonValue.SetInt(A_MoveSync); break;
    }
    w_Doc.AddMember("action", w_JsonValue, w_Allocator);

    // プレイヤー情報をセット
    switch (pm_ActionNum) {
        case A_StopSync:
            w_JsonValue.SetInt(_p1Hp);
            w_Doc.AddMember("hp1", w_JsonValue, w_Allocator);
            w_JsonValue.SetInt(_p2Hp);
            w_Doc.AddMember("hp2", w_JsonValue, w_Allocator);
        case A_PlayerNumGet:
        case A_MoveSync:
            w_JsonValue.SetString("player", w_Allocator);
            w_Doc.AddMember("name", w_JsonValue, w_Allocator);
            w_JsonValue.SetFloat(pm_Player.velocity1.x);
            w_Doc.AddMember("shot1X", w_JsonValue, w_Allocator);
            w_JsonValue.SetFloat(pm_Player.velocity1.y);
            w_Doc.AddMember("shot1Y", w_JsonValue, w_Allocator);
            w_JsonValue.SetFloat(pm_Player.position1.x - w_StartPos);
            w_Doc.AddMember("pos1X", w_JsonValue, w_Allocator);
            w_JsonValue.SetFloat(pm_Player.position1.y);
            w_Doc.AddMember("pos1Y", w_JsonValue, w_Allocator);
            w_JsonValue.SetFloat(pm_Player.velocity2.x);
            w_Doc.AddMember("shot2X", w_JsonValue, w_Allocator);
            w_JsonValue.SetFloat(pm_Player.velocity2.y);
            w_Doc.AddMember("shot2Y", w_JsonValue, w_Allocator);
            w_JsonValue.SetFloat(pm_Player.position2.x - w_StartPos);
            w_Doc.AddMember("pos2X", w_JsonValue, w_Allocator);
            w_JsonValue.SetFloat(pm_Player.position2.y);
            w_Doc.AddMember("pos2Y", w_JsonValue, w_Allocator);
            break;
        case A_CollisionSync:
            w_JsonValue.SetInt(A_CollisionSync);
            w_Doc.AddMember("action", w_JsonValue, w_Allocator);
            w_JsonValue.SetFloat(pm_Player.velocity.x);
            w_Doc.AddMember("shotX", w_JsonValue, w_Allocator);
            w_JsonValue.SetFloat(pm_Player.velocity.y);
            w_Doc.AddMember("shotY", w_JsonValue, w_Allocator);
            w_JsonValue.SetFloat(pm_Player.position.x - w_StartPos);
            w_Doc.AddMember("posX", w_JsonValue, w_Allocator);
            w_JsonValue.SetFloat(pm_Player.position.y);
            w_Doc.AddMember("posY", w_JsonValue, w_Allocator);
            w_JsonValue.SetInt(pm_Player.playerNum);
            w_Doc.AddMember("number", w_JsonValue, w_Allocator);
            break;
        default:
            break;
    }

    // JSONに値を登録
    rapidjson::StringBuffer w_Buffer;
    rapidjson::Writer<rapidjson::StringBuffer> w_Writer(w_Buffer);
    w_Doc.Accept(w_Writer);

    std::string w_JsonStr = w_Buffer.GetString();

    // WebsocketにJSON送信
    _websocket->send(w_JsonStr);
}

// Websocket受信メッセージ
void GameLayer::receiveMessage(std::string data) {
    // 背景始点位置計算
    Vec2 w_StartPos = Vec2((WINSIZE.width - 1136.0f) / 2.0f, 0.0f);

    // JSON初期化
    rapidjson::Document w_Doc;
    CCLOG("%s", data.c_str());
    w_Doc.Parse(data.c_str());

    int w_ActionNum = w_Doc["action"].GetFloat();
    int w_PlayerNum;

    // タグ取得
    auto w_Ball1 = getChildByTag(T_Player1);
    auto w_Ball2 = getChildByTag(T_Player2);

    // アニメーションを削除
    auto w_PlayerAnimation = getChildByTag(T_MyTurnAnimation);
    if (w_PlayerAnimation) {
        w_PlayerAnimation->setVisible(false);
        removeChild(w_PlayerAnimation, true);
    }

    Player w_Player;

    switch (w_ActionNum) {
        case A_RoomIn:
            // 部屋に入った後の処理
            this->scheduleOnce(CC_CALLBACK_1(GameLayer::afterCreate, this), 1.0f, "uniqueKey");
            return;
        case A_PlayerNumGet:
            // プレイヤー番号を取得
            _playerNum = w_Doc["player"].GetInt();
            showTurn(_myTurnNum);

            // プレイヤーの位置にアニメーションを表示
            showMyTurn();
            break;
        case A_StopSync:
            // ターン増加
            _myTurnNum++;
            _myTurnNum = _myTurnNum % 2;
            _p1Hp = w_Doc["hp1"].GetFloat();
            _p2Hp = w_Doc["hp2"].GetFloat();
            battleJudge();
            showHp();
            if (_turnAllow) {
                showTurn(_myTurnNum);
            }
        case A_MoveSync:
            // タッチを無効化
            _touchAllow = false;

            // JSON取得
            w_Player.velocity1 = Vec2(w_Doc["shot1X"].GetFloat(), w_Doc["shot1Y"].GetFloat());
            w_Player.position1 = Vec2(w_Doc["pos1X"].GetFloat(), w_Doc["pos1Y"].GetFloat());
            w_Player.velocity2 = Vec2(w_Doc["shot2X"].GetFloat(), w_Doc["shot2Y"].GetFloat());
            w_Player.position2 = Vec2(w_Doc["pos2X"].GetFloat(), w_Doc["pos2Y"].GetFloat());

            // ボールに力を加える
            w_Ball1->getPhysicsBody()->resetForces();
            w_Ball1->getPhysicsBody()->setVelocity(w_Player.velocity1);
            w_Ball1->setPosition(w_Player.position1 + w_StartPos);
            w_Ball2->getPhysicsBody()->resetForces();
            w_Ball2->getPhysicsBody()->setVelocity(w_Player.velocity2);
            w_Ball2->setPosition(w_Player.position2 + w_StartPos);
            break;
        case A_CollisionSync:
            w_PlayerNum = w_Doc["player"].GetFloat();
            w_Player.velocity = Vec2(w_Doc["shotX"].GetFloat(), w_Doc["shotY"].GetFloat());
            w_Player.position = Vec2(w_Doc["posX"].GetFloat(), w_Doc["posY"].GetFloat());

            if (w_PlayerNum == 0) {
                // ボールに力を加える
                w_Ball1->getPhysicsBody()->resetForces();
                w_Ball1->getPhysicsBody()->setVelocity(w_Player.velocity);
                w_Ball1->setPosition(w_Player.position + w_StartPos);
            } else {
                // ボールに力を加える
                w_Ball2->getPhysicsBody()->resetForces();
                w_Ball2->getPhysicsBody()->setVelocity(w_Player.velocity);
                w_Ball2->setPosition(w_Player.position + w_StartPos);
            }
            break;
        case A_Disconnect:
            if (_turnAllow == true) {
                // 切断
                disconnect();
            }
            break;
    }

    // 自分のプレイヤーの位置にアニメーション表示
    if (A_StopSync == w_ActionNum) {
        showMyTurn();
    }

    _shotAfterSync = true;

    /**************** デバッグ ****************/
    // 受信テキスト
    /*auto w_PlayerText = (Label*)getChildByTag(PLAYERNUMTEXT_TAG);
    if (!w_PlayerText) {
        float w_BgPosX = 200;
        float w_BgPosY = 300;
        w_PlayerText = Label::createWithTTF("aa", "fonts/arial.ttf", 20);
        w_PlayerText->setPosition(Point(w_BgPosX, w_BgPosY));
        w_PlayerText->setTextColor(Color4B::BLACK);
        addChild(w_PlayerText, 110, PLAYERNUMTEXT_TAG);
    }

    // 受信テキストの更新
    w_PlayerText->setString(StringUtils::format("PlayerNum : %d", _playerNum));

    // 受信テキスト
    auto w_RecText = (Label*)getChildByTag(RECTEXT_TAG);
    if (!w_RecText) {
        float w_BgPosX = 200;
        float w_BgPosY = 250;
        w_RecText = Label::createWithTTF("aa", "fonts/arial.ttf", 20);
        w_RecText->setPosition(Point(w_BgPosX, w_BgPosY));
        w_RecText->setTextColor(Color4B::BLACK);
        addChild(w_RecText, 110, RECTEXT_TAG);
    }

    // 受信テキストの更新
    w_RecText->setString(StringUtils::format("WebRecieve : %f %f", w_Ball1->getPhysicsBody()->getVelocity().x,
        w_Ball1->getPhysicsBody()->getVelocity().y));
    /**************** デバッグ ****************/

}

// 部屋に入った後の処理
void GameLayer::afterCreate(float dt) {
    // 部屋を探していますを非表示
    auto w_SearchRoomText = getChildByTag(T_SearchRoom);
    w_SearchRoomText->setVisible(false);

    // プレイヤーを生成
    createPlayer();

    // 障害物を生成
    createBlock();

    // プレイヤー番号をセット
    setPlayerNum();

    // 体力表示
    showHp();
}

// プレイヤーを生成
void GameLayer::createPlayer() {
    // ボールをセット
    Ball* w_Ball;
    for (int i = 1; i <= PLAYER_SUM; i++) {
        w_Ball = Ball::create(i);
        w_Ball->setPosition(cBallPos::getBallPos(i));
        w_Ball->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
        addChild(w_Ball, Z_Ball, i + PLAYER_TAG - 1);
    }
}

// 障害物を生成
void GameLayer::createBlock() {
    // ステージ情報を取得
    std::vector<StageInfo> w_StageInfo;
    w_StageInfo = cStageInfo::getStagePos(11);

    // ブロックをセット
    for (int i = 0; i < w_StageInfo.size(); i++) {
        auto w_Block = Block::create((BlockType)w_StageInfo[i].blockType);
        w_Block->setPosition(w_StageInfo[i].position);
        addChild(w_Block, Z_Block);
    }
}

// プレイヤー番号をセット
void GameLayer::setPlayerNum() {
    // JSON変数を宣言
    rapidjson::Document w_Doc;
    rapidjson::Document::AllocatorType& w_Allocator = w_Doc.GetAllocator();
    w_Doc.SetObject();

    // JSONに値追加
    rapidjson::Value w_JsonValue;
    w_JsonValue.SetInt(A_PlayerNumGet);
    w_Doc.AddMember("action", w_JsonValue, w_Allocator);

    // JSONに値を登録
    rapidjson::StringBuffer w_Buffer;
    rapidjson::Writer<rapidjson::StringBuffer> w_Writer(w_Buffer);
    w_Doc.Accept(w_Writer);

    std::string w_JsonStr = w_Buffer.GetString();

    // WebsocketにJSON送信
    _websocket->send(w_JsonStr);
}

// 体力表示
void GameLayer::showHp() {
    // 体力の上限下限
    if (_p1Hp > MAXHP) {
        _p1Hp = MAXHP;
    }else if (_p1Hp <= 0) {
        _p1Hp = 0;
    }

    if (_p2Hp > MAXHP) {
        _p2Hp = MAXHP;
    }else if (_p2Hp <= 0) {
        _p2Hp = 0;
    }

    Node* w_HpGage1;
    w_HpGage1 = getChildByTag(PLAYER_TAG);
    w_HpGage1->getChildByTag(HP_TAG)->setScaleX(_p1Hp / MAXHP);

    Node* w_HpGage2;
    w_HpGage2 = getChildByTag(1 + PLAYER_TAG);
    w_HpGage2->getChildByTag(1 + HP_TAG)->setScaleX(_p2Hp / MAXHP);

    /**************** デバッグ ****************/
    // プレイヤー1の位置
    /*auto w_P1PosText = (Label*)getChildByTag(HPTEXT_TAG);
    if (!w_P1PosText) {
        float w_BgPosX = 200;
        float w_BgPosY = 350;
        w_P1PosText = Label::createWithTTF("aa", "fonts/arial.ttf", 20);
        w_P1PosText->setPosition(Point(w_BgPosX, w_BgPosY));
        w_P1PosText->setTextColor(Color4B::BLACK);
        addChild(w_P1PosText, 120, HPTEXT_TAG);
    }
    w_P1PosText->setString(StringUtils::format("P1Pos : %d P2Pos : %d", _p1Hp, _p2Hp));
    /**************** デバッグ ****************/
}

// バトルの結果
void GameLayer::battleJudge() {
    // 結果
    if (_p1Hp <= 0 && _p2Hp <= 0) {
        outcome(OUTCOME3);
    } else if (_p1Hp <= 0 && _playerNum == 0) {
        outcome(OUTCOME1);
    } else if (_p1Hp <= 0 && _playerNum == 1) {
        outcome(OUTCOME2);
    } else if (_p2Hp <= 0 && _playerNum == 0) {
        outcome(OUTCOME2);
    } else if (_p2Hp <= 0 && _playerNum == 1) {
        outcome(OUTCOME1);
    }
}

// 勝敗結果
void GameLayer::outcome(int pm_OutcomeNum) {
    std::string w_OutcomeStr;
    if (pm_OutcomeNum == 0) {
        w_OutcomeStr = "あなたの負けです。";
    } else if (pm_OutcomeNum == 1) {
        w_OutcomeStr = "あなたの勝ちです。";
    } else {
        w_OutcomeStr = "引き分けです。";
    }

    // 音楽再生
    auto w_Audio = SimpleAudioEngine::getInstance();
    w_Audio->playEffect(cSound::getSEName(SE_Judgement));

    // スタートボタンのアニメーション(フェードイン・アウト)
    auto w_Delay = DelayTime::create(1);
    auto w_FadeOut = FadeTo::create(1.0f, 0);
    auto w_FadeIn = FadeTo::create(1.0f, 255);
    auto w_Seq = Sequence::create(w_Delay, w_FadeOut, w_FadeIn, nullptr);
    auto w_Repeat = RepeatForever::create(w_Seq);

    // スタートボタンを表示
    auto w_StartButton = Label::createWithTTF(w_OutcomeStr, cText::getFont(F_Google), SearchRoomTextSize);
    w_StartButton->setPosition(Vec2(WINSIZE / 2));
    w_StartButton->setTextColor(Color4B::WHITE);
    w_StartButton->runAction(w_Repeat);
    addChild(w_StartButton, Z_Outcome);

    // ゲームクリアのボタンを表示
    auto w_ClearButton = (MenuItemImage*)getChildByTag(T_MenuBack);
    if (!w_ClearButton) {
        auto w_BeforeButton = "home_be.png";
        auto w_AfterButton = "home_af.png";

        w_ClearButton = MenuItemImage::create(
                w_BeforeButton,
                w_AfterButton,
                CC_CALLBACK_1(GameLayer::onHomeButtonCallback, this)
        );
        w_ClearButton->setAnchorPoint(Point::ANCHOR_MIDDLE);
        w_ClearButton->setPosition(Point(WINSIZE.width / 2, WINSIZE.height / 2 - 100));

        Menu* w_Menu = Menu::create(w_ClearButton, NULL);
        w_Menu->setPosition(Vec2::ZERO);
        addChild(w_Menu, Z_Button);
    }

    // ターン変更フラグ & タッチ許可フラグ 無効
    _turnAllow = false;
    _touchAllow = false;
}

// フレーム処理
void GameLayer::update(float dt) {
    // プレイヤー1の速度が基準値より低くなった場合停止
    auto w_Player1 = getChildByTag(T_Player1);
    if (abs(w_Player1->getPhysicsBody()->getVelocity().x) < 10.0f &&
        abs(w_Player1->getPhysicsBody()->getVelocity().y) < 10.0f) {
        w_Player1->getPhysicsBody()->setVelocity(Vec2(0, 0));
    }

    // プレイヤー2の速度が基準値より低くなった場合停止
    auto w_Player2 = getChildByTag(T_Player2);
    if (abs(w_Player2->getPhysicsBody()->getVelocity().x) < 10.0f &&
        abs(w_Player2->getPhysicsBody()->getVelocity().y) < 10.0f) {
        w_Player2->getPhysicsBody()->setVelocity(Vec2(0, 0));
    }

    // 停止したら同期
    if (_shotAfterSync == true &&
        w_Player1->getPhysicsBody()->getVelocity().x == 0.0f &&
        w_Player1->getPhysicsBody()->getVelocity().y == 0.0f &&
        w_Player2->getPhysicsBody()->getVelocity().x == 0.0f &&
        w_Player2->getPhysicsBody()->getVelocity().y == 0.0f) {

        Player w_Player;
        w_Player.position1 = w_Player1->getPosition();
        w_Player.velocity1 = w_Player1->getPhysicsBody()->getVelocity();
        w_Player.position2 = w_Player2->getPosition();
        w_Player.velocity2 = w_Player2->getPhysicsBody()->getVelocity();

        unscheduleUpdate();

        CCLOG("aaaa");

        sendMessage(A_StopSync, w_Player);
    }

    /**************** デバッグ ****************/
    // プレイヤー1の位置
    /*auto w_P1PosText = (Label*)getChildByTag(P1POSTEXT_TAG);
    if (!w_P1PosText) {
        float w_BgPosX = 200;
        float w_BgPosY = 50;
        w_P1PosText = Label::createWithTTF("aa", "fonts/arial.ttf", 20);
        w_P1PosText->setPosition(Point(w_BgPosX, w_BgPosY));
        w_P1PosText->setTextColor(Color4B::BLACK);
        addChild(w_P1PosText, 120, P1POSTEXT_TAG);
    }

    // プレイヤー1のVelocity
    auto w_P1VelText = (Label*)getChildByTag(P1VELTEXT_TAG);
    if (!w_P1VelText) {
        float w_BgPosX = 200;
        float w_BgPosY = 100;
        w_P1VelText = Label::createWithTTF("aa", "fonts/arial.ttf", 20);
        w_P1VelText->setPosition(Point(w_BgPosX, w_BgPosY));
        w_P1VelText->setTextColor(Color4B::BLACK);
        addChild(w_P1VelText, 130, P1VELTEXT_TAG);
    }
    w_P1PosText->setString(StringUtils::format("P1Pos : %f %f", w_Player1->getPosition().x, w_Player1->getPosition().y));
    w_P1VelText->setString(StringUtils::format("P1Vel : %f %f", w_Player1->getPhysicsBody()->getVelocity().x, w_Player1->getPhysicsBody()->getVelocity().y));
    */
    /**************** デバッグ ****************/
}

// 衝突イベント
bool GameLayer::onContactBegin(PhysicsContact& pm_Contact) {
    // 効果音変数を宣言
    auto w_Audio = SimpleAudioEngine::getInstance();

    // タグの取得
    auto w_BodyTagA = pm_Contact.getShapeA()->getBody()->getNode()->getTag();
    auto w_BodyTagB = pm_Contact.getShapeB()->getBody()->getNode()->getTag();

    // 物理演算の取得
    auto w_BodyA = pm_Contact.getShapeA()->getBody();
    auto w_BodyB = pm_Contact.getShapeB()->getBody();

    // プレイヤーの位置
    auto w_PlayerPos = Vec2(0, 0);

    // プレイヤー同士の衝突(同期処理)
    if (( w_BodyTagA == T_Player1 && w_BodyTagB == T_Player2 ) ||
        ( w_BodyTagA == T_Player2 && w_BodyTagB == T_Player1 )) {
        w_Audio->playEffect(cSound::getSEName(SE_Contact));
        contactAnimation(pm_Contact, w_PlayerPos, ANIMNUM2);
        this->scheduleOnce(CC_CALLBACK_1(GameLayer::afterContactBegin, this, CONNUMBER1), 0.0001f, "uniqueKey");
    } else if (( w_BodyTagA == T_Player1      && w_BodyTagB == T_NormalBlock ) ||
               ( w_BodyTagA == T_NormalBlock  && w_BodyTagB == T_Player1 )) {
        w_Audio->playEffect(cSound::getSEName(SE_BlockContact));
        contactAnimation(pm_Contact, w_PlayerPos, ANIMNUM3);
        this->scheduleOnce(CC_CALLBACK_1(GameLayer::afterContactBegin, this, CONNUMBER2), 0.0001f, "uniqueKey");
    } else if (( w_BodyTagA == T_Player2      && w_BodyTagB == T_NormalBlock ) ||
               ( w_BodyTagA == T_NormalBlock  && w_BodyTagB == T_Player2 )) {
        w_Audio->playEffect(cSound::getSEName(SE_BlockContact));
        contactAnimation(pm_Contact, w_PlayerPos, ANIMNUM3);
        this->scheduleOnce(CC_CALLBACK_1(GameLayer::afterContactBegin, this, CONNUMBER3), 0.0001f, "uniqueKey");
    } else if ((w_BodyTagA == T_Player1 && w_BodyTagB == T_DamageBlock1) ||
               (w_BodyTagA == T_DamageBlock1 && w_BodyTagB == T_Player1)) {
        w_Audio->playEffect(cSound::getSEName(SE_Damage));
        contactAnimation(pm_Contact, w_PlayerPos, ANIMNUM1);
        _p1Hp = _p1Hp - 1;
        this->scheduleOnce(CC_CALLBACK_1(GameLayer::afterContactBegin, this, CONNUMBER2), 0.0001f, "uniqueKey");
    } else if ((w_BodyTagA == T_Player2 && w_BodyTagB == T_DamageBlock1) ||
               (w_BodyTagA == T_DamageBlock1 && w_BodyTagB == T_Player2)) {
        w_Audio->playEffect(cSound::getSEName(SE_Damage));
        contactAnimation(pm_Contact, w_PlayerPos, ANIMNUM1);
        _p2Hp = _p2Hp - 1;
        this->scheduleOnce(CC_CALLBACK_1(GameLayer::afterContactBegin, this, CONNUMBER3), 0.0001f, "uniqueKey");
    } else if ((w_BodyTagA == T_Player1 && w_BodyTagB == T_DamageBlock2) ||
               (w_BodyTagA == T_DamageBlock2 && w_BodyTagB == T_Player1)) {
        w_Audio->playEffect(cSound::getSEName(SE_Damage));
        contactAnimation(pm_Contact, w_PlayerPos, ANIMNUM1);
        _p1Hp = _p1Hp - 3;
        this->scheduleOnce(CC_CALLBACK_1(GameLayer::afterContactBegin, this, CONNUMBER2), 0.0001f, "uniqueKey");
    } else if ((w_BodyTagA == T_Player2 && w_BodyTagB == T_DamageBlock2) ||
               (w_BodyTagA == T_DamageBlock2 && w_BodyTagB == T_Player2)) {
        w_Audio->playEffect(cSound::getSEName(SE_Damage));
        contactAnimation(pm_Contact, w_PlayerPos, ANIMNUM1);
        _p2Hp = _p2Hp - 3;
        this->scheduleOnce(CC_CALLBACK_1(GameLayer::afterContactBegin, this, CONNUMBER3), 0.0001f, "uniqueKey");
    } else if ((w_BodyTagA == T_Player1 && w_BodyTagB == T_HealBlock) ||
               (w_BodyTagA == T_HealBlock && w_BodyTagB == T_Player1)) {
        w_Audio->playEffect(cSound::getSEName(SE_Heal));
        contactAnimation(pm_Contact, w_PlayerPos, ANIMNUM2);
        _p1Hp = _p1Hp + 5;
        this->scheduleOnce(CC_CALLBACK_1(GameLayer::afterContactBegin, this, CONNUMBER2), 0.0001f, "uniqueKey");
    } else if ((w_BodyTagA == T_Player2 && w_BodyTagB == T_HealBlock) ||
               (w_BodyTagA == T_HealBlock && w_BodyTagB == T_Player2)) {
        w_Audio->playEffect(cSound::getSEName(SE_Heal));
        contactAnimation(pm_Contact, w_PlayerPos, ANIMNUM2);
        _p2Hp = _p2Hp + 5;
        this->scheduleOnce(CC_CALLBACK_1(GameLayer::afterContactBegin, this, CONNUMBER3), 0.0001f, "uniqueKey");
    } else if ((w_BodyTagA == T_Player2            && w_BodyTagB == T_PlayerDamageBlock1) ||
               (w_BodyTagA == T_PlayerDamageBlock1 && w_BodyTagB == T_Player2)) {
        w_PlayerPos = getChildByTag(T_Player1)->getPosition();
        contactAnimation(pm_Contact, w_PlayerPos, ANIMNUM4);
        w_Audio->playEffect(cSound::getSEName(SE_Damage));
        _p1Hp = _p1Hp - 3;
        this->scheduleOnce(CC_CALLBACK_1(GameLayer::afterContactBegin, this, CONNUMBER3), 0.0001f, "uniqueKey");
    } else if ((w_BodyTagA == T_Player1            && w_BodyTagB == T_PlayerDamageBlock2) ||
               (w_BodyTagA == T_PlayerDamageBlock2 && w_BodyTagB == T_Player1)) {
        w_PlayerPos = getChildByTag(T_Player2)->getPosition();
        contactAnimation(pm_Contact, w_PlayerPos, ANIMNUM4);
        w_Audio->playEffect(cSound::getSEName(SE_Damage));
        _p2Hp = _p2Hp - 3;
        this->scheduleOnce(CC_CALLBACK_1(GameLayer::afterContactBegin, this, CONNUMBER3), 0.0001f, "uniqueKey");
    }

    // 体力表示
    showHp();

    /**************** デバッグ ****************/
    // 衝突Aテキスト
    /*auto w_ConAText = (Label*)getChildByTag(CONATEXT_TAG);
    if (!w_ConAText) {
        float w_BgPosX = 200;
        float w_BgPosY = 150;
        w_ConAText = Label::createWithTTF("aa", "fonts/arial.ttf", 20);
        w_ConAText->setPosition(Point(w_BgPosX, w_BgPosY));
        w_ConAText->setTextColor(Color4B::BLACK);
        addChild(w_ConAText, 140, CONATEXT_TAG);
    }

    // 衝突Bテキスト
    auto w_ConBText = (Label*)getChildByTag(CONBTEXT_TAG);
    if (!w_ConBText) {
        float w_BgPosX = 200;
        float w_BgPosY = 200;
        w_ConBText = Label::createWithTTF("aa", "fonts/arial.ttf", 20);
        w_ConBText->setPosition(Point(w_BgPosX, w_BgPosY));
        w_ConBText->setTextColor(Color4B::BLACK);
        addChild(w_ConBText, 150, CONBTEXT_TAG);
    }

    CCLOG("A = %d", w_BodyA->getTag());
    CCLOG("B = %d", w_BodyB->getTag());

    // ショットカウントラベルの更新
    w_ConAText->setString(StringUtils::format("ConAPos : %f %f", w_BodyA->getPosition().x, w_BodyA->getPosition().y));
    w_ConBText->setString(StringUtils::format("ConBPos : %f %f", w_BodyB->getPosition().x, w_BodyB->getPosition().y));
    /**************** デバッグ ****************/

    return true;
}

// 衝突アニメーション
void GameLayer::contactAnimation(PhysicsContact& pm_Contact, Vec2 pm_PlayerPos, int pm_AnimNumber) {
    // アニメーション用スプライトを生成
    auto w_Sprite = Sprite::create("animation/null.png");
    if (pm_AnimNumber == ANIMNUM4) {
        w_Sprite->setPosition(pm_PlayerPos);
    } else {
        w_Sprite->setPosition(pm_Contact.getContactData()->points[0]);
    }

    // アニメーションを生成
    auto w_Animation = Animation::create();
    switch (pm_AnimNumber) {
        case ANIMNUM1:
        case ANIMNUM4:
            w_Animation->addSpriteFrameWithFile("animation/Damage-0.png");
            w_Animation->addSpriteFrameWithFile("animation/Damage-1.png");
            w_Animation->addSpriteFrameWithFile("animation/Damage-2.png");
            w_Animation->addSpriteFrameWithFile("animation/Damage-3.png");
            w_Animation->addSpriteFrameWithFile("animation/Damage-4.png");
            w_Animation->addSpriteFrameWithFile("animation/Damage-5.png");
            w_Animation->addSpriteFrameWithFile("animation/Damage-6.png");
            w_Animation->addSpriteFrameWithFile("animation/Damage-7.png");
            w_Animation->addSpriteFrameWithFile("animation/Damage-8.png");
            w_Animation->addSpriteFrameWithFile("animation/Damage-9.png");
            break;
        case ANIMNUM2:
            w_Animation->addSpriteFrameWithFile("animation/Heal-00.png");
            w_Animation->addSpriteFrameWithFile("animation/Heal-01.png");
            w_Animation->addSpriteFrameWithFile("animation/Heal-02.png");
            w_Animation->addSpriteFrameWithFile("animation/Heal-03.png");
            w_Animation->addSpriteFrameWithFile("animation/Heal-04.png");
            w_Animation->addSpriteFrameWithFile("animation/Heal-05.png");
            w_Animation->addSpriteFrameWithFile("animation/Heal-06.png");
            w_Animation->addSpriteFrameWithFile("animation/Heal-07.png");
            w_Animation->addSpriteFrameWithFile("animation/Heal-08.png");
            w_Animation->addSpriteFrameWithFile("animation/Heal-09.png");
            w_Animation->addSpriteFrameWithFile("animation/Heal-10.png");
            w_Animation->addSpriteFrameWithFile("animation/Heal-11.png");
            w_Animation->addSpriteFrameWithFile("animation/Heal-12.png");
            w_Animation->addSpriteFrameWithFile("animation/Heal-13.png");
            w_Animation->addSpriteFrameWithFile("animation/Heal-14.png");
            w_Animation->addSpriteFrameWithFile("animation/Heal-15.png");
            w_Animation->addSpriteFrameWithFile("animation/Heal-16.png");
            w_Animation->addSpriteFrameWithFile("animation/Heal-17.png");
            break;
        case ANIMNUM3:
            w_Animation->addSpriteFrameWithFile("animation/Blockcontact-0.png");
            w_Animation->addSpriteFrameWithFile("animation/Blockcontact-1.png");
            w_Animation->addSpriteFrameWithFile("animation/Blockcontact-2.png");
            w_Animation->addSpriteFrameWithFile("animation/Blockcontact-3.png");
            w_Animation->addSpriteFrameWithFile("animation/Blockcontact-4.png");
            w_Animation->addSpriteFrameWithFile("animation/Blockcontact-5.png");
            break;
    }
    w_Animation->setDelayPerUnit(0.05f); // フレームごとの遅延時間

    // アニメーション終了後、オブジェクトを削除
    auto w_Func = CallFunc::create([&]() {
        auto w_TapAnim = getChildByTag(T_Animation);
        removeChild(w_TapAnim, true);
    });

    // アニメーションを表示
    auto w_Animate = Sequence::create(Animate::create(w_Animation), w_Func, nullptr);
    w_Sprite->runAction(w_Animate);
    addChild(w_Sprite, Z_Animation, T_Animation);
}

void GameLayer::showMyTurn() {
    // ボール位置にアニメーション表示
    if (_myTurnNum == 0) {
        auto w_PlayerPos = getChildByTag(T_Player1)->getPosition();
        myTurnAnimation(w_PlayerPos);
    }
    else {
        auto w_PlayerPos = getChildByTag(T_Player2)->getPosition();
        myTurnAnimation(w_PlayerPos);
    }
}

// 衝突アニメーション
void GameLayer::myTurnAnimation(Vec2 pm_PlayerPos) {

    // アニメーション用スプライトを生成
    auto w_Sprite = Sprite::create("animation/null.png");
    w_Sprite->setPosition(pm_PlayerPos);

    // アニメーションを生成
    auto w_Animation = Animation::create();
    w_Animation->addSpriteFrameWithFile("animation/myturn-00.png");
    w_Animation->addSpriteFrameWithFile("animation/myturn-01.png");
    w_Animation->addSpriteFrameWithFile("animation/myturn-02.png");
    w_Animation->addSpriteFrameWithFile("animation/myturn-03.png");
    w_Animation->addSpriteFrameWithFile("animation/myturn-04.png");
    w_Animation->addSpriteFrameWithFile("animation/myturn-05.png");
    w_Animation->addSpriteFrameWithFile("animation/myturn-06.png");
    w_Animation->addSpriteFrameWithFile("animation/myturn-07.png");
    w_Animation->addSpriteFrameWithFile("animation/myturn-08.png");
    w_Animation->addSpriteFrameWithFile("animation/myturn-09.png");
    w_Animation->addSpriteFrameWithFile("animation/myturn-10.png");
    w_Animation->addSpriteFrameWithFile("animation/myturn-11.png");
    w_Animation->addSpriteFrameWithFile("animation/myturn-12.png");
    w_Animation->addSpriteFrameWithFile("animation/myturn-13.png");
    w_Animation->addSpriteFrameWithFile("animation/myturn-14.png");
    w_Animation->addSpriteFrameWithFile("animation/myturn-15.png");
    w_Animation->addSpriteFrameWithFile("animation/myturn-16.png");
    w_Animation->addSpriteFrameWithFile("animation/myturn-17.png");
    w_Animation->addSpriteFrameWithFile("animation/myturn-18.png");
    w_Animation->setDelayPerUnit(0.05f); // フレームごとの遅延時間

    // アニメーション終了後、オブジェクトを削除
    //auto w_TapAnim = getChildByTag(T_Animation);
    //removeChild(w_TapAnim, true);

    // アニメーションを表示
    auto w_Animate = RepeatForever::create(Animate::create(w_Animation));
    w_Sprite->runAction(w_Animate);
    addChild(w_Sprite, Z_MyTurnAnimation, T_MyTurnAnimation);
}

// 衝突後処理
void GameLayer::afterContactBegin(float dt, int pm_Argument)
{
    // プレイヤーオブジェクト取得
    Node* w_Player1;
    Node* w_Player2;

    // プレイヤーの状態取得
    Player w_Player;

    switch (pm_Argument) {
        case CONNUMBER1:
            // プレイヤーオブジェクト取得
            w_Player1 = getChildByTag(T_Player1);
            w_Player2 = getChildByTag(T_Player2);

            // プレイヤーの状態取得
            w_Player.velocity1 = w_Player1->getPhysicsBody()->getVelocity();
            w_Player.position1 = w_Player1->getPosition();
            w_Player.velocity2 = w_Player2->getPhysicsBody()->getVelocity();
            w_Player.position2 = w_Player2->getPosition();

            // Websocket送信
            sendMessage(A_MoveSync, w_Player);
            CCLOG("x = %f, y = %f", w_Player1->getPhysicsBody()->getVelocity().x, w_Player1->getPhysicsBody()->getVelocity().y);
            break;
        case CONNUMBER2:
            // プレイヤーオブジェクト取得
            w_Player1 = getChildByTag(T_Player1);

            // プレイヤーの状態取得
            w_Player.playerNum = 0;
            w_Player.velocity = w_Player1->getPhysicsBody()->getVelocity();
            w_Player.position = w_Player1->getPosition();

            // Websocket送信
            sendMessage(A_CollisionSync, w_Player);
            break;
        case CONNUMBER3:
            // プレイヤーオブジェクト取得
            w_Player2 = getChildByTag(T_Player2);

            // プレイヤーの状態取得
            w_Player.playerNum = 1;
            w_Player.velocity = w_Player2->getPhysicsBody()->getVelocity();
            w_Player.position = w_Player2->getPosition();

            // Websocket送信
            sendMessage(A_CollisionSync, w_Player);
            break;
    }

}

// 切断処理
void GameLayer::disconnect() {
    // 切断テキストのアニメーション変数
    auto w_Delay   = DelayTime::create(1);
    auto w_FadeOut = FadeTo::create(1.0f, 0);
    auto w_FadeIn  = FadeTo::create(1.0f, 255);
    auto w_Seq     = Sequence::create(w_Delay, w_FadeOut, w_FadeIn, nullptr);
    auto w_Repeat  = RepeatForever::create(w_Seq);

    // 切断テキストを表示
    auto w_StartButton = Label::createWithTTF(cText::getText(TXT_Disconnect), cText::getFont(F_Google), SearchRoomTextSize);
    w_StartButton->setPosition(Vec2(WINSIZE / 2));
    w_StartButton->setTextColor(Color4B::WHITE);
    w_StartButton->runAction(w_Repeat);
    addChild(w_StartButton, Z_Button, T_SearchRoom);

    // ホームボタンを表示
    auto w_HomeButton = (MenuItemImage*)getChildByTag(T_MenuBack);
    if (!w_HomeButton) {
        auto w_BeforeButton = "home_be.png";
        auto w_AfterButton = "home_af.png";

        w_HomeButton = MenuItemImage::create(
                w_BeforeButton,
                w_AfterButton,
                CC_CALLBACK_1(GameLayer::onHomeButtonCallback, this)
        );
        w_HomeButton->setAnchorPoint(Point::ANCHOR_MIDDLE);
        w_HomeButton->setPosition(Point(WINSIZE.width / 2, WINSIZE.height / 2 - 100));

        Menu* w_Menu = Menu::create(w_HomeButton, NULL);
        w_Menu->setPosition(Vec2::ZERO);
        addChild(w_Menu, Z_Button);
    }

    _turnAllow = false;
    _touchAllow = false;
}

// ターン表示
void GameLayer::showTurn(int pm_MyTurn) {

    Label* w_StartButton;

    // 自分のターンと同じか
    if (_playerNum == pm_MyTurn) {
        // 自分のターンを表示
        w_StartButton = Label::createWithTTF(cText::getText(TXT_MyTurn), cText::getFont(F_Google), SearchRoomTextSize);
        w_StartButton->setTextColor(Color4B::BLUE);
    } else {
        // 相手のターンを表示
        w_StartButton = Label::createWithTTF(cText::getText(TXT_YourTurn), cText::getFont(F_Google), SearchRoomTextSize);
        w_StartButton->setTextColor(Color4B::RED);
    }

    // スタートボタンのアニメーション(フェードイン・アウト)
    auto w_Delay = DelayTime::create(0.5f);
    auto w_FadeIn = FadeTo::create(0.5f, 255);
    auto w_FadeOut = FadeTo::create(0.5f, 0);
    auto w_Func = CallFunc::create([&]() {
        _touchAllow = true;
        removeChild(getChildByTag(T_Turn));
    });
    auto w_Seq = Sequence::create(w_Delay, w_FadeIn, w_FadeOut, w_Func, nullptr);

    // ターンを表示
    w_StartButton->setPosition(Vec2(WINSIZE / 2));
    w_StartButton->runAction(w_Seq);
    addChild(w_StartButton, Z_Turn, T_Turn);

}

// ホームボタンを押下時処理
void GameLayer::onHomeButtonCallback(Ref* pm_pSender) {
    _websocket->close();
    // 選択画面シーンへ移動
    auto w_Scene = TitleLayer::createScene();
    auto w_Tran = TransitionFade::create(1, w_Scene);
    Director::getInstance()->replaceScene(w_Tran);
}

// タップ
bool GameLayer::onTouchBegan(Touch* pm_Touch, Event* unused_event) {
    // タッチポジションを保持
    _onTouchPosition = pm_Touch->getLocation();
    CCLOG("x = %f y = %f",_onTouchPosition.x, _onTouchPosition.y);

    // プレイヤー番号と自分のターン番号が同じであるか
    if (_playerNum  == _myTurnNum &&
        _touchAllow == true) {
        return true;
    } else {
        return false;
    }

}

// タップスライド中
void GameLayer::onTouchMoved(Touch* pm_Touch, Event* unused_event) {
    // 矢印オブジェクト宣言
    Node* w_Arrow;

    // 矢印を取得
    w_Arrow = getChildByTag(_playerNum + PLAYER_TAG)->getChildByTag(_playerNum + ARROW_TAG);

    // 矢印の向きと大きさを計算
    auto w_Angle = CC_RADIANS_TO_DEGREES((_onTouchPosition - pm_Touch->getLocation()).getAngle());
    float w_Distance = pm_Touch->getLocation().getDistance(_onTouchPosition);

    // 矢印の長さによって色を変化する
    if (w_Distance < 100) {
        w_Arrow->setVisible(false);
    } else if (w_Distance > 500) {
        w_Distance = 500;
        w_Arrow->setVisible(true);
        w_Arrow->setColor(Color3B::RED);
    } else {
        w_Arrow->setVisible(true);
        w_Arrow->setColor(Color3B::ORANGE);
    }

    // 矢印をセット
    w_Arrow->setRotation((w_Angle - 90) * -1);
    w_Arrow->setScaleY(w_Distance / 100);
}

// タップ終了
void GameLayer::onTouchEnded(Touch* pm_Touch, Event* unused_event) {
    // ボール取得
    auto w_Ball = getChildByTag(_playerNum + PLAYER_TAG);
    auto w_Arrow = w_Ball->getChildByTag(_playerNum + ARROW_TAG);

    // ボールの弾く力を計算 
    Vec2 w_Vec = _onTouchPosition - pm_Touch->getLocation();

    // 最小引っ張りの長さ
    if (std::abs(w_Vec.x) < 100 && std::abs(w_Vec.y) < 100) {
        w_Arrow->setVisible(false);
        return;
    }

    // 最大引っ張りの長さ
    auto w_Hypotence = std::hypot(std::abs(w_Vec.x), std::abs(w_Vec.y));
    if (w_Hypotence >= 100) {
        // ベクトルの長さ×強さ
        float x = w_Vec.x / w_Hypotence;
        float y = w_Vec.y / w_Hypotence;
        w_Vec = Vec2(x, y) * 1500;
    }

    // タッチを無効化
    _touchAllow = false;

    // タグ取得
    auto w_Player1 = getChildByTag(T_Player1);
    auto w_Player2 = getChildByTag(T_Player2);

    // プレイヤーセット
    Player w_Player;
    switch (_playerNum) {
        case P_Num1:
            w_Player.velocity1 = w_Vec;
            w_Player.velocity2 = w_Player2->getPhysicsBody()->getVelocity();
            break;
        case P_Num2:
            w_Player.velocity1 = w_Player1->getPhysicsBody()->getVelocity();
            w_Player.velocity2 = w_Vec;
            break;
    }
    w_Player.position1 = w_Player1->getPosition();
    w_Player.position2 = w_Player2->getPosition();

    _shotAfterSync = false;

    // プレイヤー動く同期処理
    sendMessage(A_MoveSync, w_Player);

    // 矢印非表示
    w_Arrow->setVisible(false);

    // フレーム処理を起動
    this->scheduleUpdate();
}

// タップキャンセル
void GameLayer::onTouchCancelled(Touch* touch, Event* unused_event) {
    onTouchEnded(touch, unused_event);
}