#include "fujiMenu.h"

#include <cstring>
#include <stdlib.h>
#include <stdio.h>

#include "../../include/debug.h"

#include "fnFsSD.h"
#include "fnFsTNFS.h"

#include "utils.h"

bool fujiMenu::init(const char *path, FILE * mf)
{
    Debug_printf("fujiMenu::init\n");
    release();
    _menu_file = mf;
    return true;
}

void fujiMenu::release()
{
    Debug_printf("fujiMenu::release\n");

    _current_offset = 0;
    _current_pos = 0;

    if (_menu_file != nullptr) 
    {
        fclose(_menu_file);
        _menu_file = nullptr;
    }
}

bool fujiMenu::set_pos(uint16_t newPos) 
{
    Debug_printf("fujiMenu::set_pos\n");

    char tempBuf[MAX_MENU_LINE_LEN];

    // find the offset of the new menu position.
    _current_pos = 0;
    _current_offset = 0;
    int linelen = 0;

    while (_current_pos < newPos && fgets(tempBuf, MAX_MENU_LINE_LEN, _menu_file)) 
    {

        linelen = strlen(tempBuf);

        if (linelen <= 0)
        {
            break;
        }
        else if ((_current_offset + linelen) > MAX_MENU_SIZE) {
            Debug_printf("fujiMenu::init, truncating menu because it is over MAX_MENU_SIZE");
            break;
        }
        else if ((_current_pos + 1) > MAX_MENU_LINES) {
            Debug_printf("fujiMenu::init, truncating menu because it is over MAX_MENU_LINES");
            break;
        }

        _current_pos += 1;
        _current_offset += linelen;
    }

    return true;
}

uint16_t fujiMenu::decode_menutype(const char * buf)
{
    return (uint16_t)strtol(buf, nullptr, 16);
}

fsdir_entry_t * fujiMenu::next_menu_entry()
{
    Debug_printf("fujiMenu::next_menu_entry\r\n");

    char tempBuf[MAX_MENU_LINE_LEN];
    memset(_direntry.filename, 0, MAX_MENU_LINE_LEN);

    uint8_t type = RESOURCE_TYPE_TEXT;
    int16_t len = 0;
    uint8_t itemStart = 0;

    // if we have an offset, skip to it. 
    if (_current_offset > 0) 
    {
        if (fseek(_menu_file, _current_offset, 0) != 0)
        {
            Debug_printf("fujiMenu::get_next_menu_entry, cannot seek to current offset.");
            return nullptr;
         }
    }

    if (!fgets(tempBuf, MAX_MENU_LINE_LEN, _menu_file)) 
    {
        Debug_printf("fujiMenu::next_menu_entry, can't read next line from menu file\r\n");
        return nullptr;
    }

    Debug_printf("fujiMenu::next_menu_entry next menu line: |%s|\r\n", tempBuf);

    _current_pos += 1;
    _current_offset += strlen(tempBuf);

    // menu format: [-<type> ]<item>

    len = strlen(tempBuf);

    // trim trailing newline
    if (len>0 && tempBuf[len-1] == '\n') {
        tempBuf[len-1] = 0;
        len--;
    }
    else return nullptr;

    if (tempBuf[0] == '-' && tempBuf[1] != '-') {
        char * pt = strchr(tempBuf, ' ');
        if (pt && (pt - tempBuf) < 5) {
            type = decode_menutype(tempBuf+1);
            itemStart = (pt - tempBuf + 1);
            len = len - itemStart;
        }
    }

    if (len >= MAX_MENU_ITEM_LEN) len = MAX_MENU_ITEM_LEN-1;
  
    Debug_printf("fujiMenu::next_menu_entry found file type %i for: |%s|\r\n", type, tempBuf+itemStart);

    //_direntry.filename[0] = type >> 8;
    //_direntry.filename[1] = type;
    //strncpy(_direntry.filename+2, tempBuf+itemStart, len);
    strncpy(_direntry.filename, tempBuf+itemStart, len);
    _direntry.isDir = (type == RESOURCE_TYPE_FOLDER);
    _direntry.size = 0;
    _direntry.modified_time = 0;
    _direntry.resource_type = type;
    return &_direntry;
}
