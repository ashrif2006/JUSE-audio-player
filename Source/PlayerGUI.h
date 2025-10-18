#pragma once
#include <JuceHeader.h>
#include "PlayerAudio.h"

class PlayerGUI : public juce::Component,
    public juce::Button::Listener
{
public:
    PlayerGUI(PlayerAudio& playerRef);
    ~PlayerGUI() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;

private:
    PlayerAudio& player;

    juce::TextButton playButton{ "Play" };
    juce::TextButton startButton{ "Start" };
    juce::TextButton endButton{ "End" };
    juce::TextButton loadButton{ "Load" };
    juce::TextButton muteButton{ "Mute" };
    juce::TextButton loopButton{ "Loop" };
    juce::TextButton forwardButton{ " +10s" };
    juce::TextButton backwardButton{ " -10s" };


    juce::Label titleLabel;
    juce::Label authorLabel;
    juce::Label durationLabel;
    juce::Slider speedSlider;
    juce::Label speedLabel;

    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerGUI)
};
