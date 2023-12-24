#include "Audio.h"

USING_NS_CC;

using namespace CocosDenshion;
using namespace Common;

//音楽生成
Audio* Audio::create() {
    Audio* w_pRet = new Audio();
    if (w_pRet && w_pRet->init()) {
        w_pRet->autorelease();
        return w_pRet;
    } else {
        delete w_pRet;
        w_pRet = nullptr;
        return nullptr;
    }
}

//音楽の初期化
bool Audio::init() {
    return true;
}

// 音楽の再生
void Audio::startAudio(int pm_AudioNumber, float pm_Delay) {
    // オーディオを生成
    auto w_Audio = SimpleAudioEngine::getInstance();

    // メンバー変数をセット
    _AudioNumber = pm_AudioNumber;

    // 音楽更新のために停止
    w_Audio->stopBackgroundMusic();

    // 音楽再生処理
    auto w_Delay = DelayTime::create(pm_Delay);
    auto w_Func = CallFunc::create([&]() {
        // BGM再生
        auto w_Audio = SimpleAudioEngine::getInstance();
        w_Audio->stopBackgroundMusic();
        w_Audio->playBackgroundMusic(cSound::getSoundName(_AudioNumber), true);
    });
    auto w_Seq = Sequence::create(w_Delay, w_Func, nullptr);
    runAction(w_Seq);
}