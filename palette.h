#ifndef PALETTE_H
#define PALETTE_H

#include <vector>

struct Palette {
    const char *name;

    Palette(const char *_name);
    virtual ~Palette();
    virtual void setColor(const float _val, const float _scale, unsigned char *_map) = 0;
    float clamp(const float _v, const float _min, const float _max);
};

struct Jet : Palette {
    Jet();
    ~Jet();
    void setColor(const float _val, const float _scale, unsigned char *_map);
};

struct PaletteList {
    std::vector<Palette *> items;
    unsigned char *map;
    unsigned int mapSize;

    PaletteList();
    ~PaletteList();
    const char * getName(const unsigned int _index);
    unsigned int getCount(void);
    unsigned char *generate(const char *_name, const unsigned int _size);
};

#endif // PALETTE_H
