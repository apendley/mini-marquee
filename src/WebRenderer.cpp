#include "WebRenderer.h"
#include <ESPStringTemplate.h>

// Only include in this file
#include "html/index_html.h"

// Uncomment to print logs in this file to the serial console.
//#define LOGGER Serial
#include "Logger.h"

namespace {
    // Definitions for all of the substitutions that will be made.
    struct ColorTokenGroup {
        static constexpr int tokenCount = 3;
        const char* selected;
        const char* name;
        const char* hexString;
    };

    struct ValueTokenGroup {
        static constexpr int tokenCount = 2;    
        const char* selected;
        const char* name;
    };

    const ColorTokenGroup colorTokenGroups[/*settings.colors.count()*/] = {
        {"{{CS0}}", "{{CN0}}", "{{C0}}"},
        {"{{CS1}}", "{{CN1}}", "{{C1}}"},
        {"{{CS2}}", "{{CN2}}", "{{C2}}"},
        {"{{CS3}}", "{{CN3}}", "{{C3}}"},
        {"{{CS4}}", "{{CN4}}", "{{C4}}"},
        {"{{CS5}}", "{{CN5}}", "{{C5}}"},
        {"{{CS6}}", "{{CN6}}", "{{C6}}"},
        {"{{CS7}}", "{{CN7}}", "{{C7}}"},
        {"{{CS8}}", "{{CN8}}", "{{C8}}"},
        {"{{CS9}}", "{{CN9}}", "{{C9}}"},
    };

    const ValueTokenGroup brightnessTokenGroups[/*settings.brightnessValues.count()*/] = {
        {"{{BS0}}", "{{BN0}}"},
        {"{{BS1}}", "{{BN1}}"},
        {"{{BS2}}", "{{BN2}}"},
        {"{{BS3}}", "{{BN3}}"},
        {"{{BS4}}", "{{BN4}}"},
    };

    const ValueTokenGroup textSpeedTokenGroups[/*settings.scrollDelays.count()*/] = {
        {"{{TS0}}", "{{TN0}}"},
        {"{{TS1}}", "{{TN1}}"},
        {"{{TS2}}", "{{TN2}}"},
        {"{{TS3}}", "{{TN3}}"},
        {"{{TS4}}", "{{TN4}}"},
    };

    const ValueTokenGroup fontTokenGroups[/*settings.fonts.count()*/] = {
        {"{{FS0}}", "{{FN0}}"},
        {"{{FS1}}", "{{FN1}}"},
        {"{{FS2}}", "{{FN2}}"},
        {"{{FS3}}", "{{FN3}}"},
    };

    const ValueTokenGroup rotationTokenGroups[/*settings.displayRotations.count()*/] = {
        {"{{RS0}}", "{{RN0}}"},
        {"{{RS1}}", "{{RN1}}"},
        {"{{RS2}}", "{{RN2}}"},
        {"{{RS3}}", "{{RN3}}"},
    };

    const char*  backgroundColorToken = "{{BGCOLOR}}";
}

void WebRenderer::render() {
    if (dirty == false) {
        return;
    }

    LOGFMT("index.html size: %d\n\r", sizeof(index_html));    

    ESPStringTemplate page(renderedDocument, renderedDocumentSize);

    const size_t subTokensCount = 1 // backgroundColorToken
                                + settings.colors.count() * ColorTokenGroup::tokenCount
                                + settings.brightnessValues.count() * ValueTokenGroup::tokenCount
                                + settings.scrollDelays.count() * ValueTokenGroup::tokenCount
                                + settings.fonts.count() * ValueTokenGroup::tokenCount
                                + settings.displayRotations.count() * ValueTokenGroup::tokenCount;

    TokenStringPair subs[subTokensCount];
    int currentSubToken = 0;

    static const char* selectedValue = " selected";
    static const char* notSelectedValue = "";

    subs[currentSubToken++].setPair(backgroundColorToken, settings.colors.current().hexString);

    for (int i = 0; i < settings.colors.count(); i++) {
        // Set the selected modifier for only the selected item
        const char* value = (i == settings.colors.currentIndex()) ? selectedValue : notSelectedValue;
        subs[currentSubToken++].setPair(colorTokenGroups[i].selected, value);

        // Now set the name for each item
        subs[currentSubToken++].setPair(colorTokenGroups[i].name, settings.colors.get(i).name);

        // Now set the hex color value for each item
        subs[currentSubToken++].setPair(colorTokenGroups[i].hexString, settings.colors.get(i).hexString);        
    }    

    for (int i = 0; i < settings.brightnessValues.count(); i++) {
        // Set the selected modifier for only the selected item
        const char* value = (i == settings.brightnessValues.currentIndex()) ? selectedValue : notSelectedValue;
        subs[currentSubToken++].setPair(brightnessTokenGroups[i].selected, value);

        // Now set the name for each item
        subs[currentSubToken++].setPair(brightnessTokenGroups[i].name, settings.brightnessValues.get(i).name);
    }

    for (int i = 0; i < settings.scrollDelays.count(); i++) {
        // Set the selected modifier for only the selected item
        const char* value = (i == settings.scrollDelays.currentIndex()) ? selectedValue : notSelectedValue;
        subs[currentSubToken++].setPair(textSpeedTokenGroups[i].selected, value);

        // Now set the name for each item
        subs[currentSubToken++].setPair(textSpeedTokenGroups[i].name, settings.scrollDelays.get(i).name);
    }    

    for (int i = 0; i < settings.fonts.count(); i++) {
        // Set the selected modifier for only the selected item
        const char* value = (i == settings.fonts.currentIndex()) ? selectedValue : notSelectedValue;
        subs[currentSubToken++].setPair(fontTokenGroups[i].selected, value);

        // Now set the name for each item
        subs[currentSubToken++].setPair(fontTokenGroups[i].name, settings.fonts.get(i).name);
    }

    for (int i = 0; i < settings.displayRotations.count(); i++) {
        // Set the selected modifier for only the selected item
        const char* value = (i == settings.displayRotations.currentIndex()) ? selectedValue : notSelectedValue;
        subs[currentSubToken++].setPair(rotationTokenGroups[i].selected, value);

        // Now set the name for each item
        subs[currentSubToken++].setPair(rotationTokenGroups[i].name, settings.displayRotations.get(i).name);
    }    

    page.add(index_html, subs, subTokensCount);

    // LOGLN("\n\rRendered web page:");
    // LOGLN(renderedDocument);    
    LOGFMT("rendered web page size: %d\n\r", strlen(renderedDocument) + 1);

    dirty = false;
}