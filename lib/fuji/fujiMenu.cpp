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
    Debug_printf("fujiMenu::init, IN\r\n");
    release();
    _menu_file = mf;
    return true;
}

void fujiMenu::release()
{
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
            Debug_printf("fujiMenu::init, truncating menu because it is over MAX_MENU_SIZE\r\n");
            break;
        }
        else if ((_current_pos + 1) > MAX_MENU_LINES) {
            Debug_printf("fujiMenu::init, truncating menu because it is over MAX_MENU_LINES\r\n");
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
    char tempBuf[MAX_MENU_LINE_LEN];
    memset(tempBuf, 0, MAX_MENU_LINE_LEN);

    uint16_t menuType = MENU_TYPE_TEXT;
    
    uint16_t itemLen = 0;
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
        else return nullptr;

        itemLen = len;

        if (tempBuf[0] == '-' && tempBuf[1] != '-') {
            char * pt = strchr(tempBuf, ' ');
            if (pt && (pt - tempBuf) < 5) {
                menuType = decode_menutype(tempBuf+1);
                itemStart = (pt - tempBuf + 1);
                itemLen = len - itemStart;
            }
        }

        if (itemLen >= MAX_MENU_ITEM_LEN) itemLen = MAX_MENU_ITEM_LEN-1;

        // populate _direntry;
        memset(_direntry.filename, 0, MAX_MENU_LINE_LEN);
        strncpy(_direntry.filename, &tempBuf[itemStart], itemLen);
        _direntry.isDir = (menuType == RESOURCE_TYPE_FOLDER);
        _direntry.size = 0;
        _direntry.modified_time = 0;
        _direntry.resourceType = menuType;
        return &_direntry;

    }
    else return nullptr;
}

