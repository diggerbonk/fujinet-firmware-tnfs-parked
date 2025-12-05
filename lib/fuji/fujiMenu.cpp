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

bool fujiMenu::next_menu_entry() 
{
    Debug_printf("fujiMenu::next_menu_entry\n");

    char tempBuf[MAX_MENU_LINE_LEN];
    _type = MENU_TYPE_TEXT;
    _item_len = 0;
    memset(_item, 0, MAX_MENU_ITEM_LEN);
    uint8_t itemStart = 0;

    // if we have an offset, skip to it. 
    if (_current_offset > 0) 
    {
        if (fseek(_menu_file, _current_offset, 0) != 0)
        {
            Debug_printf("fujiMenu::get_next_menu_entry, cannot seek to current offset.");
            return false;
         }
    }

    if (fgets(tempBuf, MAX_MENU_LINE_LEN, _menu_file)) 
    {
        _current_pos += 1;
        _current_offset += strlen(tempBuf);

        // menu format: [-<type> ]<item>

        int len = strlen(tempBuf);

        if (len>0 && tempBuf[len-1] == '\n') {
            tempBuf[len-1] = 0;
            len--;
        }
        else return false;

        _item_len = len;

        if (tempBuf[0] == '-' && tempBuf[1] != '-') {
            char * pt = strchr(tempBuf, ' ');
            if (pt && (pt - tempBuf) < 5) {
                _type = decode_menutype(tempBuf+1);
                itemStart = (pt - tempBuf + 1);
                _item_len = len - itemStart;
            }
        }

        if (_item_len >= MAX_MENU_ITEM_LEN) _item_len = MAX_MENU_ITEM_LEN-1;

        strncpy(_item, &tempBuf[itemStart], _item_len);

        if (_type == MENU_TYPE_FOLDER || _type == MENU_TYPE_SUBMENU) 
        {
            _item_len++;
            _item[_item_len-1] = '/';
            _item[_item_len] = 0;
        }

        return true;
    }
    else return false;
}

uint8_t fujiMenu::get_item(char *p)
{
    strcpy(p, _item);
    return _item_len;
}
