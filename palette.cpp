#include "palette.h"
#include "debug.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

Palette::Palette(const char *_name) {
    name = _name;
}

Palette::~Palette() {

}

float Palette::clamp(const float _v, const float _min, const float _max) {
    if (_v < _min) {
        return _min;
    } else if (_max < _v) {
        return _max;
    } else {
        return _v;
    }
}

Jet::Jet() : Palette("Jet") {}
Jet::~Jet() {}
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


PaletteList::PaletteList() {
    items.clear();
    map = NULL;
    mapSize = 0;

    items.push_back(new Jet());
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
