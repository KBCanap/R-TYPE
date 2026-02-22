/*
** Lua compatibility fix for Fedora/RHEL
** Their luaconf-x86_64.h defines LUA_COMPAT_5_2 without a value,
** which breaks sol2's preprocessor comparison.
*/

#pragma once

// Include Lua headers first to get their definitions
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

// Fix empty LUA_COMPAT_* defines by redefining with proper values
#ifdef LUA_COMPAT_5_1
#undef LUA_COMPAT_5_1
#define LUA_COMPAT_5_1 1
#endif

#ifdef LUA_COMPAT_5_2
#undef LUA_COMPAT_5_2
#define LUA_COMPAT_5_2 1
#endif

#ifdef LUA_COMPAT_BITLIB
#undef LUA_COMPAT_BITLIB
#define LUA_COMPAT_BITLIB 1
#endif
