/*******************************************************************************
 *
 * Privacy
 * all abstract class for encryption
 *
 * Copyright 2020 Evan . All rights reserved.
 *
 ******************************************************************************/
#ifndef PRIVACY_2020_03_16_H
#define PRIVACY_2020_03_16_H
#include <vector>
#include <optional>
#include <memory>

struct Privacy
{
    virtual int Compress(const std::vector<unsigned char> &in,
                         const std::vector<unsigned char> &key,
                         std::vector<unsigned char> &out) = 0;

    virtual int UnCompress(const std::vector<unsigned char> &in,
                           const std::vector<unsigned char> &key,
                           std::vector<unsigned char> &out) = 0;

    virtual void SetIV(const std::vector<unsigned char> &iv) = 0;

    virtual std::vector<unsigned char> ToBytes() = 0;

    virtual int GetMask() const = 0;

    virtual ~Privacy() = default;
};

std::optional<std::shared_ptr<Privacy>> build_privacy_method(const std::vector<unsigned char> &bytes_stream);
std::optional<std::vector<unsigned char> > make_hmac(const std::vector<unsigned char> &key, const std::vector<unsigned char> &msg );
std::optional<std::vector<unsigned char> > make_sha256(const std::vector<unsigned char> &src);
std::optional<std::vector<unsigned char> > make_sha1(const std::vector<unsigned char> &src);



#endif