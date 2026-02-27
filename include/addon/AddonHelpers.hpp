#ifndef ADDONHELPERS_HPP
#define ADDONHELPERS_HPP

#include <napi.h>
#include <string>
#include <vector>

#include "../utils/WavParser.hpp"

std::string toHexPreview(const std::vector<char> &data, std::size_t maxBytes = 128);
Napi::Object makeMetadataObject(Napi::Env env, const Parser &parser);
Napi::Array makeOtherChunks(Napi::Env env, const Parser &parser);

#endif // ADDONHELPERS_HPP
