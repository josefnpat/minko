/*
Copyright (c) 2014 Aerys

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "minko/file/APKProtocol.hpp"

#include "minko/file/Options.hpp"
#include "minko/Signal.hpp"

#include "SDL_rwops.h"

using namespace minko;
using namespace minko::file;

APKProtocol::APKProtocol()
{
}

void
APKProtocol::load()
{
    auto resolvedFilename = this->resolvedFilename();

    auto options = _options;

    auto protocolPrefixPosition = resolvedFilename.find("://");

    if (protocolPrefixPosition != std::string::npos)
    {
        resolvedFilename = resolvedFilename.substr(protocolPrefixPosition + 3);
    }

    _options = options;

    SDL_RWops* file = SDL_RWFromFile(resolvedFilename.c_str(), "rb");

    auto loader = shared_from_this();

    if (file)
    {
        auto offset = options->seekingOffset();
        auto size = options->seekedLength() > 0 ? options->seekedLength() : file->size(file);

        _progress->execute(shared_from_this(), 0.0);

        data().resize(size);

        file->seek(file, offset, RW_SEEK_SET);
        file->read(file, (char*) &data()[0], size, 1);
        file->close(file);

        _progress->execute(loader, 1.0);

        _complete->execute(shared_from_this());
    }
    else
    {
        _error->execute(shared_from_this());
    }
}

bool
APKProtocol::fileExists(const std::string& filename)
{
    auto resolvedFilename = filename;

    auto protocolPrefixPosition = resolvedFilename.find("://");

    if (protocolPrefixPosition != std::string::npos)
    {
        resolvedFilename = filename.substr(protocolPrefixPosition + 3);
    }

    auto file = SDL_RWFromFile(resolvedFilename.c_str(), "rb");

    return file != nullptr;
}
