#include "palette.h"
#include "debug.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

// palette definitions are taken from
// https://github.com/kbinani/colormap-shaders

float Palette::clamp(const float _v, const float _min, const float _max) {
    if (_v < _min) {
        return _min;
    } else if (_max < _v) {
        return _max;
    } else {
        return _v;
    }
}

void BlackWhite::setColor(const float _val, const float _scale, unsigned char *_map) {
    float v = clamp(_val, 0.0, 1.0);

    *(_map + 0)= (unsigned char)(v * _scale);
    *(_map + 1)= (unsigned char)(v * _scale);
    *(_map + 2)= (unsigned char)(v * _scale);
    *(_map + 3)= (unsigned char)_scale;
}

void Cool::setColor(const float _val, const float _scale, unsigned char *_map) {
    float r = (1.0 + 1.0 / 63.0) * _val - 1.0 / 63.0;
    float g = -(1.0 + 1.0 / 63.0) * _val + (1.0 + 1.0 / 63.0);

    *(_map + 0)= (unsigned char)(clamp(r, 0.0, 1.0) * _scale);
    *(_map + 1)= (unsigned char)(clamp(g, 0.0, 1.0) * _scale);
    *(_map + 2)= (unsigned char)(1.0 * _scale);
    *(_map + 3)= (unsigned char)_scale;
}

void Hot::setColor(const float _val, const float _scale, unsigned char *_map) {
    float r = (8.0 / 3.0 * _val);
    float g = (8.0 / 3.0 * _val - 1.0);
    float b = (4.0 * _val - 3.0);

    *(_map + 0)= (unsigned char)(clamp(r, 0.0, 1.0) * _scale);
    *(_map + 1)= (unsigned char)(clamp(g, 0.0, 1.0) * _scale);
    *(_map + 2)= (unsigned char)(clamp(b, 0.0, 1.0) * _scale);
    *(_map + 3)= (unsigned char)_scale;
}

void Jet::setColor(const float _val, const float _scale, unsigned char *_map) {
    float r;
    if (_val < 0.7) {
        r = 4.0 * _val - 1.5;
    } else {
        r = -4.0 * _val + 4.5;
    }

    float g;
    if (_val < 0.5) {
        g = 4.0 * _val - 0.5;
    } else {
        g = -4.0 * _val + 3.5;
    }

    float b;
    if (_val < 0.3) {
       b = 4.0 * _val + 0.5;
    } else {
       b = -4.0 * _val + 2.5;
    }

    *(_map + 0)= (unsigned char)(clamp(r, 0.0, 1.0) * _scale);
    *(_map + 1)= (unsigned char)(clamp(g, 0.0, 1.0) * _scale);
    *(_map + 2)= (unsigned char)(clamp(b, 0.0, 1.0) * _scale);
    *(_map + 3)= (unsigned char)_scale;
}

void PurpleBlue::setColor(const float _val, const float _scale, unsigned char *_map) {
    float r;
    if (_val < 0.7520372909206926) {
        r = (((9.68615208861418E+02 * _val - 1.16097242960380E+03) * _val + 1.06173672031378E+02) * _val - 1.68616613530379E+02) * _val + 2.56073136099945E+02;
    } else {
        r = -1.20830453148990E+01 * _val + 1.44337397593436E+01;
    }

    float g;
    if (_val < 0.7485333535031721) {
        g = (((-4.58537247030064E+02 * _val + 5.67323181593790E+02) * _val - 2.56714665792882E+02) * _val - 1.14205365680507E+02) * _val + 2.47073841488433E+02;
    } else {
        g = ((-2.99774273328017E+02 * _val + 4.12147041403012E+02) * _val - 2.49880079288168E+02) * _val + 1.93578601034431E+02;
    }

    float b;
    if (_val < 0.7628468501376879) {
        b = ((-5.44257972228224E+01 * _val + 2.70890554876532E+01) * _val - 9.12766750739247E+01) * _val + 2.52166182860177E+02;
    } else {
        b = (((4.55621137729287E+04 * _val - 1.59960900638524E+05) * _val + 2.09530452721547E+05) * _val - 1.21704642900945E+05) * _val + 2.66644674068694E+04;
    }

    *(_map + 0)= (unsigned char)(clamp(r / 255.0, 0.0, 1.0) * _scale);
    *(_map + 1)= (unsigned char)(clamp(g / 255.0, 0.0, 1.0) * _scale);
    *(_map + 2)= (unsigned char)(clamp(b / 255.0, 0.0, 1.0) * _scale);
    *(_map + 3)= (unsigned char)_scale;
}

void HotMetal::setColor(const float _val, const float _scale, unsigned char *_map) {
    float r;
    if (_val < 0.0) {
        r = 0.0;
    } else if (_val <= 0.57147) {
        r = 446.22 * _val / 255.0;
    } else {
       r = 1.0;
    }

    float g;
    if (_val < 0.6) {
        g = 0.0;
    } else if (_val <= 0.95) {
        g = ((_val - 0.6) * 728.57) / 255.0;
    } else {
        g = 1.0;
    }

    float b = 0.0;

    *(_map + 0)= (unsigned char)(r * _scale);
    *(_map + 1)= (unsigned char)(g * _scale);
    *(_map + 2)= (unsigned char)(b * _scale);
    *(_map + 3)= (unsigned char)_scale;
}

void Autumn::setColor(const float _val, const float _scale, unsigned char *_map) {
    float r = 1.0;
    float g = _val;
    float b = 0.0;

    *(_map + 0)= (unsigned char)(r * _scale);
    *(_map + 1)= (unsigned char)(clamp(g, 0.0, 1.0) * _scale);
    *(_map + 2)= (unsigned char)(b * _scale);
    *(_map + 3)= (unsigned char)_scale;
}

void Spring::setColor(const float _val, const float _scale, unsigned char *_map) {
    float r = 1.0;
    float g = _val;
    float b = 1.0 - _val;

    *(_map + 0)= (unsigned char)(r * _scale);
    *(_map + 1)= (unsigned char)(clamp(g, 0.0, 1.0) * _scale);
    *(_map + 2)= (unsigned char)(clamp(b, 0.0, 1.0) * _scale);
    *(_map + 3)= (unsigned char)_scale;
}

void Summer::setColor(const float _val, const float _scale, unsigned char *_map) {
    float r = _val;
    float g = 0.5 * _val + 0.5;
    float b = 0.4;

    *(_map + 0)= (unsigned char)(clamp(r, 0.0, 1.0) * _scale);
    *(_map + 1)= (unsigned char)(clamp(g, 0.0, 1.0) * _scale);
    *(_map + 2)= (unsigned char)(b * _scale);
    *(_map + 3)= (unsigned char)_scale;
}

void Winter::setColor(const float _val, const float _scale, unsigned char *_map) {
    float r = 0.0;
    float g = _val;
    float b = -0.5 * _val + 1.0;

    *(_map + 0)= (unsigned char)(r * _scale);
    *(_map + 1)= (unsigned char)(clamp(g, 0.0, 1.0) * _scale);
    *(_map + 2)= (unsigned char)(clamp(b, 0.0, 1.0) * _scale);
    *(_map + 3)= (unsigned char)_scale;
}

PaletteList::PaletteList() {
    items.clear();
    map = NULL;
    mapSize = 0;

    items.push_back(new BlackWhite());
    items.push_back(new Cool());
    items.push_back(new Hot());
    items.push_back(new Jet());
    items.push_back(new PurpleBlue());
    items.push_back(new HotMetal());
    items.push_back(new Autumn());
    items.push_back(new Spring());
    items.push_back(new Summer());
    items.push_back(new Winter());
}

PaletteList::~PaletteList() {
    if (map) {
        free(map);
    }
    mapSize = 0;
    for (size_t i = 0; i < items.size(); i++) {
        delete items[i];
    }
}

const char * PaletteList::getName(const unsigned int _index) {
    assert(_index < items.size());
    return items[_index]->name;
}

unsigned int PaletteList::getCount(void) {
    return (unsigned int)items.size();
}

unsigned char * PaletteList::generate(const char *_name, const unsigned int _size) {
    for (size_t i = 0; i < items.size(); i++) {
        Palette *pal = items[i];
        if (strncmp(_name, pal->name, strlen(_name)) == 0) {
            // allocate/reallocate palette
            if (map == NULL) {
                map = (unsigned char *)malloc(_size * 4);
            } else {
                if (mapSize < _size) {
                    map = (unsigned char *)realloc(map, _size * 4);
                }
            }
            mapSize = _size;

            float val;
            float scale = (float)mapSize - 1.0;
            for (unsigned int i = 0; i < mapSize; i++) {
                val = (float)i / scale;
                pal->setColor(val, scale, &map[i * 4]);
                // D("%s RGBA %3d %3d %3d %3d\n", pal->name, map[i * 4 + 0], map[i * 4 + 1], map[i * 4 + 2], map[i * 4 + 3]);
            }
            D("selected palette %s, size %d\n", pal->name, mapSize);
            return map;
        }
    }

    assert(1 != 1);
    // palette not found
    return NULL;
}
