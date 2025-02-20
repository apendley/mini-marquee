#pragma once

#include <Arduino.h>
#include "Settings.h"


class WebRenderer {
public:
    WebRenderer(Settings& _settings) :
        settings(_settings)
    {

    }

    void setDirty() {
        dirty = true;
    }

    bool isDirty() const {
        return dirty;
    }

    void render();

    const char* getRenderedDocument() const {
        return renderedDocument;
    }

private:
    static constexpr size_t renderedDocumentSize = 4 * 1024;
    char renderedDocument[renderedDocumentSize] = {0};

    Settings& settings;

    bool dirty = true;
};