#pragma once
#include <JuceHeader.h>
#include "PlayerAudio.h"
#include "PlayerGUI.h"

// MainComponent هو القلب الرئيسي للتطبيق
class MainComponent : public juce::AudioAppComponent
{
public:
    MainComponent();
    ~MainComponent() override;

    // AudioAppComponent methods.
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    // GUI
    void resized() override;

private:
    PlayerAudio player;   // مسؤول عن الصوت
    PlayerGUI gui{ player }; // واجهة المستخدم (بتتعامل مع PlayerAudio)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
