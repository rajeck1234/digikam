/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-10-31
 * Description : QtAV: Multimedia framework based on Qt and FFmpeg
 *
 * SPDX-FileCopyrightText: 2012-2022 Wang Bin <wbsecg1 at gmail dot com>
 * SPDX-FileCopyrightText:      2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "AudioOutputBackend.h"

// Qt includes

#include <QLibrary>
#include <QSemaphore>
#include <QThread>
#include <math.h>

// DirectX includes

#define DIRECTSOUND_VERSION 0x0600

#include <dsound.h>

// Local includes

#include "QtAV_mkid.h"
#include "QtAV_factory.h"
#include "AVCompat.h"
#include "digikam_debug.h"

#define DX_LOG_COMPONENT "DSound"

#include "DirectXHelper.h"

namespace QtAV
{

static const char kName[] = "DirectSound";

class Q_DECL_HIDDEN AudioOutputDSound final : public AudioOutputBackend
{
public:

    AudioOutputDSound(QObject* const parent = nullptr);

    QString name()                                           const override
    {
        return QString::fromLatin1(kName);
    }

    bool open()                                                    override;
    bool close()                                                   override;
    bool isSupported(AudioFormat::SampleFormat sampleFormat) const override;
    BufferControl bufferControl()                            const override;
    bool write(const QByteArray& data)                             override;
    bool play()                                                    override;
    int getOffsetByBytes()                                         override;

    bool setVolume(qreal value)                                    override;
    qreal getVolume()                                        const override;
    void onCallback()                                              override;

private:

    bool loadDll();
    bool unloadDll();
    bool init();

    bool destroy()
    {
        SafeRelease(&notify);
        SafeRelease(&prim_buf);
        SafeRelease(&stream_buf);
        SafeRelease(&dsound);
        unloadDll();

        return true;
    }

    bool createDSoundBuffers();

    static DWORD WINAPI notificationThread(LPVOID lpThreadParameter);

private:

    HINSTANCE           dll;
    LPDIRECTSOUND       dsound;         ///< direct sound object
    LPDIRECTSOUNDBUFFER prim_buf;       ///< primary direct sound buffer
    LPDIRECTSOUNDBUFFER stream_buf;     ///< secondary direct sound buffer (stream buffer)
    LPDIRECTSOUNDNOTIFY notify;
    HANDLE              notify_event;
    QSemaphore          sem;
    int                 write_offset;   ///< offset of the write cursor in the direct sound buffer
    QAtomicInt          buffers_free;

    class Q_DECL_HIDDEN PositionWatcher : public QThread
    {
        AudioOutputDSound* ao = nullptr;

    public:

        explicit PositionWatcher(AudioOutputDSound* const dsound)
            : ao(dsound)
        {
        }

        void run() override
        {
            DWORD dwResult = 0;

            while (ao->available)
            {
               dwResult = WaitForSingleObjectEx(ao->notify_event, 2000, FALSE);

               if (dwResult != WAIT_OBJECT_0)
               {
/*
                    qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
                        << QString::asprintf("WaitForSingleObjectEx for ao->notify_event error: %#lx",
                            dwResult);
*/
                    continue;
               }

               ao->onCallback();
            }
        }
    };

    PositionWatcher watcher;
};

typedef AudioOutputDSound AudioOutputBackendDSound;

static const AudioOutputBackendId AudioOutputBackendId_DSound = mkid::id32base36_6<'D', 'S', 'o', 'u', 'n', 'd'>::value;

FACTORY_REGISTER(AudioOutputBackend, DSound, kName)

// use the definitions from the win32 api headers when they define these

#define WAVE_FORMAT_IEEE_FLOAT      0x0003
#define WAVE_FORMAT_DOLBY_AC3_SPDIF 0x0092
#define WAVE_FORMAT_EXTENSIBLE      0xFFFE

/* GUID SubFormat IDs */

/* We need both b/c const variables are not compile-time constants in C, giving
 * us an error if we use the const GUID in an enum
 */

#undef DEFINE_GUID
#define DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) EXTERN_C const GUID DECLSPEC_SELECTANY name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }

DEFINE_GUID(_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT,      WAVE_FORMAT_IEEE_FLOAT,      0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(_KSDATAFORMAT_SUBTYPE_DOLBY_AC3_SPDIF, WAVE_FORMAT_DOLBY_AC3_SPDIF, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(_KSDATAFORMAT_SUBTYPE_PCM,             WAVE_FORMAT_PCM,             0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(_KSDATAFORMAT_SUBTYPE_UNKNOWN,         0x00000000,                  0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

#ifndef MS_GUID

#   define MS_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    static const GUID name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }

#endif // MS_GUID

#ifndef _WAVEFORMATEXTENSIBLE_

typedef struct Q_DECL_HIDDEN
{
   WAVEFORMATEX    Format;

   union
   {
      WORD wValidBitsPerSample;       ///< bits of precision
      WORD wSamplesPerBlock;          ///< valid if wBitsPerSample == 0
      WORD wReserved;                 ///< If neither applies, set to zero.
   } Samples;

   DWORD           dwChannelMask;     ///< which channels are

   // present in stream

   GUID            SubFormat;
} WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE;

#endif

/* Microsoft speaker definitions. key/values are equal to FFmpeg's */

#define SPEAKER_FRONT_LEFT             0x1
#define SPEAKER_FRONT_RIGHT            0x2
#define SPEAKER_FRONT_CENTER           0x4
#define SPEAKER_LOW_FREQUENCY          0x8
#define SPEAKER_BACK_LEFT              0x10
#define SPEAKER_BACK_RIGHT             0x20
#define SPEAKER_FRONT_LEFT_OF_CENTER   0x40
#define SPEAKER_FRONT_RIGHT_OF_CENTER  0x80
#define SPEAKER_BACK_CENTER            0x100
#define SPEAKER_SIDE_LEFT              0x200
#define SPEAKER_SIDE_RIGHT             0x400
#define SPEAKER_TOP_CENTER             0x800
#define SPEAKER_TOP_FRONT_LEFT         0x1000
#define SPEAKER_TOP_FRONT_CENTER       0x2000
#define SPEAKER_TOP_FRONT_RIGHT        0x4000
#define SPEAKER_TOP_BACK_LEFT          0x8000
#define SPEAKER_TOP_BACK_CENTER        0x10000
#define SPEAKER_TOP_BACK_RIGHT         0x20000
#define SPEAKER_RESERVED               0x80000000

static int channelMaskToMS(qint64 av)
{
    if (av >= (qint64)SPEAKER_RESERVED)
        return 0;

    return (int)av;
}

static int channelLayoutToMS(qint64 av)
{
    return channelMaskToMS(av);
}

AudioOutputDSound::AudioOutputDSound(QObject* const parent)
    : AudioOutputBackend(AudioOutput::DeviceFeatures() | AudioOutput::SetVolume, parent)
      dll               (nullptr),
      dsound            (nullptr),
      prim_buf          (nullptr),
      stream_buf        (nullptr),
      notify            (nullptr),
      notify_event      (nullptr),
      write_offset      (0),
      watcher           (this)
{
/*
    setDeviceFeatures(AudioOutput::DeviceFeatures()|AudioOutput::SetVolume);
*/
}

bool AudioOutputDSound::open()
{
    if (!init())
        goto error;

    if (!createDSoundBuffers())
        goto error;

    return true;

error:

    unloadDll();
    SafeRelease(&dsound);

    return false;
}

bool AudioOutputDSound::close()
{
    available = false;
    destroy();
    CloseHandle(notify_event); // FIXME: is it ok if thread is still waiting?

    return true;
}

bool AudioOutputDSound::isSupported(AudioFormat::SampleFormat sampleFormat) const
{
    return !IsPlanar(sampleFormat);
}

AudioOutputBackend::BufferControl AudioOutputDSound::bufferControl() const
{
    // Both works. I prefer CountCallback

    return CountCallback; // OffsetBytes;
}

void AudioOutputDSound::onCallback()
{
    if (bufferControl() & CountCallback)
    {
/*
        qCDebug(DIGIKAM_QTAV_LOG).noquote()
            << QString::asprintf("callback: %d", sem.available());
*/
        if (sem.available() < buffer_count)
        {
            sem.release();

            return;
        }
    }
    else
    {
/*
        if (buffers_free.deref())
        {
            return;
        }

        buffers_free.ref();
*/
    }
/*
    DWORD status;
    stream_buf->GetStatus(&status);
    qCDebug(DIGIKAM_QTAV_LOG).noquote() << QString::asprintf("status: %lu", status);
    return;

    if (status & DSBSTATUS_LOOPING)
    {
        // sound will loop even if buffer is finished

        DX_ENSURE(stream_buf->Stop());

        // reset positions to ensure the notification positions and played buffer matches

        DX_ENSURE(stream_buf->SetCurrentPosition(0));
        write_offset = 0;
    }
*/
}

bool AudioOutputDSound::write(const QByteArray& data)
{
/*
    qCDebug(DIGIKAM_QTAV_LOG).noquote()
        << QString::asprintf("sem %d %d", sem.available(), buffers_free.load());
*/
    if (bufferControl() & CountCallback)
    {
        sem.acquire();
    }
    else
    {
        if (buffers_free <= buffer_count)
            buffers_free.ref();
    }

    LPVOID dst1 = nullptr, dst2 = nullptr;
    DWORD size1 = 0, size2 = 0;

    if (write_offset >= buffer_size*buffer_count) // !!! >=
        write_offset = 0;

    HRESULT res = stream_buf->Lock(write_offset, data.size(), &dst1, &size1, &dst2, &size2, 0); // DSBLOCK_ENTIREBUFFER

    if (res == DSERR_BUFFERLOST)
    {
        qCDebug(DIGIKAM_QTAV_LOG).noquote() << QString::asprintf("buffer lost");

        DX_ENSURE(stream_buf->Restore(), false);
        DX_ENSURE(stream_buf->Lock(write_offset, data.size(), &dst1, &size1, &dst2, &size2, 0), false);
    }

    memcpy(dst1, data.constData(), size1);

    if (dst2)
        memcpy(dst2, data.constData() + size1, size2);

    write_offset += size1 + size2;

    if (write_offset >= (buffer_size * buffer_count))
        write_offset = size2;

    DX_ENSURE_OK(stream_buf->Unlock(dst1, size1, dst2, size2), false);

    return true;
}

bool AudioOutputDSound::play()
{
    DWORD status;
    stream_buf->GetStatus(&status);

    if (!(status & DSBSTATUS_PLAYING))
    {
        // must be DSBPLAY_LOOPING. Sound will be very slow if set to 0. I was fucked for a long time. DAMN!

        stream_buf->Play(0, 0, DSBPLAY_LOOPING);
    }

    return true;
}

int AudioOutputDSound::getOffsetByBytes()
{
    DWORD read_offset = 0;
    stream_buf->GetCurrentPosition(&read_offset /*play*/, nullptr /*write*/); // what's this write_offset?

    return (int)read_offset;
}

bool AudioOutputDSound::setVolume(qreal value)
{
    // dsound supports [0, 1]

    const LONG vol = ((value <= 0) ? DSBVOLUME_MIN : LONG(log10(value * 100.0) * 5000.0) + DSBVOLUME_MIN);

    // +DSBVOLUME_MIN == -100dB

    DX_ENSURE_OK(stream_buf->SetVolume(vol), false);

    return true;
}

qreal AudioOutputDSound::getVolume() const
{
    LONG vol = 0;
    DX_ENSURE_OK(stream_buf->GetVolume(&vol), 1.0);

    return pow(10.0, double(vol - DSBVOLUME_MIN) / 5000.0) / 100.0;
}

bool AudioOutputDSound::loadDll()
{
    dll = LoadLibrary(TEXT("dsound.dll"));

    if (!dll)
    {
        qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
            << QString::asprintf("Can not load dsound.dll");

        return false;
    }

    return true;
}

bool AudioOutputDSound::unloadDll()
{
    if (dll)
        FreeLibrary(dll);

    return true;
}

bool AudioOutputDSound::init()
{
    if (!loadDll())
        return false;

    typedef HRESULT (WINAPI* DirectSoundCreateFunc)(LPGUID, LPDIRECTSOUND*, LPUNKNOWN);

    //typedef HRESULT (WINAPI *DirectSoundEnumerateFunc)(LPDSENUMCALLBACKA, LPVOID);

    DirectSoundCreateFunc dsound_create = (DirectSoundCreateFunc)GetProcAddress(dll, "DirectSoundCreate");
/*
    DirectSoundEnumerateFunc dsound_enumerate = (DirectSoundEnumerateFunc)GetProcAddress(dll, "DirectSoundEnumerateA");
*/
    if (!dsound_create)
    {
        qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote() << QString::asprintf("Failed to resolve 'DirectSoundCreate'");
        unloadDll();

        return false;
    }

    DX_ENSURE_OK(dsound_create(nullptr/*dev guid*/, &dsound, nullptr), false);

    /**
     * DSSCL_EXCLUSIVE: can modify the settings of the primary buffer, only the sound of this app will be hearable when it will have the focus.
     */

    DX_ENSURE_OK(dsound->SetCooperativeLevel(GetDesktopWindow(), DSSCL_EXCLUSIVE), false);
    qCDebug(DIGIKAM_QTAV_LOG).noquote() << QString::asprintf("DirectSound initialized.");
    DSCAPS dscaps;
    memset(&dscaps, 0, sizeof(DSCAPS));
    dscaps.dwSize = sizeof(DSCAPS);
    DX_ENSURE_OK(dsound->GetCaps(&dscaps), false);

    if (dscaps.dwFlags & DSCAPS_EMULDRIVER)
        qCDebug(DIGIKAM_QTAV_LOG).noquote() << QString::asprintf("DirectSound is emulated");

    write_offset  = 0;

    return true;
}

/**
 * Creates a DirectSound buffer of the required format.
 *
 * This function creates the buffer we'll use to play audio.
 * In DirectSound there are two kinds of buffers:
 * - the primary buffer: which is the actual buffer that the soundcard plays
 * - the secondary buffer(s): these buffers are the one actually used by
 *    applications and DirectSound takes care of mixing them into the primary.
 *
 * Once you create a secondary buffer, you cannot change its format anymore so
 * you have to release the current one and create another.
 */
bool AudioOutputDSound::createDSoundBuffers()
{
    WAVEFORMATEXTENSIBLE wformat;

    // TODO:  Dolby Digital AC3

    ZeroMemory(&wformat, sizeof(WAVEFORMATEXTENSIBLE));
    WAVEFORMATEX wf;
    wf.cbSize               = 0;
    wf.nChannels            = format.channels();
    wf.nSamplesPerSec       = format.sampleRate();                      // FIXME: use supported values
    wf.wBitsPerSample       = format.bytesPerSample() * 8;
    wf.nBlockAlign          = wf.nChannels * format.bytesPerSample();
    wf.nAvgBytesPerSec      = wf.nSamplesPerSec * wf.nBlockAlign;
    wformat.dwChannelMask   = channelLayoutToMS(format.channelLayoutFFmpeg());

    if (format.channels() > 2)
    {
        wf.cbSize                           = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
        wf.wFormatTag                       = WAVE_FORMAT_EXTENSIBLE;
        wformat.SubFormat                   = _KSDATAFORMAT_SUBTYPE_PCM;
        wformat.Samples.wValidBitsPerSample = wf.wBitsPerSample;
    }

    Q_UNUSED(wformat.SubFormat);

    if (format.isFloat())
    {
        wf.wFormatTag     = WAVE_FORMAT_IEEE_FLOAT;
        wformat.SubFormat = _KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
    }
    else
    {
        wf.wFormatTag     = WAVE_FORMAT_PCM;
        wformat.SubFormat = _KSDATAFORMAT_SUBTYPE_PCM;
    }

    wformat.Format = wf;

    if (true)
    {
        // format.channels() <= 2 && !format.isFloat()) { //openal use this, don't know why
        // fill in primary sound buffer descriptor

        DSBUFFERDESC dsbpridesc;
        memset(&dsbpridesc, 0, sizeof(DSBUFFERDESC));
        dsbpridesc.dwSize  = sizeof(DSBUFFERDESC);
        dsbpridesc.dwFlags = DSBCAPS_PRIMARYBUFFER;

        // create primary buffer and set its format

        DX_ENSURE(dsound->CreateSoundBuffer(&dsbpridesc, &prim_buf, nullptr), (destroy() && false));
        DX_ENSURE(prim_buf->SetFormat((WAVEFORMATEX*)&wformat), false);
    }

    // fill in the secondary sound buffer (=stream buffer) descriptor

    DSBUFFERDESC dsbdesc;
    memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
    dsbdesc.dwSize        = sizeof(DSBUFFERDESC);

    dsbdesc.dwFlags       = DSBCAPS_GETCURRENTPOSITION2 | /** Better position accuracy  */
                            DSBCAPS_GLOBALFOCUS         | /** Allows background playing */
                            DSBCAPS_CTRLVOLUME          | /** volume control enabled    */
                            DSBCAPS_CTRLPOSITIONNOTIFY;

    dsbdesc.dwBufferBytes = buffer_size * buffer_count;
    dsbdesc.lpwfxFormat   = (WAVEFORMATEX*)&wformat;

    // Needed for 5.1 on emu101k - shit soundblaster

    if (format.channels() > 2)
        dsbdesc.dwFlags |= DSBCAPS_LOCHARDWARE;

    // now create the stream buffer (secondary buffer)

    HRESULT res = dsound->CreateSoundBuffer(&dsbdesc, &stream_buf, nullptr);

    if (res != DS_OK)
    {
        if (dsbdesc.dwFlags & DSBCAPS_LOCHARDWARE)
        {
            // Try without DSBCAPS_LOCHARDWARE

            dsbdesc.dwFlags &= ~DSBCAPS_LOCHARDWARE;
            DX_ENSURE_OK(dsound->CreateSoundBuffer(&dsbdesc, &stream_buf, nullptr), (destroy() && false));
        }
    }

    qCDebug(DIGIKAM_QTAV_LOG) << "Secondary (stream) buffer created";

    MS_GUID(IID_IDirectSoundNotify, 0xb0210783, 0x89cd, 0x11d0, 0xaf, 0x8, 0x0, 0xa0, 0xc9, 0x25, 0xcd, 0x16);

    DX_ENSURE(stream_buf->QueryInterface(IID_IDirectSoundNotify, (void**)&notify), false);

    notify_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    QVector<DSBPOSITIONNOTIFY> notification(buffer_count);

    for (int i = 0 ; i < buffer_count ; ++i)
    {
        notification[i].dwOffset     = buffer_size * (i + 1) - 1;
        notification[i].hEventNotify = notify_event;
    }
/*
    notification[buffer_count].dwOffset = DSBPN_OFFSETSTOP;
    notification[buffer_count].hEventNotify = stop_notify_event;
*/
    DX_ENSURE(notify->SetNotificationPositions(notification.size(), notification.constData()), false);
    available = true;

    watcher.start();
    sem.release(buffer_count - sem.available());

    return true;
}

} // namespace QtAV
