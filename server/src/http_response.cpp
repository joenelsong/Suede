#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <http_response.hpp>
#include <http_request.hpp>
#include <stdlib.h>
#include <cstring>
#include <string>
using std::string;

//Sort of based on solution from stack overflow, may need rewrite as it seems to have memory leak in valgrind
void HTTP_Response::base64(unsigned char const* input, int length, char** buffer)
{
    if ( (input != NULL) && (input[0] == (char)0) ) {
		*buffer = (char*)"";
    }
	else {
		BIO *bmem, *b64;
    	BUF_MEM *bptr;
   		b64 = BIO_new(BIO_f_base64());//This seems to be the source of some memory leaks that need to be resolved
		bmem = BIO_new(BIO_s_mem());
   		b64 = BIO_push(b64, bmem);
    	BIO_write(b64, input, length);
   	 	BIO_flush(b64);
    	BIO_get_mem_ptr(b64, &bptr);
    	*buffer = new char[bptr->length];
   		memcpy(*buffer, bptr->data, bptr->length);
    	(*buffer)[bptr->length - 1] = 0;
    	BIO_free_all(b64); // This should stop the memory leak
		
    }
}

string HTTP_Response::generateWebSocketAcceptVal(const string& clientKey)
{
	unsigned char hashResult[20];
	char *outBuffer = NULL;
	string localKeyCopy(clientKey.c_str());
	string guid("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
	string combined = clientKey + guid;
	SHA1((const unsigned char*) combined.c_str(), combined.size(), hashResult);
	base64(hashResult, 20, &outBuffer);
	string finalString(outBuffer);
	delete[] outBuffer;
	return finalString;
}

/**
 * This is a factory method that constructs HTTP_Response a 
 * provided HTTP_Request request.
 * @param request pointer to HTTP_Request for which the response is created for
 * @return response A HTTP_Response constructed from the binary buffer contents
 */
HTTP_Response* HTTP_Response::buildResponseToRequest(const HTTP_Request *request)
{
    HTTP_Response *response = new HTTP_Response();
    string clientKey = request->getWebSocketKey();
    string generatedKey = generateWebSocketAcceptVal(clientKey);
	//TODO: don't use setResponseString, build a real HTTP_Response object that can be used for other things
    //response->setResponseString("HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: "+ generatedKey +"\r\nSec-WebSocket-Protocol: chat\r\n\r\n");
    response->setResponseString("HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: "+ generatedKey +"\r\n\r\n");
    return response;
}

//temporary for emulating toString construction
void HTTP_Response::setResponseString(const string& testIn) //temporary
{
	responseString = testIn;
}

/**
 * toString Method generates text http response.
 * @return responseString String that is a text version of the http request, 
 * fully prepared for transmission
 */
const string& HTTP_Response::toString() const
{
	//TODO: make this a proper string construction
    return responseString;
}
