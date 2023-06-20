// Copyright Youre Games, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class YOURE_API PKCEHelper
{
public:
	PKCEHelper();
	~PKCEHelper();

	std::string getSHA256HashFromHex(const std::string& text);
	std::string randomString(size_t length);
	std::string convertStringToHex(const std::string& input);
	std::string base64_encode(const std::string& in);
	std::string base64_encode2(const std::string& in);


private:

	/// split into 64 byte blocks (=> 512 bits), hash is 32 bytes long
	enum { BlockSize = 512 / 8, HashBytes = 32 };


	/// add arbitrary number of bytes
	void add(const void* data, size_t numBytes);

	/// return latest hash as 64 hex characters
	std::string getHash();
	/// return latest hash as bytes
	void        getHash(unsigned char buffer[HashBytes]);


	/// restart
	void reset();


	/// process 64 bytes
	void processBlock(const void* data);
	/// process everything left in the internal buffer
	void processBuffer();

	/// size of processed data in bytes
	uint64_t m_numBytes;
	/// valid bytes in m_buffer
	size_t   m_bufferSize;
	/// bytes not processed yet
	uint8_t  m_buffer[BlockSize];

	enum { HashValues = HashBytes / 4 };
	/// hash, stored as integers
	uint32_t m_hash[HashValues];
};
