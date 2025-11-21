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

    char tempBuf[TNFSMENU_MAX_LINE_LEN];

    // find the offset of the new menu position.
    _current_pos = 0;
    _current_offset = 0;
    int linelen = 0;

    while (_current_pos < newPos && fgets(tempBuf, TNFSMENU_MAX_LINE_LEN, _menu_file)) 
    {

        linelen = strlen(tempBuf);

        if (linelen <= 0)
        {
            break;
        }
        else if ((_current_offset + linelen) > TNFSMENU_MAX_SIZE) {
            Debug_printf("fujiMenu::init, truncating menu because it is over TNFSMENU_MAX_SIZE\r\n");
            break;
        }
        else if ((_current_pos + 1) > TNFSMENU_MAX_LINES) {
            Debug_printf("fujiMenu::init, truncating menu because it is over TNFSMENU_MAX_LINES\r\n");
            break;
        }

        _current_pos += 1;
        _current_offset += linelen;
    }

    return true;
}

int16_t fujiMenu::decode_menutype(const char * buf, int numDigits)
{
    Debug_printf("fujiMenu decode menutype in\r\n");
    int16_t retval = 0;
    int mult = 1;
    for (int i = (numDigits-1); i >= 0; i--) {
        if (buf[i] < 48 || buf[i] > 57) return -1;
        retval += ((buf[i]-48)*mult);
        mult = (mult*10);
    }
    Debug_printf("fujiMenu decode menutype out %i\r\n", retval);
    return retval;
}

/*
 * Get the next menu item. Menu items have the format:
 *
 *     <type>|<resource>|<display name>
 *
 *     OR
 *
 *     <item resource>
 *
 * Where:
 *
 *     <type> is the numeric type ID of the menu item.
 *     <resource> this is the name of the item as shoudl be displyed on screen
 *     <displayname> screen friendly display name for the resource.
 */

fsdir_entry_t * fujiMenu::next_menu_entry() 
{
    char tempBuf[TNFSMENU_MAX_LINE_LEN];
    char myBuf[TNFSMENU_MAX_LINE_LEN];
    _type = RESOURCE_TYPE_TEXT;
    _displayname_len = 0;
    _resource_len = 0;
    memset(_displayname, 0, TNFSMENU_MAX_DISPLAY_NAME);
    memset(_resource, 0, TNFSMENU_MAX_RESOURCE);
    uint8_t displayNameStart = 0;
    uint8_t resourceStart = 0;

    // if we have an offset, skip to it. 
    if (_current_offset > 0) 
    {
        if (fseek(_menu_file, _current_offset, 0) != 0)
        {
            Debug_printf("fujiMenu::next_menu_entry, cannot seek to current offset.\r\n");
            return nullptr;
        }
    }

    if (!fgets(tempBuf, TNFSMENU_MAX_LINE_LEN, _menu_file)) 
    {
        Debug_printf("fujiMenu::next_menu_entry, can't read next line from menu file\r\n");
        return nullptr;
    }

    _current_pos += 1;
    _current_offset += strlen(tempBuf);

    // menu format: [<type>|]<name>[|<item>]

    int len = strlen(tempBuf);

    if (len>0 && tempBuf[len-1] == '\n') {
        tempBuf[len-1] = 0;
        len--;
    }
    else return nullptr;

    int newBufPos = 0;
    int offsets[4];
    char lastChar = ' ';        
    int count = 0;
    _displayname_len = len;
    _resource_len = len;

    for (int i=0; i<len; i++) 
    {
        if (tempBuf[i] == '|' && lastChar != '\\') {
            myBuf[newBufPos] = 0;
            newBufPos++;
            offsets[count] = newBufPos;
            if (count == 0) {
                displayNameStart = newBufPos;
                _displayname_len = (len - newBufPos);
                resourceStart = displayNameStart;
                _resource_len = _displayname_len;
            }
            else if (count == 1) {
                _displayname_len = (i - displayNameStart);
                resourceStart = newBufPos;
                _resource_len = (len - resourceStart);
            }
            count++;
            lastChar = tempBuf[i];
        }
        else if (tempBuf[i] == '\\') {
            lastChar = '\\';
        }
        else {
            myBuf[newBufPos] = tempBuf[i];
            newBufPos++;
            lastChar = tempBuf[i];
        }
    }

    if (count > 0) {
        _type = decode_menutype(tempBuf, displayNameStart-1);
    }

    if (_debug) {
        for (int i = 0; i < count; i++) {
            Debug_printf("fujiMenu::read_menu_entry offset %i: %s\r\n", i,  &myBuf[offsets[i]]);
        }
    }

    if (_displayname_len >= TNFSMENU_MAX_DISPLAY_NAME) _displayname_len = TNFSMENU_MAX_DISPLAY_NAME-1;
    if (_resource_len >= TNFSMENU_MAX_RESOURCE) _resource_len = TNFSMENU_MAX_RESOURCE-1;

    strncpy(_displayname, &myBuf[displayNameStart], _displayname_len);
    strncpy(_resource, &myBuf[resourceStart], _resource_len);

    if (_debug) {
        Debug_printf("fujiMenu::read_menu_entry _displayname: %s\r\n", _displayname);
        Debug_printf("fujiMenu::read_menu_entry _resource: %s\r\n", _resource);
    }

    // populate _direntry;
    strncpy(_direntry.filename, _resource, MAX_PATHLEN);
    _direntry.isDir = (_type == RESOURCE_TYPE_FOLDER);
    _direntry.size = 0;
    _direntry.modified_time = 0;
    _direntry.resourceType = _type;
    return &_direntry;
}

uint8_t fujiMenu::get_displayname(char * p)
{
    strcpy(p, _displayname);
    return _displayname_len;
}

uint8_t fujiMenu::get_resource(char *p)
{
    strcpy(p, _resource);
    return _resource_len;
}
