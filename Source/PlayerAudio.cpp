#include "PlayerAudio.h"

PlayerAudio::PlayerAudio()
{
    formatManager.registerBasicFormats();
}

PlayerAudio::~PlayerAudio()
{
    releaseResources();
}

void PlayerAudio::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    if (resamplingSource)
        resamplingSource->prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void PlayerAudio::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    if (resamplingSource)
        resamplingSource->getNextAudioBlock(bufferToFill);
}

void PlayerAudio::releaseResources()
{
    if (resamplingSource)
        resamplingSource->releaseResources();

    transportSource.releaseResources();

    resamplingSource.reset();
    readerSource.reset();
}

void PlayerAudio::loadFile(const juce::File& file)
{
    // 🔹 وقف التشغيل وتحرير أي مصدر سابق
    transportSource.stop();
    transportSource.setSource(nullptr);
    resamplingSource.reset();
    readerSource.reset();

    if (!file.existsAsFile())
        return;

    // 🔹 إنشاء القارئ
    auto* rawReader = formatManager.createReaderFor(file);
    if (rawReader == nullptr)
        return;

    // 🔹 بيانات التعريف
    juce::StringPairArray md = rawReader->metadataValues;
    title = md.getValue("title", md.getValue("TITLE", file.getFileNameWithoutExtension()));
    author = md.getValue("artist", md.getValue("ARTIST", ""));
    durationSeconds = (rawReader->sampleRate > 0.0)
        ? (static_cast<double>(rawReader->lengthInSamples) / rawReader->sampleRate)
        : 0.0;

    // 🔹 مصدر القراءة الأساسي
    readerSource.reset(new juce::AudioFormatReaderSource(rawReader, true));

    // 🔹 المصدر المعدِّل (للسرعة)
    resamplingSource.reset(new juce::ResamplingAudioSource(readerSource.get(), false));
    resamplingSource->setResamplingRatio(speed);

    // 🔹 ربط transport بالمصدر الأساسي
    transportSource.setSource(readerSource.get(), 0, nullptr, rawReader->sampleRate);

    // 🔹 تهيئة resampler
    resamplingSource->prepareToPlay(512, rawReader->sampleRate);
}

void PlayerAudio::start()
{
    transportSource.start();
}

void PlayerAudio::stop()
{
    transportSource.stop();
}

void PlayerAudio::goToStart()
{
    transportSource.setPosition(0.0);
}

void PlayerAudio::goToEnd()
{
    transportSource.setPosition(transportSource.getLengthInSeconds());
}


void PlayerAudio::skipForward(double seconds)
{
    double newPos = transportSource.getCurrentPosition() + seconds;
    double maxPos = transportSource.getLengthInSeconds();

    if (newPos > maxPos)
        newPos = maxPos;

    transportSource.setPosition(newPos);
}

void PlayerAudio::skipBackward(double seconds)
{
    double newPos = transportSource.getCurrentPosition() - seconds;
    if (newPos < 0.0)
        newPos = 0.0;

    transportSource.setPosition(newPos);
}


void PlayerAudio::toggleMute()
{
    isMuted = !isMuted;
    transportSource.setGain(isMuted ? 0.0f : 1.0f);
}

void PlayerAudio::toggleLoop()
{
    isLooping = !isLooping;
    if (readerSource)
        readerSource->setLooping(isLooping);
}

void PlayerAudio::setSpeed(double newSpeed)
{
    speed = juce::jlimit(0.1, 3.0, newSpeed);

    if (resamplingSource)
        resamplingSource->setResamplingRatio(speed);
}

bool PlayerAudio::isPlaying() const
{
    return transportSource.isPlaying();
}

juce::String PlayerAudio::getDurationString() const
{
    int totalSecs = static_cast<int>(durationSeconds);
    int mins = totalSecs / 60;
    int secs = totalSecs % 60;
    return juce::String::formatted("%02d:%02d", mins, secs);
}
