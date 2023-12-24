#ifndef __AUDIO__H__
#define __AUDIO__H__

#include "cocos2d.h"
#include "cocostudio/simpleAudioEngine.h"
#include "Common.h"

class Audio : public cocos2d::Sprite {
private:
    int _AudioNumber;
    virtual bool init();    //初期化
public:
    void startAudio(int pm_AudioNumber,float pm_Delay); //音楽を再生
    static Audio* create();	//インスタンス生成
};

#endif // __AUDIO__H__
