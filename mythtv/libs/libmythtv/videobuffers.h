#ifndef VIDEOBUFFERS_H
#define VIDEOBUFFERS_H

// Qt
#include <QSize>
#include <QMutex>
#include <QString>

// MythTV
#include "mythtvexp.h"
#include "mythframe.h"
#include "mythdeque.h"
#include "mythcodecid.h"

// Std
#include <vector>
#include <map>

using frame_queue_t  = MythDeque<MythVideoFrame*> ;
using frame_vector_t = std::vector<MythVideoFrame>;
using vbuffer_map_t  = std::map<const MythVideoFrame*, uint>;

const QString& DebugString(const MythVideoFrame *Frame, bool Short = false);
const QString& DebugString(uint  FrameNum, bool Short = false);

enum BufferType
{
    kVideoBuffer_avail     = 0x00000001,
    kVideoBuffer_limbo     = 0x00000002,
    kVideoBuffer_used      = 0x00000004,
    kVideoBuffer_pause     = 0x00000008,
    kVideoBuffer_displayed = 0x00000010,
    kVideoBuffer_finished  = 0x00000020,
    kVideoBuffer_decode    = 0x00000040,
    kVideoBuffer_all       = 0x0000003F,
};

class MTV_PUBLIC VideoBuffers
{
  public:
    VideoBuffers() = default;
    virtual ~VideoBuffers();

    static uint GetNumBuffers(int PixelFormat, int MaxReferenceFrames = 16, bool Decoder = false);
    void Init(uint NumDecode, bool ExtraForPause,
              uint NeedFree, uint NeedprebufferNormal,
              uint NeedPrebufferSmall);
    bool CreateBuffers(VideoFrameType Type, QSize Size, bool ExtraForPause,
                       uint NeedFree, uint NeedprebufferNormal,
                       uint NeedPrebufferSmall, int MaxReferenceFrames = 16);
    bool CreateBuffers(VideoFrameType Type, int Width, int Height);
    static bool ReinitBuffer(MythVideoFrame *Frame, VideoFrameType Type, MythCodecID CodecID, int Width, int Height);
    void DeleteBuffers(void);
    void SetDeinterlacing(MythDeintType Single, MythDeintType Double, MythCodecID CodecID);

    void Reset(void);
    void DiscardFrames(bool NextFrameIsKeyFrame);
    void ClearAfterSeek(void);

    void SetPrebuffering(bool Normal);

    MythVideoFrame *GetNextFreeFrame(BufferType EnqueueTo = kVideoBuffer_limbo);
    void ReleaseFrame(MythVideoFrame *Frame);
    void DeLimboFrame(MythVideoFrame *Frame);
    void StartDisplayingFrame(void);
    void DoneDisplayingFrame(MythVideoFrame *Frame);
    void DiscardFrame(MythVideoFrame *Frame);
    void DiscardPauseFrames(void);
    bool DiscardAndRecreate(MythCodecID CodecID, QSize VideoDim, int References);

    MythVideoFrame *At(uint FrameNum);
    MythVideoFrame *Dequeue(BufferType Type);
    MythVideoFrame *Head(BufferType Type);
    MythVideoFrame *Tail(BufferType Type);
    void Requeue(BufferType Dest, BufferType Source, int Count = 1);
    void Enqueue(BufferType Type, MythVideoFrame* Frame);
    void SafeEnqueue(BufferType Type, MythVideoFrame* Frame);
    void Remove(BufferType Type, MythVideoFrame *Frame);
    frame_queue_t::iterator BeginLock(BufferType Type);
    frame_queue_t::iterator End(BufferType Type);
    void EndLock();
    uint Size(BufferType Type) const;
    bool Contains(BufferType Type, MythVideoFrame* Frame) const;

    MythVideoFrame *GetScratchFrame(void);
    MythVideoFrame *GetLastDecodedFrame(void);
    MythVideoFrame *GetLastShownFrame(void);
    void SetLastShownFrameToScratch(void);

    uint ValidVideoFrames(void) const;
    uint FreeVideoFrames(void) const;
    bool EnoughFreeFrames(void) const;
    bool EnoughDecodedFrames(void) const;

    const MythVideoFrame *At(uint FrameNum) const;
    const MythVideoFrame *GetLastDecodedFrame(void) const;
    const MythVideoFrame *GetLastShownFrame(void) const;
    uint  Size(void) const;
    void Clear(uint FrameNum);
    void Clear(void);
    bool CreateBuffer(int Width, int Height, uint Number, void *Data, VideoFrameType Format);

    QString GetStatus(uint Num = 0) const;

  private:
    frame_queue_t       *Queue(BufferType Type);
    const frame_queue_t *Queue(BufferType Type) const;
    MythVideoFrame      *GetNextFreeFrameInternal(BufferType EnqueueTo);
    static void          SetDeinterlacingFlags(MythVideoFrame &Frame, MythDeintType Single,
                                               MythDeintType Double, MythCodecID CodecID);

    frame_queue_t        m_available;
    frame_queue_t        m_used;
    frame_queue_t        m_limbo;
    frame_queue_t        m_pause;
    frame_queue_t        m_displayed;
    frame_queue_t        m_decode;
    frame_queue_t        m_finished;
    vbuffer_map_t        m_vbufferMap;
    frame_vector_t       m_buffers;

    uint                 m_needFreeFrames            { 0 };
    uint                 m_needPrebufferFrames       { 0 };
    uint                 m_needPrebufferFramesNormal { 0 };
    uint                 m_needPrebufferFramesSmall  { 0 };
    bool                 m_createdPauseFrame         { false };
    uint                 m_rpos                      { 0 };
    uint                 m_vpos                      { 0 };
    mutable QMutex       m_globalLock                { QMutex::Recursive };
};

#endif // VIDEOBUFFERS_H
