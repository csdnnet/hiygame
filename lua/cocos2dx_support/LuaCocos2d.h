/****************************************************************************
 Copyright (c) 2011 cocos2d-x.org
 Copyright (c) 2011 NetDragon.com
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#ifndef __LUACOCOS2D_H
#define __LUACOCOS2D_H

#if defined(_WIN32) && defined(_DEBUG)
#pragma warning (disable:4800)
#endif

#define TOLUA_RELEASE

#if !defined(COCOS2D_DEBUG) || COCOS2D_DEBUG == 0
#define TOLUA_RELEASE
#endif

#include "tolua++.h"

int  tolua_Cocos2d_open(lua_State* tolua_S);


#endif // __LUACOCOS2D_H
