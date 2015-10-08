//
//  AnsyncStreamSession.hpp
//  Pods
//
//  Created by jinchu darwin on 15/9/29.
//
//

#ifndef AnsyncStreamSession_hpp
#define AnsyncStreamSession_hpp

#include <cstddef>
#include <functional>
#include <vector>
#include <queue>
#include <string>

#include <videocore/system/util.h>
#include <videocore/system/JobQueue.hpp>
#include <videocore/stream/IStreamSession.hpp>

namespace videocore {
    
    typedef enum {
        kAsyncStreamStateNone           = 0,
        kAsyncStreamStateConnecting     = 1,
        kAsyncStreamStateConnected      = 2,
        kAsyncStreamStateDisconnecting  = 4,
        kAsyncStreamStateDisconnected   = 5,
        kAsyncStreamStateError          = 6,
    } AnsyncStreamState_T;

#pragma mark -
#pragma mark PreallocBuffer
    class PreallocBuffer {
    public:
        PreallocBuffer(size_t capBytes);
        ~PreallocBuffer();
        
        void ensureCapacityForWrite(size_t capBytes);
        
        size_t availableBytes();    // for read
        uint8_t *readBuffer();
        
        void getReadBuffer(uint8_t **bufferPtr, size_t *availableBytesPtr);
        
        void didRead(size_t bytesRead);
        
        size_t availableSpace();   // for write
        uint8_t *writeBuffer();
        
        void getWriteBuffer(uint8_t **bufferPtr, size_t *availableSpacePtr);
        
        void didWrite(size_t bytesWritten);
        
        void reset();
        
    private:
        uint8_t *m_preBuffer;
        size_t m_preBufferSize;
        
        uint8_t *m_readPointer;
        uint8_t *m_writePointer;
    };
    
    typedef std::vector<uint8_t> AsyncStreamBuffer;
    typedef std::shared_ptr<AsyncStreamBuffer> AsyncStreamBufferSP;
    
#pragma mark -
#pragma mark AnsyncStreamSession
    
    typedef std::function<void(StreamStatus_T status)> SSConnectionStatus_T;
    typedef std::function<void(AsyncStreamBuffer& abuff)> SSAnsyncReadCallBack_T;
    typedef std::function<void()> SSAnsyncWriteCallBack_T;

    
    class AnsyncStreamReader;
    class AnsyncStreamWriter;
    
    class AnsyncStreamSession {
    public:
        AnsyncStreamSession(IStreamSession *stream);
        ~AnsyncStreamSession();
        void connect(const std::string &host, int port, SSConnectionStatus_T statuscb);
        void disconnect();
        void write(uint8_t *buffer, size_t length, SSAnsyncWriteCallBack_T writecb=nullptr);
        void readLength(size_t length, SSAnsyncReadCallBack_T readcb);
        void readMoreLength(size_t length, AsyncStreamBufferSP orgbuf, size_t offset, SSAnsyncReadCallBack_T readcb);
        
    private:
        void setState(AnsyncStreamState_T state);
        std::shared_ptr<AnsyncStreamReader> getCurrentReader();
        void doReadData();
        bool innnerReadData();
        void finishCurrentReader();
        
        std::shared_ptr<AnsyncStreamWriter> getCurrentWriter();

        void doWriteData();
        void innerWriteData();
        void finishCurrentWriter();
        
    private:
        std::unique_ptr<IStreamSession> m_stream;
        JobQueue m_eventTriggerJob;
        JobQueue m_socketJob;
        std::queue<std::shared_ptr<AnsyncStreamReader>> m_readerQueue;
        std::queue<std::shared_ptr<AnsyncStreamWriter>> m_writerQueue;
        SSConnectionStatus_T m_connectionStatusCB;
        AnsyncStreamState_T m_state;
        PreallocBuffer m_inputBuffer;
        bool m_doReadingData;
    };
}
#endif /* AnsyncStreamSession_hpp */

