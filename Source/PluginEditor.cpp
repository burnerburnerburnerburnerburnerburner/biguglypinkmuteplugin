#include "PluginEditor.h"

namespace
{
    namespace Palette
    {
        const juce::Colour deep   { 0xff603c5a };
        const juce::Colour plum   { 0xff764576 };
        const juce::Colour mauve  { 0xff82507a };
        const juce::Colour glass  { 0xff995f8f };
        const juce::Colour lilac  { 0xffa979a1 };
        const juce::Colour blush  { 0xffbc96b6 };
        const juce::Colour violet { 0xff52298e };
        const juce::Colour indigo { 0xff150f76 };
        const juce::Colour glow   { 0xfff7c9ec };
    }

    constexpr int windowWidth = 740;
    constexpr int windowHeight = 369;

    juce::Font uiFont (float height)
    {
        return juce::Font (juce::FontOptions ("Segoe UI", height, juce::Font::bold)).withExtraKerningFactor (0.14f);
    }

    // The illuminated ring runs around the outside, the glass dial sits inside it.
    struct DialGeometry
    {
        juce::Point<float> centre;
        float ringRadius, ringThickness;
        juce::Rectangle<float> glassCircle;
    };

    DialGeometry dialGeometry (juce::Rectangle<float> dialBounds)
    {
        auto outerR = juce::jmin (dialBounds.getWidth(), dialBounds.getHeight()) * 0.47f;
        auto ringR = outerR * 0.84f;
        auto thickness = outerR * 0.11f;
        auto glassD = (ringR - thickness * 1.5f) * 2.0f;
        return { dialBounds.getCentre(), ringR, thickness,
                 juce::Rectangle<float> (glassD, glassD).withCentre (dialBounds.getCentre()) };
    }

    void drawShadow (juce::Graphics& g, const juce::Path& shape, float strength = 1.0f)
    {
        auto h = shape.getBounds().getHeight();
        juce::DropShadow (Palette::indigo.withAlpha (0.32f * strength),
                          (int) (h * 0.10f) + 4,
                          { 0, (int) (h * 0.05f) + 2 })
            .drawForPath (g, shape);
    }

    // Frosted, see-through body with a bright top specular, light pooling at the
    // bottom like refraction, and a thin rim catching the light.
    void drawGlass (juce::Graphics& g, const juce::Path& shape, float lift = 1.0f)
    {
        auto b = shape.getBounds();
        auto white = juce::Colours::white;

        {
            juce::Graphics::ScopedSaveState state (g);
            g.reduceClipRegion (shape);

            g.setGradientFill ({ white.withAlpha (0.24f * lift), b.getCentreX(), b.getY(),
                                 Palette::glass.withAlpha (0.16f), b.getCentreX(), b.getBottom(), false });
            g.fillPath (shape);

            g.setGradientFill ({ Palette::glow.withAlpha (0.38f * lift), b.getCentreX(), b.getBottom(),
                                 Palette::glow.withAlpha (0.0f), b.getCentreX(), b.getBottom() - b.getHeight() * 0.5f, true });
            g.fillRect (b);

            auto highlight = b.reduced (b.getWidth() * 0.13f, 0.0f)
                                 .withHeight (b.getHeight() * 0.48f)
                                 .translated (0.0f, b.getHeight() * 0.03f);
            g.setGradientFill ({ white.withAlpha (juce::jmin (0.9f, 0.52f * lift)), highlight.getCentreX(), highlight.getY(),
                                 white.withAlpha (0.0f), highlight.getCentreX(), highlight.getBottom(), false });
            g.fillEllipse (highlight);
        }

        juce::ColourGradient rim (white.withAlpha (0.85f), b.getX(), b.getY(),
                                  white.withAlpha (0.45f), b.getRight(), b.getBottom(), false);
        rim.addColour (0.5, white.withAlpha (0.06f));
        g.setGradientFill (rim);
        g.strokePath (shape, juce::PathStrokeType (juce::jlimit (1.0f, 1.6f, b.getHeight() * 0.008f)));
    }

    juce::Path ellipsePath (juce::Rectangle<float> r)
    {
        juce::Path p;
        p.addEllipse (r);
        return p;
    }

    juce::Path pillPath (juce::Rectangle<float> r)
    {
        juce::Path p;
        p.addRoundedRectangle (r, juce::jmin (r.getWidth(), r.getHeight()) * 0.5f);
        return p;
    }

    void fillGlowSpot (juce::Graphics& g, juce::Point<float> centre, float radius, juce::Colour colour)
    {
        g.setGradientFill ({ colour, centre, colour.withAlpha (0.0f), centre.translated (radius, 0.0f), true });
        g.fillEllipse (juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (centre));
    }

    // Layout, in window pixels.
    const juce::Rectangle<float> panelBounds  { 14.0f, 14.0f, 712.0f, 341.0f };
    const juce::Rectangle<float> headerBounds { 98.0f, 40.0f, 520.0f, 48.0f };
    const juce::Rectangle<int>   dialBounds   { 106, 110, 150, 150 };
    const juce::Rectangle<int>   percentBounds { 131, 266, 100, 24 };
    const juce::Rectangle<int>   pillBounds   { 300, 163, 270, 44 };
}

//==============================================================================
void GlassLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float startAngle, float endAngle, juce::Slider&)
{
    auto geometry = dialGeometry (juce::Rectangle<int> (x, y, width, height).toFloat());
    auto centre = geometry.centre;
    auto trackR = geometry.ringRadius;
    auto thickness = geometry.ringThickness;

    drawGlass (g, ellipsePath (geometry.glassCircle), 0.8f + 0.2f * onAmount);

    auto rounded = [] (float w) { return juce::PathStrokeType (w, juce::PathStrokeType::curved, juce::PathStrokeType::rounded); };

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, trackR, trackR, 0.0f, startAngle, endAngle, true);
    g.setColour (Palette::indigo.withAlpha (0.20f));
    g.strokePath (track, rounded (thickness));

    auto angle = startAngle + sliderPos * (endAngle - startAngle);

    // The illuminated ring: a soft halo, the pink body, then a hot core.
    if (sliderPos > 0.0f)
    {
        juce::Path value;
        value.addCentredArc (centre.x, centre.y, trackR, trackR, 0.0f, startAngle, angle, true);

        g.setColour (Palette::glow.withAlpha (0.22f * onAmount));
        g.strokePath (value, rounded (thickness * 2.6f));
        g.setColour (Palette::glow.withAlpha (0.30f * onAmount));
        g.strokePath (value, rounded (thickness * 1.7f));

        g.setColour (Palette::lilac.interpolatedWith (juce::Colour (0xfff2a6dd), onAmount));
        g.strokePath (value, rounded (thickness));

        g.setColour (juce::Colours::white.withAlpha (0.65f * onAmount));
        g.strokePath (value, rounded (thickness * 0.32f));
    }

    // The 50% threshold: a small bead at twelve o'clock.
    auto threshold = centre.getPointOnCircumference (trackR, (startAngle + endAngle) * 0.5f);
    g.setColour (juce::Colours::white.withAlpha (0.85f));
    g.fillEllipse (juce::Rectangle<float> (thickness * 0.4f, thickness * 0.4f).withCentre (threshold));

    // Glass pill riding the ring at the current position, pointing outwards.
    auto thumb = centre.getPointOnCircumference (trackR, angle);
    auto thumbLength = thickness * 2.9f;
    auto thumbWidth = thickness * 1.7f;

    fillGlowSpot (g, thumb.translated (0.0f, thumbWidth * 0.25f), thumbLength * 0.75f, Palette::indigo.withAlpha (0.35f));
    fillGlowSpot (g, thumb, thumbLength * 0.9f, Palette::glow.withAlpha (0.45f * onAmount));

    juce::Graphics::ScopedSaveState state (g);
    g.addTransform (juce::AffineTransform::rotation (angle, thumb.x, thumb.y));

    auto thumbShape = pillPath (juce::Rectangle<float> (thumbWidth, thumbLength).withCentre (thumb));
    g.setColour (Palette::blush.withAlpha (0.35f));
    g.fillPath (thumbShape);
    drawGlass (g, thumbShape, 1.4f);
}

//==============================================================================
StatusPill::StatusPill()
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void StatusPill::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    drawGlass (g, pillPath (b), isMouseOver() ? 1.15f : 0.9f);

    // Status orb on the left.
    auto orbD = b.getHeight() * 0.44f;
    juce::Point<float> orbCentre (b.getX() + b.getHeight() * 0.55f, b.getCentreY());
    auto orb = juce::Rectangle<float> (orbD, orbD).withCentre (orbCentre);

    fillGlowSpot (g, orbCentre, orbD * 1.25f, Palette::glow.withAlpha (0.75f * onAmount));

    auto orbColour = Palette::plum.interpolatedWith (juce::Colour (0xffef8fd6), onAmount);
    g.setGradientFill ({ juce::Colours::white.withAlpha (0.35f + 0.6f * onAmount), orb.getX() + orbD * 0.3f, orb.getY() + orbD * 0.25f,
                         orbColour, orb.getRight(), orb.getBottom(), true });
    g.fillEllipse (orb);
    g.setColour (juce::Colours::white.withAlpha (0.6f));
    g.drawEllipse (orb, 0.8f);

    auto textArea = b.withTrimmedLeft (b.getHeight() * 1.1f).withTrimmedRight (b.getHeight() * 0.5f);
    g.setColour (isOn ? Palette::indigo : Palette::violet);
    g.setFont (uiFont (b.getHeight() * 0.36f));
    g.drawText (isOn ? "UNMUTED" : "MUTED", textArea, juce::Justification::centred, false);
}

void StatusPill::mouseUp (const juce::MouseEvent& e)
{
    if (e.mouseWasClicked() && onClick != nullptr)
        onClick();
}

//==============================================================================
GlassMuteEditor::GlassMuteEditor (GlassMuteProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    dial.setLookAndFeel (&glassLookAndFeel);
    dial.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    dial.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    dial.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
    dial.setMouseDragSensitivity (220);
    dial.onValueChange = [this] { updatePercentLabel(); };
    addAndMakeVisible (dial);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, GlassMuteProcessor::onOffId, dial);
    dial.setDoubleClickReturnValue (true, 1.0);

    percentLabel.setFont (uiFont (17.0f));
    percentLabel.setJustificationType (juce::Justification::centred);
    percentLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (percentLabel);

    pill.onClick = [this]
    {
        auto* param = audioProcessor.onOffParam;
        param->beginChangeGesture();
        param->setValueNotifyingHost (audioProcessor.isOn() ? 0.0f : 1.0f);
        param->endChangeGesture();
    };
    addAndMakeVisible (pill);

    onAmount = audioProcessor.isOn() ? 1.0f : 0.0f;
    glassLookAndFeel.onAmount = pill.onAmount = onAmount;
    pill.isOn = audioProcessor.isOn();
    updatePercentLabel();

    setResizable (false, false);
    setSize (windowWidth, windowHeight);

    startTimerHz (60);
}

GlassMuteEditor::~GlassMuteEditor()
{
    dial.setLookAndFeel (nullptr);
}

void GlassMuteEditor::updatePercentLabel()
{
    percentLabel.setText (juce::String (juce::roundToInt (dial.getValue() * 100.0)) + "%", juce::dontSendNotification);
    percentLabel.setColour (juce::Label::textColourId, audioProcessor.isOn() ? Palette::indigo : Palette::violet);
}

void GlassMuteEditor::timerCallback()
{
    auto on = audioProcessor.isOn();

    if (on != pill.isOn)
    {
        pill.isOn = on;
        pill.repaint();
        updatePercentLabel();
    }

    auto target = on ? 1.0f : 0.0f;

    if (onAmount == target)
        return;

    onAmount += (target - onAmount) * 0.2f;

    if (std::abs (target - onAmount) < 0.002f)
        onAmount = target;

    glassLookAndFeel.onAmount = pill.onAmount = onAmount;
    dial.repaint();
    pill.repaint();
}

void GlassMuteEditor::resized()
{
    dial.setBounds (dialBounds);
    percentLabel.setBounds (percentBounds);
    pill.setBounds (pillBounds);
    background = {};
}

void GlassMuteEditor::paint (juce::Graphics& g)
{
    auto scale = g.getInternalContext().getPhysicalPixelScaleFactor();

    if (background.isNull() || ! juce::approximatelyEqual (scale, backgroundScale))
        renderBackground (scale);

    g.drawImage (background, getLocalBounds().toFloat());
}

void GlassMuteEditor::renderBackground (float pixelScale)
{
    backgroundScale = pixelScale;
    background = juce::Image (juce::Image::ARGB,
                              juce::jmax (1, juce::roundToInt ((float) getWidth() * pixelScale)),
                              juce::jmax (1, juce::roundToInt ((float) getHeight() * pixelScale)),
                              true);

    juce::Graphics g (background);
    g.addTransform (juce::AffineTransform::scale (pixelScale));

    auto w = (float) getWidth();
    auto h = (float) getHeight();

    g.setGradientFill ({ Palette::mauve, 0.0f, 0.0f, Palette::deep, w, h, false });
    g.fillAll();

    // Soft blooms of colour for the glass to sit over.
    fillGlowSpot (g, { w * 0.12f, h * 0.20f }, 260.0f, Palette::blush.withAlpha (0.55f));
    fillGlowSpot (g, { w * 0.92f, h * 0.55f }, 280.0f, Palette::violet.withAlpha (0.25f));
    fillGlowSpot (g, { w * 0.45f, h * 1.05f }, 240.0f, Palette::indigo.withAlpha (0.16f));
    fillGlowSpot (g, { w * 0.70f, h * 0.02f }, 200.0f, Palette::glow.withAlpha (0.25f));
    fillGlowSpot (g, { w * 0.28f, h * 0.70f }, 150.0f, Palette::lilac.withAlpha (0.35f));

    // Faint bubbles drifting behind the panel.
    g.beginTransparencyLayer (0.35f);

    for (auto [x, y, d] : { std::tuple { 0.05f, 0.30f, 34.0f },
                            std::tuple { 0.90f, 0.18f, 22.0f },
                            std::tuple { 0.84f, 0.74f, 46.0f },
                            std::tuple { 0.04f, 0.86f, 18.0f },
                            std::tuple { 0.95f, 0.50f, 12.0f },
                            std::tuple { 0.66f, 0.86f, 26.0f },
                            std::tuple { 0.44f, 0.80f, 14.0f },
                            std::tuple { 0.97f, 0.92f, 20.0f } })
    {
        auto bubble = ellipsePath (juce::Rectangle<float> (d, d).withCentre ({ w * x, h * y }));
        drawGlass (g, bubble, 1.0f);
    }

    g.endTransparencyLayer();

    // The bordered glass panel everything sits in, with a fine frosted grain.
    juce::Path panel;
    panel.addRoundedRectangle (panelBounds, 26.0f);
    drawShadow (g, panel, 0.6f);

    {
        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (panel);

        auto white = juce::Colours::white;
        g.setGradientFill ({ white.withAlpha (0.12f), 0.0f, panelBounds.getY(),
                             white.withAlpha (0.03f), 0.0f, panelBounds.getBottom(), false });
        g.fillPath (panel);

        auto sheen = panelBounds.withHeight (46.0f);
        g.setGradientFill ({ white.withAlpha (0.14f), 0.0f, sheen.getY(), white.withAlpha (0.0f), 0.0f, sheen.getBottom(), false });
        g.fillRect (sheen);

        juce::Random grain (0x6c55);

        for (int i = 0; i < 9000; ++i)
        {
            auto px = panelBounds.getX() + grain.nextFloat() * panelBounds.getWidth();
            auto py = panelBounds.getY() + grain.nextFloat() * panelBounds.getHeight();
            g.setColour ((grain.nextBool() ? juce::Colours::white : Palette::indigo).withAlpha (0.05f));
            g.fillRect (px, py, 1.0f, 1.0f);
        }
    }

    juce::ColourGradient rim (juce::Colours::white.withAlpha (0.75f), panelBounds.getX(), panelBounds.getY(),
                              juce::Colours::white.withAlpha (0.35f), panelBounds.getRight(), panelBounds.getBottom(), false);
    rim.addColour (0.5, juce::Colours::white.withAlpha (0.12f));
    g.setGradientFill (rim);
    g.strokePath (panel, juce::PathStrokeType (1.4f));

    // Header capsule.
    auto header = pillPath (headerBounds);
    drawShadow (g, header, 0.6f);
    drawGlass (g, header, 0.85f);
    g.setColour (Palette::indigo);
    g.setFont (uiFont (17.0f));
    g.drawText ("BIG UGLY PINK MUTE PLUGIN", headerBounds, juce::Justification::centred, false);

    // Shadows for the live controls, which repaint over the top of this.
    drawShadow (g, ellipsePath (dialGeometry (dialBounds.toFloat()).glassCircle));
    drawShadow (g, pillPath (pillBounds.toFloat()), 0.8f);
}
