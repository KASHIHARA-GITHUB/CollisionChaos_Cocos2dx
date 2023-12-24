#include "TitleLayer.h"

#define WINSIZE Director::getInstance()->getWinSize() // 画面サイズ
#define SOUNDDELAY 0.0f          // 音楽遅延時間
#define TitlePositionY 100       // タイトル位置Y
#define TitleButtonPositionY 100 // タイトルボタン位置Y
#define TitleButtonSize      50  // タイトルボタン大きさ

USING_NS_CC;

using namespace CocosDenshion;
using namespace Common;

// シーン生成
Scene* TitleLayer::createScene() {
    auto w_Scene = Scene::create();      // タイトルシーンを生成する
    auto w_Layer = TitleLayer::create(); // TitleLayerクラスのレイヤーを生成する
    w_Scene->addChild(w_Layer);          // シーンに対してレイヤーを追加する

    return w_Scene; // シーンを返す
}

// タイトルレイヤー初期化処理
bool TitleLayer::init() {
    // 初期化
    if ( !Layer::init())
        return false;

    Director::getInstance()->getTextureCache()->removeAllTextures();

    // シングルタップイベントの取得
    auto w_TouchListener = EventListenerTouchOneByOne::create();
    w_TouchListener->onTouchBegan = CC_CALLBACK_2(TitleLayer::onTouchBegan, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(w_TouchListener, this);

    // オーディオを再生
    auto w_Audio = Audio::create();
    w_Audio->startAudio(S_Title, SOUNDDELAY);
    addChild(w_Audio);

    return true;
}

// レイヤー表示時処理
void TitleLayer::onEnter() {
    Layer::onEnter();

    createBackground();     // 背景を生成
    createTitleAndButton(); // タイトルとボタンを生成
}

// 背景生成
void TitleLayer::createBackground() {
    // 背景の後ろ設定
    auto w_BackBehind = Background::create(L_Game);
    w_BackBehind->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    w_BackBehind->setPosition(Vec2(WINSIZE / 2));
    addChild(w_BackBehind, Z_BackgroundBehind);

    // 背景の設定
    auto w_Background = Background::create(L_Title);
    w_Background->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    w_Background->setPosition(Vec2(WINSIZE / 2));
    addChild(w_Background, Z_Background, T_Background);
}

// タイトルとボタンを生成
void TitleLayer::createTitleAndButton() {
    // 背景の位置を取得
    auto w_Background = getChildByTag(T_Background);
    float w_BgPosX = w_Background->getPosition().x;
    float w_BgPosY = w_Background->getPosition().y;

    // タイトルを表示
    auto w_Title = Sprite::create("titlelogo.png");
    w_Title->setPosition(Vec2(w_BgPosX, w_BgPosY + TitlePositionY));
    addChild(w_Title, Z_Title, T_Title);

    // スタートボタンのアニメーション(フェードイン・アウト)
    auto w_Delay   = DelayTime::create(1);
    auto w_FadeOut = FadeTo::create(1.0f, 0);
    auto w_FadeIn  = FadeTo::create(1.0f, 255);
    auto w_Seq     = Sequence::create(w_Delay, w_FadeOut, w_FadeIn, nullptr);
    auto w_Repeat  = RepeatForever::create(w_Seq);
 
    // スタートボタンを表示
    auto w_StartButton = Label::createWithTTF(cText::getText(TXT_Title1), cText::getFont(F_Google), TitleButtonSize);
    w_StartButton->setPosition(Vec2(w_BgPosX, w_BgPosY - TitlePositionY));
    w_StartButton->setTextColor(Color4B::WHITE);
    w_StartButton->runAction(w_Repeat);
    addChild(w_StartButton, Z_Title, T_Title);
}

// タッチイベント
bool TitleLayer::onTouchBegan(Touch* pm_Touch, Event* pm_Unused_event) {
    // 効果音再生
    auto w_Audio = SimpleAudioEngine::getInstance();
    w_Audio->playEffect(cSound::getSEName(SE_Button));

    // 選択画面シーンへ移動
    auto w_Scene = GameLayer::createScene();
    auto w_Tran = TransitionFade::create(1, w_Scene);
    Director::getInstance()->replaceScene(w_Tran);

    // タップアニメーション表示
    tapAnimation(pm_Touch);

    return true;
}

// タップアニメーション
void TitleLayer::tapAnimation(Touch* pm_Touch) {
    // タップアニメーション生成
    auto w_Tap = Tap::create();
    w_Tap->setPosition(Vec2(pm_Touch->getLocation().x, pm_Touch->getLocation().y));
    addChild(w_Tap, Z_Tap, T_Tap);
}
