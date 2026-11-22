#include "PluginEditor.h"
PsytranceAcidGeneratorAudioProcessorEditor::PsytranceAcidGeneratorAudioProcessorEditor(PsytranceAcidGeneratorAudioProcessor& p)
: AudioProcessorEditor(&p),processor(p)
{
    setSize(1020,620);
    title.setText("PSYTRANCE  /  ACID GENERATOR",juce::dontSendNotification);
    title.setFont(juce::Font(25.0f,juce::Font::bold)); title.setColour(juce::Label::textColourId,juce::Colour(0xff7fffe1));
    addAndMakeVisible(title);
    presetBox.addItemList([&]{juce::StringArray a;for(int i=1;i<=100;++i)a.add("Preset "+juce::String(i).paddedLeft('0',3));return a;}(),1);
    presetBox.setSelectedId(1,juce::dontSendNotification);
    presetBox.onChange=[this]{processor.loadPreset(presetBox.getSelectedId()-1);};
    addAndMakeVisible(presetBox);
    randomButton.onClick=[this]{int n=juce::Random::getSystemRandom().nextInt(100);presetBox.setSelectedId(n+1);};
    addAndMakeVisible(randomButton);
    syncButton.setEnabled(false); syncButton.setTooltip("Sequencer follows DAW host transport and BPM automatically");
    addAndMakeVisible(syncButton);
    for(size_t i=0;i<knobs.size();++i){
        knobs[i]=std::make_unique<Slider>(); knobs[i]->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        knobs[i]->setTextBoxStyle(Slider::TextBoxBelow,false,68,18);
        knobs[i]->setColour(Slider::rotarySliderFillColourId,juce::Colour(0xff00d7b2));
        knobs[i]->setColour(Slider::thumbColourId,juce::Colour(0xffffb84d));
        addAndMakeVisible(*knobs[i]);
        attachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts,paramIDs[i],*knobs[i]);
        labels[i].setText(names[i],juce::dontSendNotification);
        labels[i].setJustificationType(juce::Justification::centred);
        labels[i].setColour(juce::Label::textColourId,juce::Colours::white);
        labels[i].setFont(juce::Font(10.5f,juce::Font::bold));
        addAndMakeVisible(labels[i]);
    }
}
void PsytranceAcidGeneratorAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff10121c));
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff182b38),0,60,juce::Colour(0xff10121c),1020,620,false));
    g.fillRoundedRectangle(12,12,996,596,18);
    g.setColour(juce::Colour(0xff00d7b2)); g.drawRoundedRectangle(12,12,996,596,18,2);
    g.setColour(juce::Colour(0xff263e4a)); g.drawHorizontalLine(72,28,992);
    g.setColour(juce::Colour(0xff9bb9c2)); g.setFont(12);
    g.drawText("HYPER-ACID ENGINE   •   GOA / PSY MODULATION   •   TEMPO-LOCKED FX",30,80,950,22,juce::Justification::centredLeft);
    g.drawText("SEQUENCER / MODULATION",30,345,400,20,juce::Justification::centredLeft);
    for(int s=0;s<16;++s){
      auto x=30+s*52; auto y=378;
      g.setColour(s%4==0?juce::Colour(0xff00a88e):juce::Colour(0xff344a55));
      g.fillRoundedRectangle((float)x,(float)y,42,34,5);
      g.setColour(juce::Colours::white); g.setFont(10); g.drawText(juce::String(s+1),x,y+8,42,16,juce::Justification::centred);
    }
    g.setColour(juce::Colour(0xff78949d));g.setFont(10);
    g.drawText("16-step pattern lane • step vocoder gate • host transport sync",30,421,800,18,juce::Justification::centredLeft);
    g.drawText("HOST SYNC: sequencer clock follows DAW BPM / PPQ. Press PLAY in your DAW to run the pattern.",30,570,950,16,juce::Justification::centredLeft);
}
void PsytranceAcidGeneratorAudioProcessorEditor::resized()
{
    title.setBounds(30,25,520,42); presetBox.setBounds(700,28,150,28);randomButton.setBounds(860,28,110,28);syncButton.setBounds(30,105,125,25);
    const int startX=30,startY=135,w=86,h=95,gap=8;
    for(int i=0;i<20;++i){
      int row=i/10,col=i%10; int x=startX+col*(w+gap);int y=startY+row*105;
      labels[i].setBounds(x,y,w,18);knobs[i]->setBounds(x,y+17,w,76);
    }
}
