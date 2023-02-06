#pragma once
#include "xorstr.hpp"
#include "auth.hpp"

using namespace KeyAuth;

std::string name = XorStr("Spinayy"); // application name. right above the blurred text aka the secret on the licenses tab among other tabs
std::string ownerid = XorStr("boom"); // ownerid, found in account settings. click your profile picture on top right of dashboard and then account settings.
std::string secret = XorStr("shakalaka"); // app secret, the blurred text on licenses tab and other tabs
std::string version = XorStr("1.0"); // leave alone unless you've changed version on website
std::string url = XorStr("https://keyauth.win/api/1.2/"); // change if you're self-hosting

api KeyAuthApp(name, ownerid, secret, version, url);
