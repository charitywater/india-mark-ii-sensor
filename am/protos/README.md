# NanoPB

[Online Documentation](https://jpa.kapsi.fi/nanopb/docs/concepts.html)


## Example Encoding Message

```c
#include <stdio.h>
#include <pb_encode.h>
#include "messages.pb.h"

// Example of how to encode a status message
int encodeStatusMessage()
{
    /* This is the buffer where we will store our message. */
    uint8_t buffer[128];
    size_t message_length;
    bool status;

    /* Allocate space on the stack to store the message data.
        *
        * Nanopb generates simple struct definitions for all the messages.
        * - check out the contents of simple.pb.h!
        * It is a good idea to always initialize your structures
        * so that you do not have garbage data from RAM in there.
        */
    StatusMessage message = StatusMessage_init_zero;

    /* Create a stream that will write to our buffer. */
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    // Fill in all fields here...
    message.header.productId = 4;



    /* Now we are ready to encode the message! */
    status = pb_encode(&stream, StatusMessage_fields, &message);
    message_length = stream.bytes_written;

    /* Then just check for any errors.. */
    if (!status)
    {
        printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
        return 1;
    }

    // Send stream to desired location here...
```

## Example Decoding Message

```c
#include <stdio.h>
#include <pb_decode.h>
#include "messages.pb.h"

// Example of how to decode status message
// Assuming you would have a buffer/length to decode.
int decodeStatusMessage(uint8_t * buffer, size_t message_length)
{

    /* Allocate space for the decoded message. */
    StatusMessage message = StatusMessage_init_zero;

    /* Create a stream that reads from the buffer. */
    pb_istream_t stream = pb_istream_from_buffer(buffer, message_length);

    /* Now we are ready to decode the message. */
    status = pb_decode(&stream, StatusMessage_fields, &message);

    /* Check for errors... */
    if (!status)
    {
        printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
        return 1;
    }

    /* Print the data contained in the message. */
    printf("The product ID is %d!\n", (int)message.header.productId);

}
```