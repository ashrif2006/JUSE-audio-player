#include "MainComponent.h"



// تحت الـ #include ... في بداية MainComponent.cpp

juce::String readID3v1Artist(const juce::File& file)
{
    if (!file.existsAsFile())
        return "File not found";

    juce::FileInputStream inputStream(file);
    if (!inputStream.openedOk())
        return "Can't open file";

    if (inputStream.getTotalLength() < 128)
        return "File too small for ID3v1";

    inputStream.setPosition(inputStream.getTotalLength() - 128);

    char tag[3];
    inputStream.read(tag, 3);

    if (strncmp(tag, "TAG", 3) != 0)
        return "No ID3v1 tag";

    char title[30];
    char artist[30];

    inputStream.read(title, 30);
    inputStream.read(artist, 30);

    return juce::String(artist, 30).trim();
}



//==============================================================================

MainComponent::MainComponent()
{
    formatManager.registerBasicFormats();



    // Add buttons
    addAndMakeVisible(playPauseButton);
    addAndMakeVisible(muteButton);
    addAndMakeVisible(loopButton);
    addAndMakeVisible(jumpBackButton);
    addAndMakeVisible(jumpForwardButton);
    addAndMakeVisible(loadButton);

    addAndMakeVisible(waveformPlaceholder);
    waveformPlaceholder.setColour(juce::ResizableWindow::backgroundColourId, juce::Colours::orange);


    playPauseButton.addListener(this);
    muteButton.addListener(this);
    loopButton.addListener(this);
    jumpBackButton.addListener(this);
    jumpForwardButton.addListener(this);
    loadButton.addListener(this);

    addAndMakeVisible(volumeSlider);
    volumeSlider.setRange(0.0, 1.0);
    volumeSlider.setValue(lastVolume);
    volumeSlider.addListener(this);

    addAndMakeVisible(playlistBox);
    playlistBox.setModel(this);

    addAndMakeVisible(metadataLabel);
    metadataLabel.setText("No file loaded", juce::dontSendNotification);
    metadataLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    metadataLabel.setJustificationType(juce::Justification::centredLeft);

    setSize(700, 350);
    setAudioChannels(0, 2);
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
    if (readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }
    transportSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    transportSource.releaseResources();
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds().reduced(10);

    // تقسيم النافذة لمستويات
    auto controlsArea = bounds.removeFromTop(40);
    auto sliderArea = bounds.removeFromTop(40);
    auto waveformArea = bounds.removeFromTop(100); // هنستخدمها لاحقًا
    auto playlistArea = bounds.removeFromTop(getHeight() / 3);
    auto metadataArea = bounds.removeFromBottom(30);

    // --- الأزرار ---
    auto buttonWidth = 90;
    auto spacing = 10;

    playPauseButton.setBounds(controlsArea.removeFromLeft(buttonWidth));
    controlsArea.removeFromLeft(spacing);
    muteButton.setBounds(controlsArea.removeFromLeft(buttonWidth));
    controlsArea.removeFromLeft(spacing);
    loopButton.setBounds(controlsArea.removeFromLeft(buttonWidth + 20));
    controlsArea.removeFromLeft(spacing);
    jumpBackButton.setBounds(controlsArea.removeFromLeft(buttonWidth));
    waveformPlaceholder.setBounds(waveformArea);

    controlsArea.removeFromLeft(spacing);
    jumpForwardButton.setBounds(controlsArea.removeFromLeft(buttonWidth));

    // زرار التحميل + السلايدر
    auto loadWidth = 110;
    loadButton.setBounds(sliderArea.removeFromLeft(loadWidth));
    sliderArea.removeFromLeft(spacing);
    volumeSlider.setBounds(sliderArea);

    // --- الـ Playlist ---
    playlistBox.setBounds(playlistArea);

    // --- Metadata (اسم الأغنية + الفنان) ---
    metadataLabel.setBounds(metadataArea);
}


void MainComponent::buttonClicked(juce::Button* button)
{
    if (button == &playPauseButton)
    {
        if (transportSource.isPlaying())
        {
            transportSource.stop();
            playPauseButton.setButtonText("Play");
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
        if (readerSource != nullptr)
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
        fileChooser = std::make_unique<juce::FileChooser>("Select audio files...", juce::File{}, "*.wav;*.mp3");

        fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& chooser)
            {
                auto files = chooser.getResults();

                for (auto& file : files)
                {
                    if (file.existsAsFile())
                    {
                        playlistFiles.add(file.getFullPathName());
                    }
                }

                playlistBox.updateContent();

                if (!playlistFiles.isEmpty())
                {
                    currentFileIndex = 0;
                    loadFile(playlistFiles[0]);
                    playlistBox.selectRow(0);
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

int MainComponent::getNumRows()
{
    return playlistFiles.size();
}

void MainComponent::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::lightblue);

    if (rowNumber >= 0 && rowNumber < playlistFiles.size())
    {
        juce::String fileName = juce::File(playlistFiles[rowNumber]).getFileName();
        g.setColour(juce::Colours::black);
        g.drawText(fileName, 5, 0, width, height, juce::Justification::centredLeft, true);
    }
}

void MainComponent::selectedRowsChanged(int lastRowSelected)
{
    if (lastRowSelected >= 0 && lastRowSelected < playlistFiles.size())
    {
        currentFileIndex = lastRowSelected;
        loadFile(playlistFiles[currentFileIndex]);
    }
}

void MainComponent::loadFile(const juce::String& path)
{
    juce::File file(path);

    if (file.existsAsFile())
    {
        transportSource.stop();
        transportSource.setSource(nullptr);
        readerSource.reset();

        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
        if (reader != nullptr)
        {
            double sampleRate = reader->sampleRate;
            readerSource.reset(new juce::AudioFormatReaderSource(reader.release(), true));
            readerSource->setLooping(isLooping);
            transportSource.setSource(readerSource.get(), 0, nullptr, sampleRate);
            transportSource.setGain(static_cast<float>(volumeSlider.getValue()));

            juce::String fileName = file.getFileName();
            juce::String artist = readID3v1Artist(file);

            juce::String info = "File: " + fileName;
            if (artist.isNotEmpty())
                info += " | Artist: " + artist;

            metadataLabel.setText(info, juce::dontSendNotification);
        }
    }
}


