#include "privacy/privacy.h"
#include "privacy/raw.h"
#include "privacy/gcm.h"
#include "privacy/chacha20.h"
#include <sstream>
#include <optional>
#include <iomanip>
#include "core/func.hpp"
#include "privacy/xcha20_poly1305.h"
#include "sodium.h"
#include "core/g.h"

std::optional<std::shared_ptr<Privacy>> PrivacyBase::build_privacy_method(const std::vector<unsigned char> &bytes_stream)
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
		sp = std::shared_ptr<Privacy>(new XChaCha20Poly1305());
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

std::optional<std::vector<unsigned char>> PrivacyBase::make_hmac(const std::vector<unsigned char> &key, const std::vector<unsigned char> &msg)
{
	std::vector<unsigned char> out(32, 0);

	int r = crypto_auth_hmacsha256(out.data(),
								   msg.data(),
								   msg.size(),
								   key.data());
	if (r != 0)
	{
		LOG(ERROR) << "crypto_auth_hmacsha256 failed!" << r;
		return std::nullopt;
	}
	return out;
}

static unsigned char sz_sha1_base[] = "E234V678A012N456I890O234V678U012";
std::optional<std::vector<unsigned char>> PrivacyBase::make_sha1(const std::vector<unsigned char> &src)
{
	static std::vector<unsigned char> key(std::begin(sz_sha1_base), std::begin(sz_sha1_base)+32);
	return make_hmac(key,src);	
}


