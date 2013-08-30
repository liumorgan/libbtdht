#ifndef __DHT_Message__
#define __DHT_Message__

#include "Buffer.h"
#include <utility> // for std::pair

enum DHTMessageTypes
{
	DHT_UNDEFINED_MESSAGE = 0,
	DHT_QUERY,     // 'y' = q
	DHT_RESPONSE,  // 'y' = r
	DHT_ERROR      // 'y' = e
};

enum DHTCommands
{
	DHT_QUERY_UNDEFINED = 0,
	DHT_QUERY_PING,
	DHT_QUERY_FIND_NODE,
	DHT_QUERY_GET_PEERS,
	DHT_QUERY_ANNOUNCE_PEER,
	DHT_QUERY_VOTE,
	DHT_QUERY_GET,
	DHT_QUERY_PUT
};

class BencodedDict;

/**
 DHTMessage class

 This class is currently concerned with extracting the information for DHT
 queries.  In the future it may be expanded to accomodate responses and errors.

 NOTE:  The BencodedDictionary object from which the message data is extracted
 must be maintained for as long as you want to use the data in the public
 pointer memebers of the instance of this object.  If you use the char* versions
 of the constructor and DecodeMessageData members, the internal object will be
 used, otherwise, your object will be used.

 Only those public data members that represent the arguments of the command
 of the message will have valid values or references.  ALL OTHER MEMBERS
 WILL REMAIN UNINITIALIZED.

 When DecodeMessageData(char *) is used, the object will attempt to obtain the
 region for a 'v' element using the BencEntity::ParseInPlace function that
 supports returning a region.  If it is found, it wil be mapped to Buffer
 vBuf for consumption.  If len in the vBuf member is 0, then there is no v data.
 NOTE:  The parse function returns the region for the first v element it finds.
 DHT RPC's have two defined v elements:  1) v in the arguments dictionary for
 the data element of a 'put' command, and 2) v for the version in the outer
 dictionary.  A correctly formatted benc string should always have the put v
 positioned before the version v.  Only if the command is 'put' will the public
 vBuf element be set.

 NOTE:
 Currently, for backwards compatibility, the BencEntity v pointer is still
 supported. This will eventually be removed in favor of the vBuf described
 above.

 There is very little targeted decoding of a reply ('y' = r) message that can
 be done by the object since reply messages don't contain an equivalent to a
 command that indicates what arguments should be expected in the message.  It
 is up to the dht to use the transaction ID to associate a response with
 a query it emitted and then determine what parts it should extract.
*/
class DHTMessage
{
private:
	// when using with the bencoded string constructor, must make our own dictionary
	BencodedDict* _bDict;
	BencodedDict* _bDictForUser;
	bool _argumentsAreValid;
	bool _parseSuccessful;
	std::pair<unsigned char*, unsigned char*> region;

	void CopyFrom(DHTMessage &src);
	void DecodeQuery(BencodedDict &bDict);
	void Init();
	/* future
	void DecodeError(BencodedDict &bDict);
	*/

public:
	DHTMessage();
	DHTMessage(DHTMessage &src);
	DHTMessage(BencodedDict &Dictionary);
	DHTMessage(char * bencMessageString);
	DHTMessage(byte* bencMessageBytes, int numBytes);
	~DHTMessage();
	DHTMessage& operator=(DHTMessage &rhs);

	void DecodeMessageData(char *bencMessageString);
	void DecodeMessageData(byte* bencMessageBytes, int numBytes);
	void DecodeMessageData(BencodedDict &bDict);
	bool ValidArguments(){return _argumentsAreValid;}
	bool ParseSuccessful(){return _parseSuccessful;}

	/** Only use this function AFTER DecodeMessageData has been invoked */
	BencodedDict& GetBencodedDictionary(){return *_bDictForUser;}

	DHTMessageTypes dhtMessageType;
	DHTCommands dhtCommand;

	// These public parts are "as is" and may or may not be valid
	// depending on the DHT message received
	cstr type;
	cstr command; // this command is mirrored in the command enum
	byte *id;
	int portNum;
	int vote;
	int seed;
	int noseed;
	int scrape;
	int sequenceNum;  // 'seq' for mutable put
	int impliedPort;
	Buffer filename;
	Buffer infoHash;
	Buffer token;
	BencodedDict *args;
	Buffer transactionID; // tid
	Buffer version; // ver
	Buffer signature;  // 'sig' for mutable put
	Buffer key; // 'k' for mutable put
	Buffer target;

	// this is used to point to the 'v' region of a "put" that is extracted when
	// a bencstring is parsed.  It is assigned the region's values when it is
	// determined that a "put" request was made.  Otherwise it is unassigned.
	Buffer vBuf;

	// reply specific components
	BencodedDict* replyDict;
};

inline DHTMessage::DHTMessage()
{
	Init();
}

inline DHTMessage::DHTMessage(BencodedDict &Dictionary)
{
	Init();
	DecodeMessageData(Dictionary);
}

inline DHTMessage& DHTMessage::operator=(DHTMessage &rhs)
{
	CopyFrom(rhs);
	return *this;
}

inline DHTMessage::DHTMessage(DHTMessage &src)
{
	CopyFrom(src);
}


#endif // __DHT_Message__