//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CWaveFileHandler , helper class for handling wav files. Use this class to read/write wav files
// internally, this class makes use of memory mapped files for doing reads. 
// 
// PLEASE NOTE : the PutRiff, PutFmt and PutData calls MUST be made in this order only. Invoking
// PutRiff after PutFmt or PutData may corrupt the wav file. this limitation will be addressed in
// the next revision.
//
// All methods would throw a const char* exception.
//
// This class can  handle one file in read mode and one in write mode at the same time.
//
// -Vinayak Raghuvamshi
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CWAVEFILEHANDLER_H_
#define _CWAVEFILEHANDLER_H_

#include <stdint.h>
#include <QByteArray>
#include <QFile>

namespace Ui
{
    class WaveInspector;
}

struct WAVE_FORMAT
{
    uint16_t	wFormatTag;
    uint16_t	wChannels;
    uint32_t	dwSamplesPerSec;
    uint32_t	dwAvgBytesPerSec;
    uint16_t	wBlockAlign;
    uint16_t	wBitsPerSample;
};

struct RIFF_HEADER
{
    char	szRiffID[4];		// 'R','I','F','F'
    uint32_t	dwRiffSize;
    char	szRiffFormat[4];	// 'W','A','V','E'
};

struct FMT_BLOCK
{
    char		szFmtID[4];	// 'f','m','t',' '
    uint32_t		dwFmtSize;
	WAVE_FORMAT	wavFormat;
};

struct DATA_BLOCK
{
    char	szDataID[4];	// 'd','a','t','a'
    uint32_t	dwDataSize;
};

struct AUDIO_DATA
{
    uint16_t m_minAudioValue;
    uint16_t m_maxAudioValue;
};

struct AUDIO_CHANNEL
{
    AUDIO_DATA *m_pAudioData;
};

class CWaveFileHandler
{
public:
	CWaveFileHandler(void);
	virtual ~CWaveFileHandler(void);
    /**
     * Opens a wav file for reading. fails if not exists. throws char * exception
     */
    bool OpenForRead(const char* szFile);
    bool isValidRiffHeader();
    /**
     * Looks for and verifies the 'fmt ' chunk
     */
    qint64 FindValidFmtChunk();
    /**
     * Looks for the 'data' chunk
     */
    qint64 FindValidDataChunk();

    /**
     * Analyzes data chunk
     */
    void analyzedataChunk();
    /**
     * Returns the value of the length of the audio sample in seconds
     */
    uint16_t lengthOfAudioSample();
    /**
     * Returns the value of the length of the audio sample in seconds
     */
    AUDIO_CHANNEL * getAudioChannels();
    void CloseReader(void);

    /**
     * getters
     */
    RIFF_HEADER * getRiffHeader();
    FMT_BLOCK *   getFmtHeader();
    DATA_BLOCK *  getDataBlock();
    qint64 getWavFileSize();

private:
    QFile * m_wavfile;
    RIFF_HEADER     *m_pRiffHeader;		// start of the Riff chunk
	FMT_BLOCK		*m_pFmtHeader;		// start of the Fmt chunk
	DATA_BLOCK		*m_pDataBlock;		// start of the Data header chunk
    AUDIO_CHANNEL   *m_pAudioChannels;      // pointer to structure for audio data analysis
};

inline RIFF_HEADER * CWaveFileHandler::getRiffHeader()
{
    return m_pRiffHeader;
}

inline FMT_BLOCK *   CWaveFileHandler::getFmtHeader()
{
    return m_pFmtHeader;
}

inline DATA_BLOCK *  CWaveFileHandler::getDataBlock()
{
    return m_pDataBlock;
}

inline qint64 CWaveFileHandler::getWavFileSize()
{
    return m_wavfile->size();
}

inline AUDIO_CHANNEL * CWaveFileHandler::getAudioChannels()
{
    return m_pAudioChannels;
}

#endif // _CWAVEFILEHANDLER_H_
