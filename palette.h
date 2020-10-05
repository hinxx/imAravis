#ifndef PALETTE_H
#define PALETTE_H

#include <vector>

struct Palette {
    const char *name;

    Palette(const char *_name) {
        name = _name;
    }
    virtual ~Palette() {}
    virtual void setColor(const float _val, const float _scale, unsigned char *_map) = 0;
    float clamp(const float _v, const float _min, const float _max);
};

struct BlackWhite: Palette {
    BlackWhite() : Palette("Black White") {}
    ~BlackWhite() {}
    void setColor(const float _val, const float _scale, unsigned char *_map);
};

struct Cool : Palette {
    Cool() : Palette("Cool") {}
    ~Cool() {}
    void setColor(const float _val, const float _scale, unsigned char *_map);
};

struct Hot : Palette {
    Hot() : Palette("Hot") {}
    ~Hot() {}
    void setColor(const float _val, const float _scale, unsigned char *_map);
};

struct Jet : Palette {
    Jet() : Palette("Jet") {}
    ~Jet() {}
    void setColor(const float _val, const float _scale, unsigned char *_map);
};

struct PurpleBlue : Palette {
    PurpleBlue() : Palette("Purple Blue") {}
    ~PurpleBlue() {}
    void setColor(const float _val, const float _scale, unsigned char *_map);
};

struct HotMetal : Palette {
    HotMetal() : Palette("Hot Metal") {}
    ~HotMetal() {}
    void setColor(const float _val, const float _scale, unsigned char *_map);
};

struct Autumn : Palette {
    Autumn() : Palette("Autumn") {}
    ~Autumn() {}
    void setColor(const float _val, const float _scale, unsigned char *_map);
};

struct Spring : Palette {
    Spring() : Palette("Spring") {}
    ~Spring() {}
    void setColor(const float _val, const float _scale, unsigned char *_map);
};

struct Summer : Palette {
    Summer() : Palette("Summer") {}
    ~Summer() {}
    void setColor(const float _val, const float _scale, unsigned char *_map);
};

struct Winter : Palette {
    Winter() : Palette("Winter") {}
    ~Winter() {}
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
