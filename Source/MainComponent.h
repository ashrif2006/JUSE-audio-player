#pragma once

#include <JuceHeader.h>

class MainComponent : public juce::AudioAppComponent,
    public juce::Button::Listener,
    public juce::Slider::Listener,
    public juce::ListBoxModel
    public juce::Timer

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
    // رقم 9 - وظائف التايمر
    void timerCallback() override;


private:
    void loadFile(const juce::String& path);

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::TextButton playPauseButton{ "Play" };
    juce::TextButton muteButton{ "Mute" };
    juce::TextButton loopButton{ "Loop: Off" };
    juce::TextButton jumpBackButton{ "<< 5s" };
    juce::TextButton jumpForwardButton{ "5s >>" };
    juce::TextButton loadButton{ "Load Files" };

    juce::Slider volumeSlider;
    juce::Component waveformPlaceholder;


    juce::ListBox playlistBox;

    juce::Label metadataLabel;

    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::StringArray playlistFiles; // قائمة ملفات المسارات
    int currentFileIndex{ -1 };

    bool isMuted{ false };
    bool isLooping{ false };
    float lastVolume{ 0.5f };
         // رقم 6 - عناصر السرعة
 juce::Slider speedSlider;
 juce::Label speedLabel;
 double getSpeedFromIndex(int index);
 void updateSpeedLabel();

 // رقم 9 - عناصر الشكل الموجي والموضع
 WaveformDisplay waveformDisplay;
 juce::Slider positionSlider;
 juce::Label currentTimeLabel;
 juce::Label totalTimeLabel;
 juce::String formatTime(double seconds);


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
// رقم 9 - كلاس الشكل الموجي
class WaveformDisplay : public juce::Component, public juce::ChangeListener
{
public:
    WaveformDisplay();
    ~WaveformDisplay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void loadFile(const juce::File& file);
    void setPosition(double pos);

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail audioThumbnail;
    double position = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};
