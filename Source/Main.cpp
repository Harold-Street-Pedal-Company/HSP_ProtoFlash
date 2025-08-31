#include <JuceHeader.h>

//==============================================================================
// Helper to sanitize console text coming back from dfu-util
static juce::String makeSafeConsoleText(const char* data, int n)
{
    juce::String out;
    out.preallocateBytes((size_t)n + 8);
    for (int i = 0; i < n; ++i)
    {
        const unsigned char c = (unsigned char)data[i];
        if (c == 0) break;                           // stop at NUL
        if (c == 9 || c == 10 || c == 13) out += juce::String::charToString(static_cast<wchar_t>(c));  // \t \n \r
        else if (c >= 32 && c < 127)      out += juce::String::charToString(static_cast<wchar_t>(c));  // printable ASCII
        else                               out += '.'; // replace weird bytes
    }
    return out;
}

//==============================================================================
// Tiny LED component for DFU status
class StatusLamp : public juce::Component
{
public:
    void setOn(bool onNow) { if (onNow != on) { on = onNow; repaint(); } }
    void paint(juce::Graphics& g) override
    {
        auto area = getLocalBounds().toFloat().reduced(2.0f);
        g.setColour(juce::Colours::black.withAlpha(0.25f));
        g.fillRoundedRectangle(area, 4.0f);
        g.setColour(on ? juce::Colours::green : juce::Colours::red);
        g.fillEllipse(area.reduced(6.0f));
        g.setColour(juce::Colours::white.withAlpha(0.25f));
        g.drawEllipse(area.reduced(6.0f), 1.2f);
    }
private:
    bool on = false;
};

//==============================================================================
// MainComponent: UI for selecting a .bin file and flashing via dfu-util
class MainComponent : public juce::Component,
    private juce::Button::Listener,
    private juce::Timer
{
public:
    MainComponent()
    {
        // Title
        title.setText("Harold Street Pedals' Protoseed Flasher", juce::dontSendNotification);
        {
            // JUCE 8-safe: no deprecated constructor
            juce::Font f;
            f.setHeight(22.0f);
            f.setBold(true);
            title.setFont(f);
        }
        addAndMakeVisible(title);

        // File chooser + flash + save log row
        browseButton.setButtonText("Browse");
        browseButton.addListener(this);
        addAndMakeVisible(browseButton);

        flashButton.setButtonText("Flash Selected File");
        flashButton.addListener(this);
        addAndMakeVisible(flashButton);

        saveLogButton.setButtonText("Save Log");
        saveLogButton.onClick = [this] { saveLog(); };
        addAndMakeVisible(saveLogButton);

        // Selected file
        fileLabel.setText("No file selected", juce::dontSendNotification);
        addAndMakeVisible(fileLabel);

        // dfu-util path (optional)
        dfuPathLabel.setText("dfu-util path (optional):", juce::dontSendNotification);
        addAndMakeVisible(dfuPathLabel);
        addAndMakeVisible(dfuPathEditor);

        // DFU status
        statusText.setText("Device not detected", juce::dontSendNotification);
        {
            juce::Font f2;
            f2.setHeight(16.0f);
            f2.setBold(true);
            statusText.setFont(f2);
        }
        statusText.setColour(juce::Label::textColourId, juce::Colours::red);
        addAndMakeVisible(statusText);
        addAndMakeVisible(statusLamp);

        // Output console
        outputBox.setMultiLine(true);
        outputBox.setReadOnly(true);
        outputBox.setScrollbarsShown(true);
        addAndMakeVisible(outputBox);

        setSize(720, 480);
        startTimer(200);
    }

    ~MainComponent() override { stopTimer(); }

    //==============================================================================
    void resized() override
    {
        auto r = getLocalBounds().reduced(10);

        title.setBounds(r.removeFromTop(32));
        r.removeFromTop(6);

        // Row: buttons + file label
        {
            auto row = r.removeFromTop(30);
            browseButton.setBounds(row.removeFromLeft(110));
            row.removeFromLeft(6);
            flashButton.setBounds(row.removeFromLeft(180));
            row.removeFromLeft(6);
            saveLogButton.setBounds(row.removeFromLeft(110));
            row.removeFromLeft(10);
            fileLabel.setBounds(row);
        }

        r.removeFromTop(8);

        // Row: dfu path editor
        {
            auto row = r.removeFromTop(28);
            dfuPathLabel.setBounds(row.removeFromLeft(180));
            dfuPathEditor.setBounds(row);
        }

        r.removeFromTop(8);

        // Row: status
        {
            auto row = r.removeFromTop(26);
            statusText.setBounds(row.removeFromLeft(200));
            row.removeFromLeft(6);
            statusLamp.setBounds(row.removeFromLeft(32));
        }

        r.removeFromTop(8);
        outputBox.setBounds(r);
    }

private:
    // UI
    juce::Label       title;
    juce::TextButton  browseButton, flashButton, saveLogButton;
    juce::Label       fileLabel;

    juce::Label       dfuPathLabel;
    juce::TextEditor  dfuPathEditor;

    juce::Label       statusText;
    StatusLamp        statusLamp;

    juce::TextEditor  outputBox;

    // State
    juce::File        selectedFile;
    juce::ChildProcess dfu;
    bool              dfuRunning = false;

    //==============================================================================
    void buttonClicked(juce::Button* b) override
    {
        if (b == &browseButton)   showFilePicker();
        else if (b == &flashButton)
        {
            if (!selectedFile.existsAsFile())
            {
                outputBox.insertTextAtCaret("--- ERROR: No file selected ---\n");
                return;
            }
            if (!selectedFile.hasFileExtension(".bin"))
            {
                outputBox.insertTextAtCaret("--- ERROR: Please select a .bin file ---\n");
                return;
            }
            flashSelectedFile();
        }
    }

    void showFilePicker()
    {
        auto chooser = std::make_shared<juce::FileChooser>(
            "Select a binary to flash",
            juce::File(), "*.bin");

        chooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser](const juce::FileChooser& fc)
            {
                auto selected = fc.getResult();
                if (selected != juce::File())
                {
                    selectedFile = selected;
                    fileLabel.setText(selectedFile.getFullPathName(),
                        juce::dontSendNotification);
                    outputBox.insertTextAtCaret("Selected: "
                        + selectedFile.getFullPathName() + "\n");
                }
                else
                {
                    outputBox.insertTextAtCaret("--- No file selected ---\n");
                }
            });
    }

    juce::String getDfuExeQuoted() const
    {
        auto path = dfuPathEditor.getText().trim();
        if (path.isNotEmpty())
            return path.quoted();
        return juce::String("dfu-util");
    }

    void flashSelectedFile()
    {
        outputBox.insertTextAtCaret("--- Flashing: " + selectedFile.getFileName() + " ---\n");
        flashButton.setEnabled(false);

        juce::String exe = getDfuExeQuoted();

        juce::String binPath = selectedFile.getFullPathName();
        binPath = binPath.replaceCharacter('\\', '/');
        juce::String cmd = exe + " -a 0 -s 0x08000000:leave -D " + binPath.quoted();

        if (!dfu.start(cmd))
        {
            outputBox.insertTextAtCaret("--- ERROR: Could not start dfu-util ---\n");
            flashButton.setEnabled(true);
            return;
        }

        dfuRunning = true;
    }

    void timerCallback() override
    {
        updateDFUStatus();

        if (dfuRunning)
        {
            char buf[2048];
            int n = dfu.readProcessOutput(buf, sizeof(buf) - 1);
            if (n > 0)
                outputBox.insertTextAtCaret(makeSafeConsoleText(buf, n));

            if (!dfu.isRunning())
            {
                while ((n = dfu.readProcessOutput(buf, sizeof(buf) - 1)) > 0)
                    outputBox.insertTextAtCaret(makeSafeConsoleText(buf, n));

                outputBox.insertTextAtCaret("--- dfu-util finished ---\n");
                dfuRunning = false;
                flashButton.setEnabled(true);
            }
        }
    }

    void updateDFUStatus()
    {
        juce::ChildProcess probe;
        juce::String exe = getDfuExeQuoted();
        if (probe.start(exe + " -l"))
        {
            auto out = probe.readAllProcessOutput();
            bool found = out.containsIgnoreCase("Found DFU");
            statusText.setText(found ? "Protoseed in DFU mode" : "Protoseed not detected",
                juce::dontSendNotification);
            statusText.setColour(juce::Label::textColourId,
                found ? juce::Colours::green : juce::Colours::red);
            statusLamp.setOn(found);
            return;
        }

        statusText.setText("dfu-util not found / not launching", juce::dontSendNotification);
        statusText.setColour(juce::Label::textColourId, juce::Colours::orange);
        statusLamp.setOn(false);
    }

    void saveLog()
    {
        auto fc = std::make_shared<juce::FileChooser>("Save log", juce::File(), "*.txt");
        fc->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this, fc](const juce::FileChooser& c)
            {
                auto f = c.getResult();
                if (f != juce::File())
                {
                    f.replaceWithText(outputBox.getText());
                    outputBox.insertTextAtCaret("Saved log: " + f.getFullPathName() + "\n");
                }
            });
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

//==============================================================================
// Main app window
class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow()
        : DocumentWindow("HSP Protoseed Flasher",
            juce::Desktop::getInstance().getDefaultLookAndFeel()
            .findColour(juce::ResizableWindow::backgroundColourId),
            DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setContentOwned(new MainComponent(), true);
        centreWithSize(getWidth(), getHeight());
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

//==============================================================================
// Application entry point
class FlashApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName()    override { return "Protoseed Flasher"; }
    const juce::String getApplicationVersion() override { return "1.1"; }
    bool moreThanOneInstanceAllowed()          override { return true; }

    void initialise(const juce::String&) override { mainWindow.reset(new MainWindow()); }
    void shutdown()                         override { mainWindow = nullptr; }
    void systemRequestedQuit()              override { quit(); }

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(FlashApp)
