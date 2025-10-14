#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent,
    public juce::Button::Listener,
    public juce::Slider::Listener
{
public:
    //==============================================================================
    
    MainComponent();
    ~MainComponent() override;
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;


private:
    //==============================================================================
    // Your private member variables go here...
    juce::TextButton playPauseButton{ "paly" };
    juce::TextButton muteButton{ "Mute" };
    juce::TextButton loopButton{ "Loop: Off" };
    juce::TextButton jumpBackButton{ "<< 5s" };
    juce::TextButton jumpForwardButton{ "5s >>" };
    juce::TextButton loadButton{ "Load Audio" };

    juce::Slider volumeSlider;

    juce::Label fileNameLabel;


    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

    // States
    bool isMuted = false;
    bool isLooping = false;
    float lastVolume = 0.5f;


    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
