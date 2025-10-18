#pragma once
#include <JuceHeader.h>

class PlayerAudio : public juce::AudioSource
{
public:
    PlayerAudio();
    ~PlayerAudio() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void loadFile(const juce::File& file);
    void start();
    void stop();
    void goToStart();
    void goToEnd();
    void toggleMute();
    void toggleLoop();
    void setSpeed(double newSpeed);
    

    void skipForward(double seconds = 10.0);
    void skipBackward(double seconds = 10.0);



    bool isPlaying() const;
    bool isMutedFlag() const { return isMuted; }
    bool isLoopingFlag() const { return isLooping; }

    juce::String getTitle() const { return title; }
    juce::String getAuthor() const { return author; }
    juce::String getDurationString() const;

private:
    juce::AudioFormatManager formatManager;
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    std::unique_ptr<juce::ResamplingAudioSource> resamplingSource;

    juce::String title, author;
    double durationSeconds = 0.0;
    double speed = 1.0;
    bool isMuted = false;
    bool isLooping = false;
};
