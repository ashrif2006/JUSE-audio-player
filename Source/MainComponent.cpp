#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    formatManager.registerBasicFormats();

    // إضافة الأزرار للواجهة
    addAndMakeVisible(playPauseButton);
    addAndMakeVisible(muteButton);
    addAndMakeVisible(loopButton);
    addAndMakeVisible(jumpBackButton);
    addAndMakeVisible(jumpForwardButton);
    addAndMakeVisible(loadButton);


    // تسجيل الأزرار مع الـ listener
    playPauseButton.addListener(this);
    muteButton.addListener(this);
    loopButton.addListener(this);
    jumpBackButton.addListener(this);
    jumpForwardButton.addListener(this);
    loadButton.addListener(this);

    addAndMakeVisible(volumeSlider);
    volumeSlider.setRange(0.0, 1.0);        // المدى من 0 (صامت) إلى 1 (أعلى صوت)
    volumeSlider.setValue(lastVolume);      // القيمة الابتدائية
    volumeSlider.addListener(this);

    setSize(600, 200);                      // حجم الواجهة
    setAudioChannels(0, 2);
    //اسم الملف الصوتي
    addAndMakeVisible(fileNameLabel);
    fileNameLabel.setText("No file loaded", juce::dontSendNotification);
    fileNameLabel.setJustificationType(juce::Justification::centredLeft);


}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    transportSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    transportSource.releaseResources();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setFont (juce::FontOptions (16.0f));
    g.setColour (juce::Colours::white);
    
}

void MainComponent::resized()
{
    int x = 10, y = 10, w = 90, h = 30, spacing = 10;

    playPauseButton.setBounds(x, y, w, h);
    x += w + spacing;
    muteButton.setBounds(x, y, w, h);
    x += w + spacing;
    loopButton.setBounds(x, y, w + 20, h);

    x = 10; y += h + spacing;
    jumpBackButton.setBounds(x, y, w, h);
    x += w + spacing;
    jumpForwardButton.setBounds(x, y, w, h);
    x += w + spacing;
    loadButton.setBounds(x, y, w + 20, h);

    volumeSlider.setBounds(10, y + h + spacing, 200, 20);

    volumeSlider.setBounds(10, y + h + spacing, 200, 20);

    fileNameLabel.setBounds(220, y + h + spacing, getWidth() - 230, 20);

}
void MainComponent::buttonClicked(juce::Button* button)
{
    if (button == &playPauseButton)
    {
        if (transportSource.isPlaying())
        {
            transportSource.stop();
            playPauseButton.setButtonText("play");
        }
        else
        {
            transportSource.start();
            playPauseButton.setButtonText("||");

        }
    }
    else if (button == &muteButton)
    {
        isMuted = !isMuted;
        if (isMuted)
        {
            lastVolume = static_cast<float>(volumeSlider.getValue());
            transportSource.setGain(0.0f);
            muteButton.setButtonText("Unmute");
        }
        else
        {
            transportSource.setGain(lastVolume);
            muteButton.setButtonText("Mute");
        }
    }
    else if (button == &loopButton)
    {
        isLooping = !isLooping;
        loopButton.setButtonText(isLooping ? "Loop: On" : "Loop: Off");
        if (readerSource.get() != nullptr)
            readerSource->setLooping(isLooping);
    }
    else if (button == &jumpBackButton)
    {
        double currentPos = transportSource.getCurrentPosition();
        transportSource.setPosition(std::max(0.0, currentPos - 5.0));
    }
    else if (button == &jumpForwardButton)
    {
        double currentPos = transportSource.getCurrentPosition();
        double length = transportSource.getLengthInSeconds();
        transportSource.setPosition(std::min(length, currentPos + 5.0));
    }
    else if (button == &loadButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>("Select an audio file...", juce::File{}, "*.wav;*.mp3");
        fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file.existsAsFile())
                {
                    // هنا نعرض اسم الملف في الـ Label
                    fileNameLabel.setText(file.getFileName(), juce::dontSendNotification);

                    auto* reader = formatManager.createReaderFor(file);
                    if (reader != nullptr)
                    {
                        readerSource.reset(new juce::AudioFormatReaderSource(reader, true));
                        readerSource->setLooping(isLooping);
                        transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
                        transportSource.setGain(static_cast<float>(volumeSlider.getValue()));
                    }
                }
            });
    }


}

void MainComponent::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volumeSlider && !isMuted)
    {
        lastVolume = static_cast<float>(volumeSlider.getValue());
        transportSource.setGain(lastVolume);
    }
}

