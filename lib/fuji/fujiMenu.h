#ifndef _FUJI_MENU_
#define _FUJI_MENU_

#include <stdio.h>
#include "fnFS.h"

#define TNFSMENU_MAX_SIZE 65535
#define TNFSMENU_MAX_DISPLAY_NAME 80
#define TNFSMENU_MAX_RESOURCE 168
#define TNFSMENU_MAX_LINE_LEN MAX_PATHLEN
#define TNFSMENU_MAX_LINES 4096

class fujiMenu
{
private:

    FILE * _menu_file = nullptr;
    uint16_t _current_offset = 0;
    uint16_t _current_pos = 0;
    fsdir_entry _direntry;
    bool _debug = false;;

    uint8_t _type = RESOURCE_TYPE_TEXT;
    uint8_t _displayname_len = 0;
    char _displayname[TNFSMENU_MAX_DISPLAY_NAME];
    uint8_t _resource_len = 0;
    char _resource[TNFSMENU_MAX_RESOURCE];
    int16_t decode_menutype(const char * buf, int numDigits);

public:

    fujiMenu() {};
    ~fujiMenu() {};

    uint16_t get_menu_entry_type() { return _type; };
    uint8_t get_displayname_len() { return _displayname_len; };
    uint8_t get_resource_len() { return _resource_len; };
    uint8_t get_displayname(char * p);
    uint8_t get_resource(char * p);
    bool init(const char *path, FILE * mf);
    void release();
    bool get_initialized() { return (_menu_file != nullptr); };
    uint16_t get_pos() { return _current_offset; };
    bool set_pos(uint16_t newPos);
    fsdir_entry_t *  next_menu_entry();

};

#endif // _FUJI_MENU_
