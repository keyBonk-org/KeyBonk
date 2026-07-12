#include "functions/audioPlay.hpp"
#include "debug.hpp"
#include "globalDevelopmentControl.hpp"
#include "functions/files.hpp"
#include "global.hpp"
#include "audio-player/audioPlayer.hpp"

// 播放音频文件的通用函数
void PlayAudioFile(int keyCode)
{
    if (keybonk::global.audioPreloadReady.load(std::memory_order_acquire)) // 检查预加载是否完成
    {
        // ready设为true说明预加载完成，没必要上锁了
        if (auto it = keybonk::global.audioList.find(keyCode); it != keybonk::global.audioList.end())
        {
            yumo::addAudio(it->second);
        }
    }
}