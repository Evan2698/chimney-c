#include "privacy/privacy.h"
#include "privacy/raw.h"
#include "privacy/gcm.h"
#include "privacy/chacha20.h"
#include <glog/logging.h>
#include <sstream>
#include <optional>
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include "common/func.hpp"

std::optional<std::shared_ptr<Privacy>> build_privacy_method(const std::vector<unsigned char> &bytes_stream)
{
	if (bytes_stream.size() < 15)
	{
		LOG(ERROR) << "can not create privacy method: " << bytes_stream.size();
		return std::nullopt;
	}

	std::shared_ptr<Privacy> sp;
	auto mask = ((unsigned)bytes_stream[0]) << 8 | bytes_stream[1];
	switch (mask)
	{
	case 0x1234:
		sp = std::shared_ptr<Privacy>(new GCM());
		LOG(INFO) << "GCM method:" << bytes_stream.size();
		break;

	case 0x1235:
		sp = std::shared_ptr<Privacy>(new ChaCha20());
		LOG(INFO) << "ChaCha20 method: " << bytes_stream.size();
		break;

	case 0x1236:
		/* TODO */
		LOG(INFO) << "ChaCha20-POLY1305 method: " << bytes_stream.size();
		break;

	case 0x1237:
		sp = std::shared_ptr<Privacy>(new RawEncrypt());
		LOG(INFO) << "RAW method: " << bytes_stream.size();
		break;
	default:
		break;
	}

	if (!!sp)
	{
		std::vector<unsigned char>::size_type len = bytes_stream[2];
		if (3 + len != bytes_stream.size())
		{
			LOG(ERROR) << "verify size failed!" << bytes_stream.size();
			return std::nullopt;
		}
		sp->SetIV(std::vector<unsigned char>(bytes_stream.begin() + 3, bytes_stream.end()));
	}
	else
	{
		return std::nullopt;
	}

	return sp;
}

std::optional<std::vector<unsigned char>> make_hmac(const std::vector<unsigned char> &key, const std::vector<unsigned char> &msg)
{
	unsigned char hash[128] = {0};
	unsigned int len = 128;

	auto ctx = HMAC_CTX_new();
	if (ctx == nullptr)
	{
		return std::nullopt;
	}

	if (!HMAC_Init_ex(ctx, key.data(), key.size(), EVP_sha256(), NULL))
	{
		HMAC_CTX_free(ctx);
		return std::nullopt;
	}

	if (!HMAC_Update(ctx, msg.data(), msg.size()))
	{
		HMAC_CTX_free(ctx);
		return std::nullopt;
	}

	if (!HMAC_Final(ctx, hash, &len))
	{
		HMAC_CTX_free(ctx);
		return std::nullopt;
	}

	return std::vector<unsigned char>(std::begin(hash), std::begin(hash) + len);
}

std::optional<std::vector<unsigned char>> make_sha1(const std::vector<unsigned char> &src)
{
	unsigned char szBuffer[SHA_DIGEST_LENGTH] = {0};
	SHA_CTX context;
	if (!SHA1_Init(&context))
		return std::nullopt;

	if (!SHA1_Update(&context, src.data(), src.size()))
		return std::nullopt;

	if (!SHA1_Final(szBuffer, &context))
		return std::nullopt;

	auto k = ToHex(std::begin(szBuffer), std::end(szBuffer));
	if (k.size() > 32)
	{
		k.resize(32);
	}

	return std::vector<unsigned char>(k.begin(), k.end());
}

std::optional<std::vector<unsigned char>> make_sha256(const std::vector<unsigned char> &src)
{
	unsigned char szBuffer[SHA256_DIGEST_LENGTH] = {0};
	SHA256_CTX context;
	if (!SHA256_Init(&context))
		return std::nullopt;

	if (!SHA256_Update(&context, src.data(), src.size()))
		return std::nullopt;

	if (!SHA256_Final(szBuffer, &context))
		return std::nullopt;

	auto k = ToHex(std::begin(szBuffer), std::end(szBuffer));

	return std::vector<unsigned char>(k.begin(), k.end());
}
