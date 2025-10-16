#pragma once

#include <JuceHeader.h>

class MainComponent : public juce::AudioAppComponent,
    public juce::Button::Listener,
    public juce::Slider::Listener,
    public juce::ListBoxModel
{
public:
    MainComponent();
    ~MainComponent() override;

    // AudioAppComponent overrides
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

    // Button listener
    void buttonClicked(juce::Button* button) override;

    // Slider listener
    void sliderValueChanged(juce::Slider* slider) override;

    // ListBoxModel overrides
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void selectedRowsChanged(int lastRowSelected) override;

private:
    void loadFile(const juce::String& path);

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

    juce::TextButton playPauseButton{ "Play" };
    juce::TextButton muteButton{ "Mute" };
    juce::TextButton loopButton{ "Loop: Off" };
    juce::TextButton jumpBackButton{ "<< 5s" };
    juce::TextButton jumpForwardButton{ "5s >>" };
    juce::TextButton loadButton{ "Load Files" };

    juce::Slider volumeSlider;

    juce::ListBox playlistBox;

    juce::Label metadataLabel;

    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::StringArray playlistFiles; // قائمة ملفات المسارات
    int currentFileIndex{ -1 };

    bool isMuted{ false };
    bool isLooping{ false };
    float lastVolume{ 0.5f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
