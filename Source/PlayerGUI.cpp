#include "PlayerGUI.h"

PlayerGUI::PlayerGUI(PlayerAudio& p) : player(p)
{
    // الأزرار
    addAndMakeVisible(playButton);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(startButton);
    addAndMakeVisible(endButton);
    addAndMakeVisible(muteButton);
    addAndMakeVisible(loopButton);
    addAndMakeVisible(forwardButton);
    addAndMakeVisible(backwardButton);
    forwardButton.setButtonText(" +10s");
    backwardButton.setButtonText(" -10s");

    forwardButton.onClick = [this] { player.skipForward(); };
    backwardButton.onClick = [this] { player.skipBackward(); };

    playButton.addListener(this);
    loadButton.addListener(this);
    startButton.addListener(this);
    endButton.addListener(this);
    muteButton.addListener(this);
    loopButton.addListener(this);

    // الـ Labels (بيانات الأغنية)
    addAndMakeVisible(titleLabel);
    addAndMakeVisible(authorLabel);
    addAndMakeVisible(durationLabel);

    titleLabel.setText("Title: -", juce::dontSendNotification);
    authorLabel.setText("Artist: -", juce::dontSendNotification);
    durationLabel.setText("Duration: 0:00", juce::dontSendNotification);

    titleLabel.setFont(juce::Font(14.0f));
    authorLabel.setFont(juce::Font(14.0f));
    durationLabel.setFont(juce::Font(14.0f));

    speedLabel.setText("Speed:", juce::dontSendNotification);
    addAndMakeVisible(speedLabel);

    speedSlider.setRange(0.5, 2.0, 0.1); // من 0.5x إلى 2.0x
    speedSlider.setValue(1.0);           // القيمة الافتراضية
    speedSlider.onValueChange = [this]()
        {
            player.setSpeed(speedSlider.getValue());
        };
    addAndMakeVisible(speedSlider);

}

PlayerGUI::~PlayerGUI() = default;

void PlayerGUI::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}
void PlayerGUI::resized()
{
    auto area = getLocalBounds().reduced(15);
    int buttonWidth = 100;
    int buttonHeight = 35;
    int spacing = 15;

    // الصف الأول
    auto topRow = area.removeFromTop(buttonHeight);
    playButton.setBounds(topRow.removeFromLeft(buttonWidth));
    topRow.removeFromLeft(spacing);
    loadButton.setBounds(topRow.removeFromLeft(buttonWidth));
    topRow.removeFromLeft(spacing);
    startButton.setBounds(topRow.removeFromLeft(buttonWidth));
    topRow.removeFromLeft(spacing);
    endButton.setBounds(topRow.removeFromLeft(buttonWidth));

    area.removeFromTop(spacing);

    // الصف الثاني
    auto secondRow = area.removeFromTop(buttonHeight);
    muteButton.setBounds(secondRow.removeFromLeft(buttonWidth));
    secondRow.removeFromLeft(spacing);
    loopButton.setBounds(secondRow.removeFromLeft(buttonWidth));

    area.removeFromTop(spacing);

    // ✅ الصف الثالث: الأزرار الجديدة
    auto thirdRow = area.removeFromTop(buttonHeight);
    backwardButton.setBounds(thirdRow.removeFromLeft(buttonWidth));
    thirdRow.removeFromLeft(spacing);
    forwardButton.setBounds(thirdRow.removeFromLeft(buttonWidth));

    area.removeFromTop(spacing * 2);

    // Labels
    int labelWidth = area.getWidth() / 3;
    int labelHeight = 24;
    auto labelRow = area.removeFromTop(labelHeight);
    titleLabel.setBounds(labelRow.removeFromLeft(labelWidth));
    authorLabel.setBounds(labelRow.removeFromLeft(labelWidth));
    durationLabel.setBounds(labelRow.removeFromLeft(labelWidth));

    area.removeFromTop(20);
    auto sliderArea = area.removeFromTop(40);
    speedLabel.setBounds(sliderArea.removeFromLeft(60));
    speedSlider.setBounds(sliderArea);
}


void PlayerGUI::buttonClicked(juce::Button* button)
{
    if (button == &playButton)
    {
        if (player.isPlaying())
        {
            player.stop();
            playButton.setButtonText("Play");
        }
        else
        {
            player.start();
            playButton.setButtonText("Stop");
        }
    }
    else if (button == &loadButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select an audio file...",
            juce::File{},
            "*.mp3;*.wav;*.aiff"
        );

        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                juce::File file = fc.getResult();
                if (!file.existsAsFile())
                {
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::WarningIcon,
                        "Error",
                        "File not found or invalid."
                    );
                    fileChooser.reset();
                    return;
                }

                // نحاول تحميل الملف بأمان
                try
                {
                    player.loadFile(file);

                    titleLabel.setText("Title: " + player.getTitle(), juce::dontSendNotification);

                    juce::String artist = player.getAuthor();
                    if (artist.isEmpty()) artist = "-";

                    authorLabel.setText("Artist: " + artist, juce::dontSendNotification);
                    durationLabel.setText("Duration: " + player.getDurationString(), juce::dontSendNotification);
                }
                catch (...)
                {
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::WarningIcon,
                        "Error loading file",
                        "An unexpected error occurred while loading the file."
                    );
                }

                // بعد الانتهاء نحذف الـ chooser بأمان
                fileChooser.reset();
            }
        );
    }


    else if (button == &startButton)
    {
        player.goToStart();
    }
    else if (button == &endButton)
    {
        player.goToEnd();
    }
    else if (button == &muteButton)
    {
        player.toggleMute();
        muteButton.setButtonText(player.isMutedFlag() ? "Unmute" : "Mute");
    }
    else if (button == &loopButton)
    {
        player.toggleLoop();
        loopButton.setButtonText(player.isLoopingFlag() ? "Loop: ON" : "Loop: OFF");
    }
}
