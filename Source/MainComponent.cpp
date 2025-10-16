 #include "MainComponent.h"

// دالة مساعدة لقراءة بيانات ID3v1
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
    addAndMakeVisible(waveformDisplay);
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
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);

    // رقم 6 - منزلق السرعة مع قيم محددة
    addAndMakeVisible(speedSlider);
    speedSlider.setRange(0, 6); // 7 قيم: 0-6
    speedSlider.setValue(2);    // القيمة الافتراضية 1.0x (المؤشر 2)
    speedSlider.addListener(this);
    speedSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    // إضافة التسميات لقيم السرعة
    addAndMakeVisible(speedLabel);
    speedLabel.setText("Speed: 1.0x", juce::dontSendNotification);
    speedLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    speedLabel.setJustificationType(juce::Justification::centredLeft);

    // رقم 9 - منزلق الموضع
    addAndMakeVisible(positionSlider);
    positionSlider.setRange(0.0, 1.0);
    positionSlider.addListener(this);
    positionSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    // قائمة التشغيل
    addAndMakeVisible(playlistBox);
    playlistBox.setModel(this);

    addAndMakeVisible(metadataLabel);
    metadataLabel.setText("No file loaded", juce::dontSendNotification);
    metadataLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    metadataLabel.setJustificationType(juce::Justification::centredLeft);

    // رقم 9 - تسميات الوقت
    addAndMakeVisible(currentTimeLabel);
    addAndMakeVisible(totalTimeLabel);
    currentTimeLabel.setText("0:00", juce::dontSendNotification);
    totalTimeLabel.setText("0:00", juce::dontSendNotification);
    currentTimeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    totalTimeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    currentTimeLabel.setJustificationType(juce::Justification::centred);
    totalTimeLabel.setJustificationType(juce::Justification::centred);

    // بدء التايمر للتحديث المستمر
    startTimer(50); // تحديث كل 50 مللي ثانية

    setSize(700, 350);
    setAudioChannels(0, 2);
}

MainComponent::~MainComponent()
{
    stopTimer(); // إضافة هذا السطر
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
    auto speedArea = bounds.removeFromTop(40); // إضافة منطقة السرعة
    auto waveformArea = bounds.removeFromTop(100);
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
    controlsArea.removeFromLeft(spacing);
    jumpForwardButton.setBounds(controlsArea.removeFromLeft(buttonWidth));

    // زرار التحميل + السلايدر
    auto loadWidth = 110;
    loadButton.setBounds(sliderArea.removeFromLeft(loadWidth));
    sliderArea.removeFromLeft(spacing);
    volumeSlider.setBounds(sliderArea);

    // رقم 6 - منطقة السرعة
    auto speedLabelArea = speedArea.removeFromLeft(80);
    speedLabel.setBounds(speedLabelArea);
    speedArea.removeFromLeft(10);
    speedSlider.setBounds(speedArea.removeFromLeft(200));

    // --- رقم 9 - منطقة الشكل الموجي والموضع ---
    auto timeLabelsArea = waveformArea.removeFromBottom(20);
    waveformDisplay.setBounds(waveformArea);

    // رقم 9 - منزلق الموضع
    positionSlider.setBounds(waveformArea);

    // رقم 9 - تسميات الوقت
    currentTimeLabel.setBounds(timeLabelsArea.removeFromLeft(50));
    totalTimeLabel.setBounds(timeLabelsArea.removeFromRight(50));

    // --- الـ Playlist ---
    playlistBox.setBounds(playlistArea);

    // --- Metadata (اسم الأغنية + الفنان) ---
    metadataLabel.setBounds(metadataArea);
}

// رقم 9 - WaveformDisplay implementation
WaveformDisplay::WaveformDisplay() : thumbnailCache(5), audioThumbnail(512, formatManager, thumbnailCache)
{
    formatManager.registerBasicFormats();
}

WaveformDisplay::~WaveformDisplay()
{
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);

    if (audioThumbnail.getNumChannels() == 0)
    {
        g.setColour(juce::Colours::white);
        g.drawText("No file loaded", getLocalBounds(), juce::Justification::centred);
    }
    else
    {
        g.setColour(juce::Colours::lightblue);
        audioThumbnail.drawChannels(g, getLocalBounds(), 0.0, audioThumbnail.getTotalLength(), 1.0f);

        // رسم مؤشر الموضع الحالي
        if (position > 0.0)
        {
            g.setColour(juce::Colours::red);
            int xPosition = static_cast<int>(position * getWidth());
            g.drawLine(xPosition, 0, xPosition, getHeight(), 2.0f);
        }
    }
}

void WaveformDisplay::resized()
{
}

void WaveformDisplay::loadFile(const juce::File& file)
{
    audioThumbnail.setSource(new juce::FileInputSource(file));
    repaint();
}

void WaveformDisplay::setPosition(double pos)
{
    position = pos;
    repaint();
}

void WaveformDisplay::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    repaint();
}

// رقم 9 - دالة التايمر للتحديث المستمر
void MainComponent::timerCallback()
{
    if (transportSource.isPlaying() || transportSource.getCurrentPosition() > 0.0)
    {
        // تحديث منزلق الموضع
        double currentPos = transportSource.getCurrentPosition();
        double totalLength = transportSource.getLengthInSeconds();

        if (totalLength > 0.0)
        {
            positionSlider.setValue(currentPos / totalLength, juce::dontSendNotification);

            // تحديث تسميات الوقت
            currentTimeLabel.setText(formatTime(currentPos), juce::dontSendNotification);
            totalTimeLabel.setText(formatTime(totalLength), juce::dontSendNotification);
        }

        // تحديث الشكل الموجي
        waveformDisplay.setPosition(currentPos / totalLength);
    }
}

// رقم 9 - دالة تنسيق الوقت
juce::String MainComponent::formatTime(double seconds)
{
    int totalSeconds = static_cast<int>(seconds);
    int minutes = totalSeconds / 60;
    int secs = totalSeconds % 60;
    return juce::String(minutes) + ":" + (secs < 10 ? "0" : "") + juce::String(secs);
}

// رقم 6 - دالة للحصول على قيمة السرعة من المؤشر
double MainComponent::getSpeedFromIndex(int index)
{
    switch (index)
    {
    case 0: return 0.25;
    case 1: return 0.5;
    case 2: return 1.0;
    case 3: return 1.25;
    case 4: return 1.5;
    case 5: return 1.75;
    case 6: return 2.0;
    default: return 1.0;
    }
}

// رقم 6 - دالة لتحديث تسمية السرعة
void MainComponent::updateSpeedLabel()
{
    int index = static_cast<int>(speedSlider.getValue());
    double speed = getSpeedFromIndex(index);
    speedLabel.setText("Speed: " + juce::String(speed) + "x", juce::dontSendNotification);
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
    // رقم 3 - زر كتم الصوت
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
    // رقم 6 - منزلق السرعة
    else if (slider == &speedSlider)
    {
        int index = static_cast<int>(speedSlider.getValue());
        double newSpeed = getSpeedFromIndex(index);

        if (readerSource != nullptr && readerSource->getAudioFormatReader() != nullptr)
        {
            double originalSampleRate = readerSource->getAudioFormatReader()->sampleRate;
            transportSource.setSource(readerSource.get(), 0, nullptr, originalSampleRate * newSpeed);
        }

        updateSpeedLabel();
    }
    // رقم 9 - منزلق الموضع
    else if (slider == &positionSlider)
    {
        if (transportSource.getLengthInSeconds() > 0.0)
        {
            double newPosition = positionSlider.getValue() * transportSource.getLengthInSeconds();
            transportSource.setPosition(newPosition);
        }
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
            
            // رقم 6 - تطبيق السرعة الحالية
            int speedIndex = static_cast<int>(speedSlider.getValue());
            double currentSpeed = getSpeedFromIndex(speedIndex);
            transportSource.setSource(readerSource.get(), 0, nullptr, sampleRate * currentSpeed);
            
            transportSource.setGain(static_cast<float>(volumeSlider.getValue()));

            juce::String fileName = file.getFileName();
            juce::String artist = readID3v1Artist(file);

            juce::String info = "File: " + fileName;
            if (artist.isNotEmpty())
                info += " | Artist: " + artist;

            metadataLabel.setText(info, juce::dontSendNotification);

            // رقم 9 - تحديث الشكل الموجي
            waveformDisplay.loadFile(file);
            
            // رقم 9 - إعادة تعيين تسميات الوقت
            currentTimeLabel.setText("0:00", juce::dontSendNotification);
            totalTimeLabel.setText(formatTime(transportSource.getLengthInSeconds()), juce::dontSendNotification);

            // رقم 6 - تحديث تسمية السرعة
            updateSpeedLabel();
        }
    }
}
