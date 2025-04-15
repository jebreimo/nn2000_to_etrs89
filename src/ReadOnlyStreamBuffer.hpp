//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-01-04.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <istream>

namespace Utilities
{
    class ReadOnlyStreamBuffer : public std::basic_streambuf<char>
    {
    public:
        ReadOnlyStreamBuffer(const char* data, size_t size)
            : data_(data), pos_(0), size_(std::streamsize(size))
        {
        }

    protected:
        std::streamsize xsgetn(char* ptr, std::streamsize count) final
        {
            auto n = std::min(count, size_ - pos_);
            std::copy(data_ + pos_, data_ + pos_ + n, ptr);
            pos_ += n;
            return n;
        }

        pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode) final
        {
            if (dir == std::ios_base::cur)
            {
                if (off > size_ - pos_)
                    pos_ = size_;
                else if (-off > pos_)
                    pos_ = 0;
                else
                    pos_ += off;
            }
            else if (dir == std::ios_base::end)
            {
                if (off > 0)
                    pos_ = size_;
                else if (-off > size_)
                    pos_ = 0;
                else
                    pos_ = size_ + off;
            }
            else if (dir == std::ios_base::beg)
            {
                if (off < 0)
                    pos_ = 0;
                else if (off > size_)
                    pos_ = size_;
                else
                    pos_ = off;
            }
            return pos_;
        }

        pos_type seekpos(pos_type pos, std::ios_base::openmode) final
        {
            pos_ = std::min(std::streamsize(pos), size_);
            return pos_;
        }

        const char* data_;
        std::streamsize pos_;
        std::streamsize size_;
    };
}
