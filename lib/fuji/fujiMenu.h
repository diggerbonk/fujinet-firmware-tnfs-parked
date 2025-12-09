#ifndef _FUJI_MENU_
#define _FUJI_MENU_

#include <stdio.h>
#include "fnFS.h"

#define MAX_MENU_SIZE 65535
#define MAX_MENU_ITEM_LEN MAX_PATHLEN
#define MAX_MENU_LINE_LEN MAX_PATHLEN
#define MAX_MENU_LINES 2048

class fujiMenu
{
private:

    FILE * _menu_file = nullptr;
    uint16_t _current_offset = 0;
    uint16_t _current_pos = 0;
    fsdir_entry _direntry;

    uint16_t decode_menutype(const char * buf);

public:

    fujiMenu() {};
    ~fujiMenu() {};

    bool init(const char *path, FILE * mf);
    void release();
    bool get_initialized() { return (_menu_file != nullptr); };
    uint16_t get_pos() { return _current_offset; };
    bool set_pos(uint16_t newPos);
    fsdir_entry_t * next_menu_entry();

};

#endif // _FUJI_MENU_
